/*  
 * pngnq-s9: neuquant32.c
 * 
 * ------------------------------------------------------------------------
 *  pngnq-s9 Authorship and Copyright
 * --------------------------
 * 
 *  pngnq-s9 is a modification of pngnq.  pngnq is based on pngquant and
 *  the NeuQuant procedure.  pngquant, in turn, was based on ppmquant. 
 * 
 * 
 * ------------------------------------------------------------------------
 *  NeuQuant Notice
 * -----------------
 * 
 *   NeuQuant Neural-Net Quantization Algorithm
 *  
 *   Copyright (c) 1994 Anthony Dekker
 *  
 *   NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 *   See "Kohonen neural networks for optimal colour quantization" in
 *   "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 *   for a discussion of the algorithm.
 *   See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *  
 *   Any party obtaining a copy of these files from the author, directly or
 *   indirectly, is granted, free of charge, a full and unrestricted
 *   irrevocable, world-wide, paid up, royalty-free, nonexclusive right and
 *   license to deal in this software and documentation files (the
 *   "Software"), including without limitation the rights to use, copy,
 *   modify, merge, publish, distribute, sublicense, and/or sell copies of
 *   the Software, and to permit persons who receive copies from any such
 *   party to do so, with the only requirement being that this copyright
 *   notice remain intact.
 * 
 * 
 * ------------------------------------------------------------------------
 *  pngquant Notice
 * -----------------
 * 
 *   Copyright (c) 1998-2000 Greg Roelofs.  All rights reserved.
 * 
 *   This software is provided "as is," without warranty of any kind,
 *   express or implied.  In no event shall the author or contributors
 *   be held liable for any damages arising in any way from the use of
 *   this software.
 * 
 *   Permission is granted to anyone to use this software for any purpose,
 *   including commercial applications, and to alter it and redistribute
 *   it freely, subject to the following restrictions:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, disclaimer, and this list of conditions.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, disclaimer, and this list of conditions in the documenta-
 *      tion and/or other materials provided with the distribution.
 *   3. All advertising materials mentioning features or use of this
 *      software must display the following acknowledgment:
 * 
 *         This product includes software developed by Greg Roelofs
 *         and contributors for the book, "PNG: The Definitive Guide,"
 *         published by O'Reilly and Associates.
 * 
 * 
 * ------------------------------------------------------------------------
 *  pngnq Notice
 * --------------
 *
 *  Based on Greg Roelf's pngquant which was itself based on Jef
 *  Poskanzer's ppmquant.  Uses Anthony Dekker's Neuquant algorithm
 *  extended to handle the alpha channel.
 *
 *  Modified to quantize 32bit RGBA images for the pngnq program.  Also
 *  modified to accept a numebr of colors argument. 
 *  Copyright (c) Stuart Coyle 2004-2006
 * 
 *  Rewritten by Kornel Lesiński (2009):
 *  Euclidean distance, color matching dependent on alpha channel and with
 *  gamma correction. code refreshed for modern compilers/architectures:
 *  ANSI C, floats, removed pointer tricks and used arrays and structs.
 *
 *  Copyright (C) 1989, 1991 by Jef Poskanzer.
 *  Copyright (C) 1997, 2000, 2002 by Greg Roelofs; based on an idea by
 *                                Stefan Schneider.
 *  Copyright (C) 2004-2009 by Stuart Coyle
 *  Copyright (C) Kornel Lesiński (2009)
 *
 *  Permission to use, copy, modify, and distribute this software and
 *  its documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and
 *  that both that copyright notice and this permission notice appear in
 *  supporting documentation.  This software is provided "as is" without
 *  express or implied warranty.
 * 
 * 
 * ------------------------------------------------------------------------
 *  pngnq-s9 Notice
 * -----------------
 * 
 *  Further alterations copyright (c) Adrian Pellas-Rice 2011-2012: YUV
 *  colour space, unisolate, transparency extenuation, user-supplied
 *  palette, per-component sensitivity, exclusion threshold, repel learning
 *  phase, remap result caching and SIMD oriented code, etc.  Alterations
 *  available under the same terms as for pngnq above.
 * 
 * 
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 * ------------------------------------------------------------------------
 *
 * neuquant32.c -- Implements a modified NeuQuant colour selection
 *                 procedure.
 *
 * Contributors: Dekker, Coyle, Lesiński, Pellas-Rice.
 *
 * This module implements a modified version of Dekker's Neuquant procedure,
 * described in the paper mentioned in his copyright notice above.  For a
 * description of the modifications present in pngnq-s9, refer to the document
 * "The Saiki 9 Modifications to pngnq". 
 *
 * The key data structure used by this module is a Kohonen style neural network
 * with lockable nodes.  Each node in the network represents a palette colour
 * that is being learnt in order to quantize the image.  The representation is
 * quite directed - nq_colour_vector has four components that correspond
 * directly to RGBA (or YUVA) colour components, although they may have been
 * scaled or extenuated to skew the learning procedure.  
 *
 * There are five arrays that contain data associated with neurons.  In all
 * cases only the first netsize entries are valid, the rest are unused:
 * 
 * network[] contains each neuron in the network.
 * 
 * neuronlock[] is used to indicate whether each neuron (with a corresponding
 * index) is locked.  (The user can supply locked colours that they insist go
 * into the final colour palette.)  When network[x] is locked, neuronlock[x]
 * should point to the original (untransformed) RGBA value for the colour.
 * When network[x] is unlocked, neuronlock[x] must be NULL.  
 *
 * freq[x] measures the decaying frequency of use of neuron x.  It is used by
 * the Neuquant procedure to calculate the load-balancing bias for neuron x.  
 * The values in freq are still calculated using fixed point arithmetic.
 * 
 * bias[x] contains the load-balancing bias for neuron x.  The values in bias
 * still use fixed point arithmetic.
 *
 * repel_points[x] stores a measure of how often neuron x has been repelling
 * its neighbours.
 *
 * ------------------------------------------------------------------------
 * 
 */


#include "neuquant32.h"
#include "nqcvector.h"
#include <math.h>
#include <stdlib.h>

/*
 * Utility macros
 */

    /* ABS(n) returns the absolute value of n, where n is a number.*/
#define ABS(a) ((a)>=0?(a):-(a))

#define true        1
#define false       0

    /* Value larger than any difference we will calculate. */
#define MAXDIFF      1<<30
    /* No difference, zero. */
#define DIFFZERO     0.0f


/* 
 *  Network Definitions, Types and Global Variables
 */

#define maxnetpos   (MAXNETSIZE-1)
#define ncycles     100                 /* no. of learning cycles */

/* defs for freq and bias */
#define gammashift  10                  /* gamma = 1024 */
#define gamma       ((double)(1<<gammashift))
#define betashift   10
#define beta        (1.0/(1<<betashift))/* beta = 1/1024 */
#define betagamma   ((double)(1<<(gammashift-betashift)))

/* defs for decreasing radius factor */
#define initrad     (MAXNETSIZE>>3)     /* for 256 cols, radius starts */
#define initradius  (initrad*1.0)       /* and decreases by a           */
#define radiusdec   30                  /* factor of 1/30 each cycle    */ 

/* defs for decreasing learning alpha factor */
#define alphabiasshift  10              /* alpha starts at 1.0 */
#define initalpha   ((double)(1<<alphabiasshift))
#define initialalphadec 30              /* alpha decay starts near 30 */
#define alphadecsamplefacdivisor 3      /* fraction of samplefac for decay*/
double alphadec;                        /* biased by 10 bits */

/* defs for pngnq-s9 extended learning */
#define normal_learning_extension_factor 2 /* normally learn twice as long */
#define extra_long_colour_threshold 40 /* learn even longer when under 40 */
#define extra_long_divisor 8 /* fraction of netsize for extra learning */

/* radbias and alpharadbias used for radpower calculation */
#define radbiasshift    8
#define radbias         (1<<radbiasshift)
#define alpharadbshift  (alphabiasshift+radbiasshift)
#define alpharadbias    ((double)(1<<alpharadbshift))

/* [MINZERO, MAX255] is the ordinary range of colour component values, (but internal processing may change each component's
 * range).  These are abbreviations only, changing the values may introduce errors.
 */
#define MINZERO 0.0
#define MAX255 255.0

/* 
 * The Input Image
 */   
static unsigned char *thepicture;      /* the input image itself */
static unsigned int lengthcount;        /* lengthcount = H*W*4 */

/* 
 * The neural network used to learn the colours, and associated arrays. 
 */
static nq_colour_vector network[MAXNETSIZE];    /* The network itself */
static unsigned char *neuronlock[MAXNETSIZE];   /* Locks for each of the neurons. NULL means unlocked, otherwise a pointer to
                                                 * the original palette colour value is used. */
static double bias [MAXNETSIZE];        /* Bias array for learning */
static double freq [MAXNETSIZE];        /* Freq array for learning */
static double radpower[initrad];        /* Radpower for precomputation */
static unsigned int netsize;            /* Number of colours to use. */
static unsigned int repel_points[MAXNETSIZE]; /* Number of points this neuron has for repelling */
#define REPEL_THRESHOLD 16              /* See repel_coincident()... */
#define REPEL_STEP_DOWN 1               /* ... for an explanation of... */
#define REPEL_STEP_UP 4                 /* ... how these points work. */


/*
 * Parameters and computation tables for colour selection. 
 */
static unsigned int colour_space;       /* Colour space to use, RGB or YUV */

#define GAMMA_TABLE_SIZE 256            /* Size of gamma correction table. */
static double gamma_correction;         /* Gamma value for gamma correction */
static float gamma_correction_table[GAMMA_TABLE_SIZE]; /* Table itself. */

static double alpha_extenuation;   /* Offset to exaggerate alpha values of 0 and 255. */
static unsigned int force_alpha_class_correctness;   /* Offset to exaggerate alpha values of 0 and 255. */
static int use_alpha_importance_heuristic; /* Use colorimportance() heuristic to avoid emphasising non-alpha components when
                                            * alpha is low.*/ 

static double exclusion_threshold;      /* Component wise threshold for deeming colours identical. */

nq_colour_vector sensitivity;           /* Sensitivity - factors used to scale the valid range of each component, affecting
                                           relative sensitivity to particular colour compoments. */
nq_colour_vector reciprocal_sensitivity;/* Precomputed 1.0/sensitivity */
nq_colour_vector remap_sensitivity;     /* Sensitivity for remap phase. */

nq_colour_vector disaster_threshold;    /* Thresholds for deeming component differences 'diastrous', calculated using... */
#define disaster_factor 0.125           /* ... sensitivity and this disaster_factor. */


/* 
 * YUV Conversion Constants
 */
nq_colour_vector to_y_factors;          /* Y row of toYUV conversion matrix */
nq_colour_vector to_u_factors;          /* U row of toYUV conversion matrix */
nq_colour_vector to_v_factors;          /* V row of toYUV conversion matrix */
nq_colour_vector to_yuv_scale;          /* To scale and offset the new YUV..*/
#define u_offset_fraction 0.436f        /* ...values to our standard range..*/
#define v_offset_fraction 0.615f        /* ...of [MINZERO, MAX255].*/

nq_colour_vector to_r_factors;          /* R row, fromYUV conversion matrix */
nq_colour_vector to_g_factors;          /* G row, fromYUV conversion matrix */
nq_colour_vector to_b_factors;          /* B row, fromYUV conversion matrix */
nq_colour_vector from_yuv_unscale;      /* To un-scale YUV values from our standard range back to conversion matrices' ranges.*/

/*
 * inxsearch()'s Cache
 */
#define SEARCH_CACHE_SIZE 65536     /* Size of cache: 64K, depends on hash_value() returning 16 bit hash. */ 
#define CACHE_EMPTY_ENTRY -1        /* Value given to invalid cahce entry.*/
static int search_cache[SEARCH_CACHE_SIZE];  /* Cache itself */ 
static int search_cache_keys[SEARCH_CACHE_SIZE][NQ_COLOUR_NUM_COMPONENTS];  /* Keys of valid cache entries. */

/*
 * Local function declarations.  Each is documented at point of definition. 
 */
nq_colour_vector resensitise(nq_colour_vector pix);
nq_colour_vector desensitise(nq_colour_vector pix); 
nq_colour_vector alpha_exaggerate(nq_colour_vector pix);
nq_colour_vector alpha_unexaggerate(nq_colour_vector pix);
void netsort(int currnetsize);
nq_colour_vector internaliseLearningPixel(int r_in, int g_in, int b_in, int al_in);
nq_colour_vector externalise(nq_colour_vector pix);
static inline nq_colour_vector calc_context_weighting(nq_colour_vector target_pix);
static inline float sensitised_round_clamp(float x, float sens);

/*
 * Functions that embody complex constants: 
 */

/* init_yuv_matrices() initialises the nq_colour_vectors that make up the YUV conversion matrices.  Parameters sourced from
 * Wikipedia, en.wikipedia.org/wiki/YUV circa 2012.
 */
void init_yuv_matrices() {

    /* For toYUV() */
    to_y_factors = init_vec(0.299f, 0.587f, 0.114f, 0.0f);
    to_u_factors = init_vec(-0.14713f, -0.28886f, 0.436f, 0.0f);
    to_v_factors = init_vec(0.615f, -0.51499f, -0.10001f, 0.0f);
    
    to_yuv_scale = init_vec(1.0f, 1.0/0.872, 1.0/1.23, 1.0f);

    /* For fromYUV() */
    to_r_factors = init_vec(1.0f, 0.0f, 1.13983f, 0.0f);
    to_g_factors = init_vec(1.0f, -0.39465f, -0.58060f, 0.0f);
    to_b_factors = init_vec(1.0f, 2.03211f, 0.0f, 0.0f);
    
    from_yuv_unscale = init_vec(1.0f, 0.872f, 1.23f, 1.0f);
}


/* calc_context_weighting(target):
 *
 * In certain cases, particular colour components will be somewhat or entirely unimportant.  When alpha is zero, for example,
 * no other colour component matters.  In YUV mode, as Y approaches zero or 255, U and V are increasingly irrelevant.
 * 
 * To account for this in functions that compare candidate colours to a target colour, we use the target colour to create a
 * weighting vector, and then multiply the difference colour vector by the weighting vector to selectively downplay or ignore
 * components as necessary.
 *
 * This function returns such a weighting vector.  Each component is given a value in the range [0.0, 1.0], where normal full
 * importance is 1.0, and total irrelevance is 0.0.
 * 
 * Most colours simply require full (1,1,1,1) weighting.  There are two special cases as mentioned above, (1) low alpha, and
 * (2) extreme Y in YUV mode.
 *
 * For the low alpha case, our observation is that when alpha is zero the other components don't matter, and when alpha is very
 * low they probably don't matter.  In this case the weighting vector will be (k,k,k,1.0) where k is one for all alpha values
 * from 100% down to 50%, and then linearly declines to zero as alpha declines from 50% down to 0%.
 *
 * Earlier versions of pngnq encoded a variation on this behaviour as the colorimportance() heuristic.  When images use only
 * one hue, even small hue variations with very low alpha stand out, so this behaviour can be suppressed by setting
 * use_alpha_importance_heuristic to false.
 *
 * For YUV extreme Y case, our function similar but with declines at both ends.  The weigthing vector is (1.0,k,k,1.0) where k
 * is one between 75% and 25%, but linearly declines to zero from 25% to 0% and also from 75% to 100%. 
 */

static inline nq_colour_vector calc_context_weighting(nq_colour_vector target_pix) {

    nq_colour_vector context_weighting;
    const float full = 1.0;
    const float none = 0.0;

    const float mid = 0.5;

    float colour_importance = full;
    if(     use_alpha_importance_heuristic
        &&  target_pix.elems[alpha] <= (mid*sensitivity.elems[alpha]*MAX255)) {
        
        colour_importance = target_pix.elems[alpha] / (mid*sensitivity.elems[alpha]*MAX255);

        if(colour_importance < none) {
            colour_importance = none;
        }
    }

    context_weighting = init_vec(colour_importance, colour_importance,
        colour_importance, full);

    const float low = 1.0 / 32.0;
    const float high = 1.0 - low;

    if(colour_space == YUV) {
        float uv_importance;
        float low_point = low*sensitivity.elems[yy]*MAX255;
        float high_point = high*sensitivity.elems[yy]*MAX255;
        float max_point = sensitivity.elems[yy]*MAX255;
        if(target_pix.elems[yy] < low_point) {
            uv_importance = ABS(target_pix.elems[yy] / low_point);
        } else if(target_pix.elems[yy] > high_point) {
            uv_importance = ABS((max_point - target_pix.elems[yy]) / low_point);
        } else {
            uv_importance = full;
        }

        context_weighting = vec_mul(context_weighting, init_vec(full, uv_importance, uv_importance, full));
    }

    return context_weighting;
}


/*
 * programming_error(message) is used when logic violations are detected in the program, and we need to abort.  It prints
 * 'message' as well as a short apology to the user.
 */
void programming_error(char *message) {
    fprintf(stderr, "--- PNGNQ has encountered an internal programming error ---\n%s\n"
            "We apologise for the inconvenience.  (Please check online for a known fix, or consider filing a "
            "bug report.)\n", message);
    exit(EXIT_FAILURE);
}




/*
 *
 * ============= Function Definitions ==============
 *
 */


/* 
 * palinitnet() initialises our network and global parameters.
 * 
 * - The initial unlocked neurons are distributed along the segmented line from RGBA (0,0,0,0) to (16,16,16,255) then
 * (255,255,255,255), ie. from transparent black to opaque dark grey to opaque white.
 *
 * - Locked palette neurons are sorted in ascending order of sum of component values and intermingled with the unlocked neurons
 * as evenly as practicable.
 *
 * - Precomputation is performed, cache is cleared, etc.
 *
 * Of the parameters:
 *
 * [thepal] should point to a block of rgba packed pixel data, or be NULL if there is no palette.  This block should be
 * num_pal_colours*4 bytes long.  This colour data will not be altered.
 *
 * [num_pal_colours] is the number of colours in the user-supplied palette.  Must be zero if there is no palette.
 * 
 * [gamma_p] is the gamma value for gamma correcting the palette.  Should be > 0 and <= 10.
 *
 * [the_pic] is the input image being quantized, and must point to a block of rgba packed pixel data, containing len bytes.
 * Must not be NULL. This pixel data will not be altered.
 *
 * [len] is as above.
 *
 * [colours] is the total number of colours requested for the output image, including any colours from the user-supplied
 * palette.  Must be in the range [1,256].
 *
 * [i_colour_space] must be RGB or YUV, indicating the internal colour space for neuquant to use.
 * 
 * [gamma_c] is the gamma value to be used for gamma correcting colours internally.  This applies to all gamma correction (and
 * inverse "uncorrection") except the correction (but not uncorrection) of the user supplied palette.  Should be > 0 and <= 10.
 *
 * [i_alpha_extenuation] is the amount to extenuate alpha values of zero and 255 by.  Zero for no extenuation.  Should be in
 * the range [0, 255]
 *
 * [i_force_alpha_class_correctness] should be true to enforce the pngnq -T strict correctness for alpha values of 0 and 255,
 * or false otherwise.
 * 
 * The sens parameters set sensitivity for colour selection and remapping.  Each value must be in the range [0.125,1.0].
 *
 * [i_exclusion_threshold] sets the exclusion threshold used during colour selection.  Must be >= 0.
 *
 * [i_use_alpha_importance_heuristic] turns on the alpha colorimportance heuristic behaviour when it is true, or turns it off
 * otherwise.
 * 
 */
void palinitnet(
    unsigned char *thepal,          /* Pre-determined palette and attributes. */
    unsigned int num_pal_colours,
    double gamma_p,

    unsigned char *thepic,          /* The input image and its length. */
    unsigned int len,

    unsigned int colours,           /* Learning and colour selection parameters. */
    unsigned int i_colour_space,
    double gamma_c, 
    double i_alpha_extenuation,
    unsigned int i_force_alpha_class_correctness,
    
                                    /* Sensitivity settings for colour selection and remap. */
    double i_r_sens, double i_g_sens, double i_b_sens, double i_a_sens,
    double i_remap_r_sens, double i_remap_g_sens, double i_remap_b_sens, double i_remap_a_sens, 
    
    double i_exclusion_threshold,   /* More learning and colour selection parameters. */
    int i_use_alpha_importance_heuristic
    )
{
    /* Point at which to make initial neuron colours fully opaque. */ 
    const int opacity_point = 16.0;
    
    int i;
    
    /* Clear out network from previous runs */
    /* thanks to Chen Bin for this fix */
    memset((void*)network,0,sizeof(network));

    /* Initialise all settings. */
    thepicture = thepic;
    lengthcount = len;
    netsize = colours;
    colour_space = i_colour_space;
    alpha_extenuation = i_alpha_extenuation;
    force_alpha_class_correctness = i_force_alpha_class_correctness;
    
    sensitivity.elems[red] = (float) i_r_sens;
    sensitivity.elems[green] = (float) i_g_sens;
    sensitivity.elems[blue] = (float) i_b_sens;
    sensitivity.elems[alpha] = (float) i_a_sens;    
    reciprocal_sensitivity.elems[red] = (float) (1.0/i_r_sens);
    reciprocal_sensitivity.elems[green] = (float) (1.0/i_g_sens);
    reciprocal_sensitivity.elems[blue] = (float) (1.0/i_b_sens);
    reciprocal_sensitivity.elems[alpha] = (float) (1.0/i_a_sens);
    remap_sensitivity.elems[zero] = i_remap_r_sens;
    remap_sensitivity.elems[one] = i_remap_g_sens;
    remap_sensitivity.elems[two] = i_remap_b_sens;
    remap_sensitivity.elems[alpha] = i_remap_a_sens;

    exclusion_threshold = i_exclusion_threshold;
    nq_colour_vector disaster_base = init_vec(MAX255, MAX255, MAX255, MAX255);
    disaster_threshold = vec_mul(sensitivity, 
                            sca_mul(disaster_factor, disaster_base));
    use_alpha_importance_heuristic = i_use_alpha_importance_heuristic;

    init_yuv_matrices();
    
    if(num_pal_colours > 0) {
        /* Set up gamma function using palette gamma. */
        gamma_correction = gamma_p;
        for(i = 0; i < GAMMA_TABLE_SIZE; i++) {
            double temp;
            temp = pow(i/MAX255, 1.0/gamma_correction) * MAX255;
            temp = round(temp);
            gamma_correction_table[i] = temp;
        }

        /* Set up learning frequency and bias values -- same for every neuron */
        for(i = 0; i < netsize; i++) {
            freq[i] = 1.0/netsize;
            bias[i] = 0;
            repel_points[i] = 0;
        }

        /* Write the palette colour values into the first num_pal_colours nodes.  Lock those nodes. */
        for(i = 0; i < num_pal_colours; i++) {

            // XXX Diagnostic code:
            // fprintf(stderr, "unsorted palette bytes %d: %7u %7u %7u %7u\n", 
            //         i, thepal[(4*i)], thepal[(4*i)+1], thepal[(4*i)+2], thepal[(4*i)+3]);
            
            network[i] = internaliseLearningPixel(
                    thepal[(4*i)], thepal[(4*i)+1], thepal[(4*i)+2], thepal[(4*i)+3]);
                    
            // XXX Diagnostic code:
            // fprintf(stderr, "internalised to %d       : %7.2f %7.2f %7.2f %7.2f\n", 
            //         i, network[i].r, network[i].g, network[i].b, network[i].al);
            // nq_pixel temp = externalise(network[i]);
            // fprintf(stderr, "externalised to %d       : %7.2f %7.2f %7.2f %7.2f\n\n",
            //         i, temp.r, temp.g, temp.b, temp.al);

            /* Lock each neuron.  Locks point to colours from the palette file,
             * which may need to be retrieved later on.  */
            neuronlock[i] = thepal + (4*i);        
          
        }

        /* Sort the network as it stands */
        netsort(num_pal_colours); 
    }

    /* Set up gamma function using image gamma. */
    gamma_correction = gamma_c;
    for(i=0;i<GAMMA_TABLE_SIZE;i++)
    {
        double temp;
        temp = pow(i/MAX255, 1.0/gamma_correction) * MAX255;
        temp = round(temp);
        gamma_correction_table[i] = temp;
    }

    /* Fill in the remaining network nodes so they are unlocked, and at neutral starting colours. 

       We intersperse the new nodes roughly evenly among the palette nodes.  If 25% of the nodes are free, then every 4th node
       will be a free node.
    */ 

    int remaining_colours = netsize - num_pal_colours;
    int step_size = 0; /* How many nodes we step over each iteration. */

    if(num_pal_colours == 0) {
        step_size = 1;
    } else if(remaining_colours == 0 || remaining_colours >= num_pal_colours) {
        step_size = remaining_colours / num_pal_colours;
    } else {
        step_size = num_pal_colours / remaining_colours;
    }

    /* Start at zero, then insert a new neuron at every step.  When the current neuron position gets too big, we wrap it around
     * using the mod operator. */
    int curr_neuron_pos = 0;

    while(remaining_colours != 0) {
        /* Shift every neuron up by one */
        for(i = netsize - remaining_colours - 1; i >= curr_neuron_pos; i--) {
            network[i+1] = network[i];
            neuronlock[i+1] = neuronlock[i];
        }
        
        /* Insert new neuron */
        int initial_value = curr_neuron_pos*MAX255/netsize;
        int initial_alpha;
        if (i < opacity_point) { /*Sets alpha values at 0 for dark pixels.*/
            initial_alpha = (i*MAX255/opacity_point);
        } else {
            initial_alpha = MAX255;
        }
        
        network[curr_neuron_pos] = internaliseLearningPixel(initial_value,
            initial_value, initial_value, initial_alpha);
        neuronlock[curr_neuron_pos] = NULL;
       
        /* Calculate next position */ 
        remaining_colours--;
        curr_neuron_pos = 
            (curr_neuron_pos + 1 + step_size) % (netsize - remaining_colours);
        /* The +1 accounts for the neuron we just inserted. */
    }
        
    /* Clear the inxsearch() cache */
    int j;
    for(i = 0; i < SEARCH_CACHE_SIZE; i++) {
        search_cache[i] = CACHE_EMPTY_ENTRY;
        for(j = 0; j < NQ_COLOUR_NUM_COMPONENTS; j++) {
            search_cache_keys[i][j] = 0;
        }
    }

}


/* toYUV(pix) returns the colour of pix converted to YUVA space.  pix should be a valid RGBA colour vector.
 */
nq_colour_vector toYUV(nq_colour_vector pix) {

    nq_colour_vector ret;

    /* The three vectors to_y_factors, to_u_factors and to_v_factors effectively make up a YUVA colour conversion matrix.  We
     * drop the last row because it's trivial.
     *
     * This code is equivalent to matrix multiplication: */
   
    ret.elems[yy] = sum_com(vec_mul(to_y_factors,pix));
    ret.elems[uu] = sum_com(vec_mul(to_u_factors,pix));
    ret.elems[vv] = sum_com(vec_mul(to_v_factors,pix));
    ret.elems[alpha] = pix.elems[alpha];

    /* Offset u and v to positive range */
    ret.elems[uu] += MAX255*u_offset_fraction;  
            /* u now ranges about 0 to 0.88*MAX255 */
    ret.elems[vv] += MAX255*v_offset_fraction;
            /* v now ranges about 0 to 1.23*MAX255 */

    /* Scale u and v so they range from 0 to MAX255 */
    ret = vec_mul(to_yuv_scale, ret);

    return ret;
}


/* fromYUV(pix) returns the colour of pix converted to RGBA space.  pix should be a valid YUVA colour vector.
 */
nq_colour_vector fromYUV(nq_colour_vector pix) {
    
    nq_colour_vector ret;
    nq_colour_vector intermediate;

    /* We have stretched and offset U and V to range from MINZERO to MAX255 instead of their usual range.  Return them to their
       regular range: */
    intermediate = vec_mul(from_yuv_unscale, pix);

    intermediate.elems[uu] -= MAX255*u_offset_fraction;
    intermediate.elems[vv] -= MAX255*v_offset_fraction;

    /* These three vectors to_r_factors, to_g_factors and to_b_factors effectively make up a YUVA -> RGBA colour conversion
     * matrix.  We drop the last row because alpha's trivial.  This code does a matrix multiply:
     */
    
    ret.elems[red] = sum_com(vec_mul(to_r_factors,intermediate));
    ret.elems[green] = sum_com(vec_mul(to_g_factors, intermediate));
    ret.elems[blue] = sum_com(vec_mul(to_b_factors, intermediate));
    ret.elems[alpha] = pix.elems[alpha];
    
    return ret;
}

/* un_gamma_correct(x), where x is a colour component value, returns x after reversing its gamma correction and clamping its
 * range to [MINZERO, MAX255].
 */
static float un_gamma_correct(float temp) {

    if(temp < MINZERO) {
        return MINZERO;
    }
    
    temp = pow(temp/MAX255, gamma_correction) * MAX255;    

    if(temp > MAX255) {
        return MAX255;
    }

    return temp;
}

/* gamma_correct(x), where x is a colour component value, returns x gamma corrected.  Note this function is pre-computed and
 * relies on gamma_correction_table being filled in by palinitnet() or initnet().
 */
inline static float gamma_correct(unsigned int temp) {    
    return gamma_correction_table[temp];
}

/* internalise(r,g,b,a) re-encodes the RGBA colour described by r, g, b, a into pngnq's internal colour space, and returns that
 * encoding as an nq_colour_vector.  r, g, b and a should be integers in the range [0,255], otherwise no sensible result is
 * guaranteed.
 *
 * Internalising involves the following (potentially optional) steps:
 * - gamma correcting non-alpha components
 * - exaggerating (or extenuating) alpha values
 * - changing the colour coordinate space to YUV
 * - desensitising components (ie scaling them down by separate constants).
 */
nq_colour_vector internaliseLearningPixel(int r_in, int g_in, int b_in, int al_in) {
    
    nq_colour_vector intermediate;

    intermediate.elems[red] = gamma_correct(r_in);
    intermediate.elems[green] = gamma_correct(g_in);
    intermediate.elems[blue] = gamma_correct(b_in);
    intermediate.elems[alpha] = al_in;
    
    intermediate = alpha_exaggerate(intermediate);

    switch(colour_space) {
        case RGB:
            break;

        case YUV:
            intermediate = toYUV(intermediate);
            break;

        default:
            programming_error("internalise() is using an unknown colour "
                " space.\n");
            break;
    }

    intermediate = desensitise(intermediate);
    return intermediate;

}


/* round_clamp(x) rounds x to an integer value and then clamps it in the range [MINZERO, MAX255]. 
 */
inline float round_clamp(float x) {
    x = round(x);
    if(x < MINZERO) {
        return MINZERO;
    } else if(x > MAX255) {
        return MAX255;
    } else {
        return x;
    }
}

/* round_clamp_colour_vec(pix) applies round_clamp to each component of pix, thus returning each to the range [MINZERO,
 * MAX255]. 
 */
nq_colour_vector round_clamp_colour_vec(nq_colour_vector pix) {
    pix.elems[red] = round_clamp(pix.elems[red]);
    pix.elems[green] = round_clamp(pix.elems[green]);
    pix.elems[blue] = round_clamp(pix.elems[blue]);
    pix.elems[alpha] = round_clamp(pix.elems[alpha]);

    return pix;
}

/* sensitised_round_clamp(x, sens) rounds x to an integer value and then clamps it in the range [MINZERO, round(sens*MAX255)].
 */
static inline float sensitised_round_clamp(float x, float sens) {
    x = round(x);
    if(x < MINZERO) {
        return MINZERO;
    } else if(x > round(sens*MAX255)) {
        return round(sens*MAX255);
    } else {
        return x;
    }
}



/* desensitise(pix) returns a copy of pix having scaled each component value of pix separately according to the sensitivity
 * scale factors.  The range of ordinary values any given component may occupy is reduced from [MINZERO, MAX255] to [MINZERO,
 * S*MAX255] where S the relevant component factor specified in the variable 'sensitivity'.*/ 
nq_colour_vector desensitise(nq_colour_vector pix) {
    pix = vec_mul(pix, sensitivity);
    return pix;
}


/* resensitise(pix) returns a copy of pix, having scaled each component values in such a way as to reverse the effect of
 * desensitise(pix).  (Of course, both functions can involve the loss of precision of float values.) The range of ordinary
 * values any given component may occupy is increased from [MINZERO, S*MAX255] to [MINZERO, MAX255] where S the relevant
 * component factor specified in the variable 'sensitivity'.*/ 
nq_colour_vector resensitise(nq_colour_vector pix) {
    pix = vec_mul(pix, reciprocal_sensitivity);
    return pix;
}

/* alpha_exaggerate(pix) returns a copy of pix with its alpha value potentially exaggerated.  alpha of zero is remapped to
 * MINZERO - alpha_extenuation, while 255 remaps to MAX255 + alpha_extenuation.  Note that alpha_exaggerate is
 * operating on fully sensitised values only. */
nq_colour_vector alpha_exaggerate(nq_colour_vector pix) {
    if(pix.elems[alpha] >= MAX255) {
        pix.elems[alpha] += alpha_extenuation; 
    } else if(pix.elems[alpha] <= MINZERO) {
        pix.elems[alpha] -= alpha_extenuation;
    }

    return pix;
}

/* alpha_unexaggerate(pix) returns a copy of pix with any effect of alpha_exaggerate(pix) reversed. 
 */
nq_colour_vector alpha_unexaggerate(nq_colour_vector pix) {
    if(pix.elems[alpha] > MAX255) {
        pix.elems[alpha] = MAX255; 
    } else if(pix.elems[alpha] < MINZERO) {
        pix.elems[alpha] = MINZERO;
    }
    return pix;
}
    

/* externalise(pix) reverses internalisation.
 * It:
 * - Resensitises to full sensitivity
 * - Converts pixel back to conventional rgb colour space
 * - Removes any alpha extenuation (exaggeration).
 * - Removes any gamma correction
 * - Clamps component values to the range [0,255]
 */
nq_colour_vector externalise(nq_colour_vector internal_pix) {

    nq_colour_vector intermediate = internal_pix;

    intermediate = resensitise(internal_pix);

    switch(colour_space) {
        case RGB:
            break;

        case YUV:
            intermediate = fromYUV(intermediate);
            break;

        default:
            programming_error("externalise() is using an unknown colour space.\n");
            break;
    }

    intermediate = alpha_unexaggerate(intermediate);
    
    intermediate.elems[red] = un_gamma_correct(intermediate.elems[red]);
    intermediate.elems[green] = un_gamma_correct(intermediate.elems[green]);
    intermediate.elems[blue] = un_gamma_correct(intermediate.elems[blue]);

    intermediate = round_clamp_colour_vec(intermediate);

    return intermediate;
}





/* getcolormap(map, strict_pal_rgba) writes the colour palette selected by our neural network into the block of memory pointed
 * at by map, and externalises and re-internalises neuron values using new sensitivity settings for the remapping phase calls
 * to inxsearch().  The colours are written in RGBA format, with one unsigned byte for each component, each in the range
 * [0,255].  If strict_pal_rgba is true, then if any of the selected colours were provided by the user as a locked palette, the
 * RGBA bytes for those colours will be copied directly from the source, instead of the network, where certain settings may
 * have altered them.  The caller is responsible for allocating and freeing map, which must be at least 4*netsize bytes in
 * size.
 */
void getcolormap(unsigned char *map, int strict_pal_rgba) {

    unsigned int j;
    nq_colour_vector externalised[netsize];

    /* Externalise each neuron's colour, and save a copy for recalibration. */
    for(j = 0; j < netsize; j++) {
        nq_colour_vector intermediate = externalise(network[j]);
        externalised[j] = intermediate;
        if(!strict_pal_rgba || !neuronlock[j]) { 
            /* This is the normal case, taking the map value from the network.*/
            *map++ = intermediate.elems[red];
            *map++ = intermediate.elems[green];
            *map++ = intermediate.elems[blue];
            *map++ = intermediate.elems[alpha];

        } else {
            /* However, if we are strictly retaining palette file rgb values, and neuron j is a locked palette neuron, then we
             * directly copy the original rgba values rather using the neuron's values. */

            *map++ = *(neuronlock[j]); /* neuronlock points at the...*/
            *map++ = *(neuronlock[j]+1); /*...palette file pixel data. */ 
            *map++ = *(neuronlock[j]+2);
            *map++ = *(neuronlock[j]+3);
        }

    }

    /* Recalibrate sensitivity, and re-internalise every network node. */
    sensitivity = remap_sensitivity;
    reciprocal_sensitivity.elems[zero] = (float) (1.0/remap_sensitivity.elems[zero]);
    reciprocal_sensitivity.elems[one] = (float) (1.0/remap_sensitivity.elems[one]);
    reciprocal_sensitivity.elems[two] = (float) (1.0/remap_sensitivity.elems[two]);
    reciprocal_sensitivity.elems[alpha] = (float) (1.0/remap_sensitivity.elems[alpha]);

    for(j = 0; j < netsize; j++) {
        network[j] = internaliseLearningPixel(externalised[j].elems[red], externalised[j].elems[green],
                                              externalised[j].elems[blue], externalised[j].elems[alpha]);
    }
        
}




/* netsort(currnetsize) sorts network, assuming it contains valid neurons  from 0 to currnetsize-1.  This is a simple insertion
 * sort indexing on the sum of the components (eg r+g+b+al) in ascending order.  The neuronlock array is sorted in coordination
 * so that network[i] and neuronlock[i] continue to correspond. */
void netsort(int currnetsize)
{
    unsigned int i,j,smallpos;
    double smallval;

    for (i=0; i<currnetsize; i++) {
        
        /* Identify smallest item in remaining unsorted part of array.*/

        smallpos = i;
        smallval = sum_com(network[i]);

        for (j=i+1; j<currnetsize; j++) {
            double candval = sum_com(network[j]);
            if(candval < smallval) {
                smallpos = j;
                smallval = candval;
            }
        }

        /* swap network[i] and network[smallpos] entries */
        if (i != smallpos) {
            nq_colour_vector temp = network[smallpos];
            network[smallpos] = network[i];
            network[i] = temp;
            
            unsigned char* templ = neuronlock[smallpos];
            neuronlock[smallpos] = neuronlock[i];
            neuronlock[i] = templ;
        }
    }
}



/* get_hash(r,g,b,a) returns a 16 bit hash value for a 32 bit RGBA colour suitable for use by inxsearch().  Each of r, g, b,
 * and a should be in the range [0,255].
 *
 * For the hash value, we distribute the 32 bits of colour evenly across the 16 bits of hash as indicated below, so that each
 * hash bit is the XOR of just two colour bits.  The bits are aligned so that it is unlikely that two colours nearby in RGBA
 * space will clash in the cache.
 *
 *    a3 a2 a1 a0   -- -- -- --   -- -- -- --   a7 a6 a5 a4
 *    -- -- -- --   b7 b6 b5 b4   b3 b2 b1 b0   -- -- -- --
 *    g7 g6 g5 g4   g3 g2 g1 g0   -- -- -- --   -- -- -- --
 *    -- -- -- --   -- -- -- --   r7 r6 r5 r4   r3 r2 r1 r0
 */
inline static unsigned int get_hash(unsigned int r, unsigned int g, unsigned int b, 
        unsigned int a) {

    return  (   (   ((a << 12) & 0xf000)
                |   ((a >>  4) & 0x000f)
                )
            ^   (b << 4)
            ^   (g << 8)
            ^   r
            );
}


/* inxsearch(al,b,g,r) searches our neural network to find the neuron whose colour is closest to the RGBA colour specified by
 * al, b, g and r.  Those values are assumed to range [0,255].  Colour distance is calculated as a weighted cartesian distance
 * subject to all the internal colour processing settings, such as conversion to a different colour space and sensitivity
 * adjustment.  The index of the best matching colour is returned, in the range [0, netsize-1].
 *
 * inxsearch caches its results in a very simple hash table,  search_cache[].  At the time of coding, casual profiling
 * suggested that on normal pictures for the human eye, caching cut inxsearch()'s CPU time by at least 2/3rds relative to doing
 * a simple brute force search every time.
 */
unsigned int inxsearch( int al, int b, int g, int r)
{
    static int have_warned_alpha_class = false; /* XXX Ungainly way to prevent warning multiple times. */
    unsigned int i,best=0;
    double bestd = MAXDIFF;
    double dist;
   

    /* Check the cache */

    unsigned int hash_value = get_hash(r,g,b,al);
    int network_candidate = search_cache[hash_value];
        /* If the cache entry is valid and our colour matches the cache's key, then we have a cache hit and can just return
         * that value.  Yay!
         */
    if(         network_candidate != CACHE_EMPTY_ENTRY
            &&  search_cache_keys[hash_value][red] == r
            &&  search_cache_keys[hash_value][green] == g
            &&  search_cache_keys[hash_value][blue] == b
            &&  search_cache_keys[hash_value][alpha] == al
    ) {
        return (unsigned int) network_candidate;     
    }


    /* Manually find the nearest colour in the network instead. */
    
    nq_colour_vector intermediate;
    intermediate = internaliseLearningPixel(r,g,b,al); 

    /* Precompute difference between target colour and every colour in the network, as well as that difference squared. */
    nq_colour_vector diffs[netsize];
    float dists[netsize];
    nq_colour_vector context_weighting = calc_context_weighting(intermediate);

    int vdx;
    for(vdx = 0; vdx < netsize; vdx++) {
        diffs[vdx] = vec_mul(context_weighting, vec_sub(network[vdx], intermediate)); 
        dists[vdx] = sum_com(vec_mul(diffs[vdx], diffs[vdx]));
    }

    int do_alpha_check =    (   force_alpha_class_correctness 
                            &&  (   al == MAX255 
                                ||  al == MINZERO
                                )
                            ); 

    for(i=0; i < netsize; i++) {

        dist = dists[i];

        /* When required, use only map colours that have the same alpha class as the input pixel. */
        if(do_alpha_check) {
            if(   sensitised_round_clamp(network[i].elems[alpha], sensitivity.elems[alpha])
               != sensitised_round_clamp(intermediate.elems[alpha], sensitivity.elems[alpha])) {
                /* Values in network might be unrounded or
                 * out of range, so they must be clamped and rounded for
                 * this exacting alpha comparison */
                dist = MAXDIFF;
            }
        } 
 
        if(dist<bestd) {
            bestd = dist;
            best = i;
        }        
    }

    /* Emit a warning if we cannot force this pixel to keep a perfect 0 or 255 alpha value.  (If this is the case, bestd will
     * be the maximum value and i should still be zero.  In any other case a better bestd should be chosen, so the condition
     * below can be simplified.)
     */
    if(     do_alpha_check 
        &&  !have_warned_alpha_class
        &&  bestd == MAXDIFF 
        &&  (   sensitised_round_clamp(intermediate.elems[alpha], sensitivity.elems[alpha]) 
            !=  sensitised_round_clamp(network[best].elems[alpha], sensitivity.elems[alpha])
            ) 
    ) {
        fprintf(stderr, 
            "PNGNQ WARNING: Unable to force alpha class correctness.\n"
            "               Not all alpha values of 0 and 255 will be retained in output.\n");
        have_warned_alpha_class = true;
    }

    /* Place our result in the cache */
    search_cache[hash_value] = (unsigned char) best;
    search_cache_keys[hash_value][red] = r;
    search_cache_keys[hash_value][green] = g;
    search_cache_keys[hash_value][blue] = b;
    search_cache_keys[hash_value][alpha] = al;

    return best;
}



/* age_freq_and_bias(i) decays the bias and freq measures for neuron i
 */
static inline void age_freq_and_bias(int i) {
    double betafreq = freq[i] / (1<< betashift);
    freq[i] -= betafreq;
    bias[i] += betafreq * (1<<gammashift);
} 


/* contest(pix) finds the neuron in network[] that is the best choice for learning the colour of pix and returns its index.  It
 * also updates (ages) the bias and frequency tables used to identify over- and under- utilised neurons.
 *
 * Usually the 'best' choice is the closest neuron to pix, with closeness determined by a weighted component-wise Manhattan
 * distance in the internalised colour space.  However, the learning procedure also counts how frequently particular neurons
 * are being used (trained), and uses a bias factor to select under-utilised neurons, even when they are not the very closest
 * to pix.
 *
 * As a final complication, if the match is perfect (ie. under the exclusion_threshold in each component), contest will return
 * (-index -1) to flag this.
 */
int contest(nq_colour_vector target_pix) 
{

    /* Calculate the component-wise differences between target_pix colour and every colour in the network, and weight according
     * to component relevance.
     */
    nq_colour_vector diffs[netsize];
    nq_colour_vector context_weighting = calc_context_weighting(target_pix);

    int vdx;
    for(vdx = 0; vdx < netsize; vdx++) {
        diffs[vdx] = vec_mul(context_weighting,
                             vec_sub(network[vdx], target_pix)); 
    }

    /* Now we find the best match using various measures of distance and utilisation. */

    unsigned int i;
    double dist; /* The distance between target_pix colour and current neuron colour. */
    unsigned int bestpos; /* Index of best neuron so far. */
    unsigned int bestbiaspos; /* Index of neuron with best distance-bias combination so far. */
    double bestd; /* Distance to target_pix for best neuron so far.*/
    double bestbiasd; /* Best distance-bias combination so far. */
    int perfect = false; /* Is bestpos a perfect match for the colour. */ 
    
    bestd = MAXDIFF;
    bestbiasd = bestd;
    bestpos = 0;
    bestbiaspos = bestpos;
    
    /* This loop finds best neuron.  Our definition of 'best' is either a perfect match, or failing that, the neuron with the
     * minimum 'dist minus bias'.  
     * 
     * Our definition of a perfect match is that the difference in each component is under exclusion_threshold in magnitude.  
     *
     * 'dist' is basically the manhattan distance between the neuron colour, but there are some special cases as commented
     * below.  'bias' is calculated per-neuron from freq, a count of how frequently the neuron has been taught that decays over
     * time, using the formula bias[i] = gamma*((1/netsize)-freq[i]).  For frequently chosen neurons, freq[i] is high and
     * bias[i] is negative.  That means infrequently taught neurons are more likely to be chosen.  See Dekker's Neuquant paper
     * for precise details about bias and freq.
     *
     */
    
    for(i = 0; i < netsize; i++) {

        if(!perfect) { 
            double bestbiasd_biased = bestbiasd + bias[i];
            
            /* Detect perfect matches. */
            if  (   ABS(diffs[i].elems[alpha]) < exclusion_threshold
                &&  ABS(diffs[i].elems[red]) < exclusion_threshold
                &&  ABS(diffs[i].elems[green]) < exclusion_threshold
                &&  ABS(diffs[i].elems[blue]) < exclusion_threshold
                ) {
                perfect = true;
                dist = DIFFZERO;

            /* If we don't have a perfect match, the distance between the
             * target and the neuron is the manhattan distance. */
            } else {
                dist =      ABS(diffs[i].elems[alpha])
                        +   ABS(diffs[i].elems[red])
                        +   ABS(diffs[i].elems[green])
                        +   ABS(diffs[i].elems[blue]);
            }
            
            /* See if the current neuron is better. */
            if (dist < bestd) {
                bestd = dist;
                bestpos = i;
            }
            if (dist < bestbiasd_biased) {
                bestbiasd = dist - bias[i];
                bestbiaspos = i;
            }
        }

        /* Age (decay) the current neurons bias and freq values. */
        age_freq_and_bias(i);
    } 

    /* Increase the freq and bias values for the chosen neuron. */
    freq[bestpos] += beta;
    bias[bestpos] -= betagamma;

    /* If our bestpos pixel is a 'perfect' match, we return bestpos, not bestbiaspos.  That is, we only decide to look at
     * bestbiaspos if the current target pixel wasn't a good enough match with the bestpos neuron, and there is some hope that
     * we can train the bestbiaspos neuron to become a better match. */
    if(perfect) {
        return -bestpos -1;  /* flag this was a perfect match */
    } else {
        return bestbiaspos;
    }
}


/* posneg(x) returns +1 if x is positive or zero, or -1 otherwise.
 * 
 * XXX Posneg could be turned into a vector function in various ways.
 */
inline double posneg(double x) {
    if(x < 0) {
        return -1.0;
    } else {
        return 1.0;
    }
}

/* repelcoincident(i) traverses the entire neural network, identifies any neurons that are close in colour to neuron i, and
 * moves them away from neuron i.  Our definition of 'close' is being within exclusion_threshold of neuron i in each component.  
 *
 * Because repelcoincident() can be cpu-intensive, each neuron is given repel points (in the repel_points array) to limit how
 * often a full repel pass is made.  Each time neuron i gets a full repel pass, it gets REPEL_STEP_UP points.  When the number
 * of points it has exceeds REPEL_THRESHOLD, we stop doing full repel passes, and deduct REPEL_STEP_DOWN points instead.
 * Eventually the number of repel points will eventually oscillate around the threshold.  With current settings, that means
 * that only every 4th function call will result in a full pass.
 */
void repelcoincident(int i) {
    /* Use brute force to precompute the distance vectors between our neuron and each neuron. */

    if(repel_points[i] > REPEL_THRESHOLD) {
        repel_points[i] -= REPEL_STEP_DOWN;
        return;
    }

    nq_colour_vector diffs[netsize];
    nq_colour_vector context_weighting = calc_context_weighting(network[i]);

    int vdx;
    for(vdx = 0; vdx < netsize; vdx++) {
        diffs[vdx] = vec_mul(context_weighting, vec_sub(network[vdx], network[i])); 
    }


    /* Identify which neurons are too close to neuron[i] and shift them away.
     * */
    
    /* repel_step is the amount we move the neurons away by in each component.
     * radpower[0]/alpharadbias is similar to the proportion alterneigh uses.
     * */
    double repel_step = exclusion_threshold*(radpower[0] / alpharadbias);
    int j;

    for(j = 0; j < netsize; j++) {
        /* Don't repel i from itself or any locked neuron. */
        if(j == i || neuronlock[j]) {
            continue;
        }
       
        if  (   ABS(diffs[j].elems[zero]) < exclusion_threshold
            &&  ABS(diffs[j].elems[one]) < exclusion_threshold
            &&  ABS(diffs[j].elems[two]) < exclusion_threshold
            &&  ABS(diffs[j].elems[three]) < exclusion_threshold
            ) {

            nq_colour_vector repel_vec = 
                vec_mul(context_weighting,
                        init_vec(   posneg(diffs[j].elems[zero]), posneg(diffs[j].elems[one]), posneg(diffs[j].elems[two]),
                                    posneg(diffs[j].elems[three])
                        )
                );

            repel_vec = sca_mul(repel_step, repel_vec);
            network[j] = vec_add(network[j], repel_vec);
        }
    }

    repel_points[i] += REPEL_STEP_UP; 
}


/* altersingle(learning_alpha, unisolate, i, target_colour) moves neuron i towards target_colour by a factor of learning_alpha
 * (commonly called the learning rate).  If neuron i and the target colour differ by more than the diaster_threshold value in
 * any component, then the learning rate will be multiplied by unisolate+1 to move the neuron more forcefully.  The special
 * cases when alpha or Y are zero are handled sensibly. */

static void altersingle(double learning_alpha,double unisolate,unsigned int i, nq_colour_vector target_vec) {    

    /* If the neuron is locked, do nothing! */
    if(neuronlock[i]) {
        return;
    }

    float learning_factor = learning_alpha / initalpha;
    nq_colour_vector context_weighting = calc_context_weighting(target_vec); 
    nq_colour_vector diff = vec_mul(context_weighting, vec_sub(network[i], target_vec));

    /* In each case below, we check for a disastrous colour error, and if one is detected, multiply (unisolate+1) into the
     * learning_factor before updating neuron i.
     */

    if  (   (ABS(diff.elems[zero]) > disaster_threshold.elems[zero]) 
        ||  (ABS(diff.elems[one]) > disaster_threshold.elems[one]) 
        ||  (ABS(diff.elems[two]) > disaster_threshold.elems[two]) 
        ||  (ABS(diff.elems[three]) > disaster_threshold.elems[three]) 
        )   {
        learning_factor *= (1.0f + unisolate);
        if(learning_factor > 1.0f) {
            learning_factor = 1.0f;
        }
    }
    
    network[i] = vec_sub(network[i], sca_mul(learning_factor, diff)); 
}




/* alterneigh(rad, i, target_colour) trains the neighbours of neuron i towards target_colour.  The exact neighbours in neuron
 * space are those members of network[] whose indices fall in the range [i-rad, i+rad] excluding neuron i.  The learning rate
 * used for the neigbours is decreased as their indices move further away from i, according to the precomputed formula in the
 * array radpower.
 *
 * The special cases when alpha or Y are zero are handled sensibly.  Explicit variations on the main loop have been provided
 * since they were not adequately optimised by the development compiler.
 */
static void alterneigh(unsigned int rad, unsigned int i, nq_colour_vector target_colour)
{
    double *learning_rate_table;
    double learning_rate;

    /* Calculate bottom and top neuron indices. */
    int lo = i-rad;   if (lo<0) lo=0;
    int hi = i+rad;   if (hi>netsize-1) hi=netsize-1;

    /* Set up low (k) and high (j) iterators. We iterate from the inside out. */
    int j = i+1;
    int k = i-1;
    learning_rate_table = radpower;

    nq_colour_vector context_weighting = calc_context_weighting(target_colour);
    
    while ((j<=hi) || (k>=lo)) {
        learning_rate = (*(++learning_rate_table)) / alpharadbias;
        if (j<=hi) {
            if(!neuronlock[j]) {
                network[j] = vec_sub(network[j], sca_mul(learning_rate, 
                    vec_mul(context_weighting, vec_sub(network[j], target_colour))));
            }
            j++;
        }
        if (k>=lo) {
            if(!neuronlock[k]) {
                network[k] = vec_sub(network[k], sca_mul(learning_rate, 
                    vec_mul(context_weighting, vec_sub(network[k], target_colour))));
            }
            k--;
        }
    }
}


/* learn(samplefac, unisolate, verbose) executes the main learning cycles of Neuquant.  A normal Neuquant learning phase will
 * sample 1 in samplefac pixels from the input image.  Other versions of Neuquant have only one learning phase, but in this
 * version there are two.  So this version will sample twice as many pixels for the same n value.
 *
 * samplefac must be in the range [1,30].  Set verbose to true for informational messages to be written to stderr during
 * execution. 
 */
void learn(unsigned int samplefac, double unisolate, unsigned int verbose) 
{
    unsigned int i;
    int j;
    unsigned int rad,step,delta,samplepixels;
    double radius,learning_alpha;
    unsigned char *p;
    unsigned char *lim;

    alphadec = initialalphadec + ((samplefac-1)/alphadecsamplefacdivisor);
    p = thepicture;
    lim = thepicture + lengthcount;
    samplepixels = lengthcount/(4*samplefac); 
    delta = samplepixels/ncycles;  /* here's a problem with small images: samplepixels < ncycles => delta = 0 */
    if(delta==0) delta = 1;        /* kludge to fix */
    learning_alpha = initalpha;
    radius = initradius;
    if(netsize <= 2*initradius) {
        radius = netsize/2;
    }
    
    rad = radius;
    if (rad <= 1) rad = 0;
    for (i=0; i<rad; i++) 
        radpower[i] = floor( learning_alpha*(((rad*rad - i*i)*radbias)/(rad*rad)) );
    
    if(verbose) fprintf(stderr,"beginning 1D learning: initial radius=%d\n", rad);

    if ((lengthcount%prime1) != 0) step = 4*prime1;
    else {
        if ((lengthcount%prime2) !=0) step = 4*prime2;
        else {
            if ((lengthcount%prime3) !=0) step = 4*prime3;
            else step = 4*prime4;
        }
    }
   
    /* pngnq-s9 learns for longer than pngnq in the reasonable hope that the last few colours will be better chosen.
     *
     * By default, we learn for two times as long as the old pngnq.  But when we are only choosing a small number of colours, we
     * may learn for up to six times as long.
     */ 
    int learning_extension;
    if(netsize < extra_long_colour_threshold) {
        learning_extension = 2 + ((extra_long_colour_threshold-netsize)/extra_long_divisor);
    } else {
        learning_extension = normal_learning_extension_factor;
    }

    i = 0;
    while (i < learning_extension*samplepixels) /* Two phases of samplepixels each. */
    {
        nq_colour_vector intermediate;
        if (p[3]) 
        {    intermediate = internaliseLearningPixel(p[0], p[1], p[2], p[3]);
        }
        else
        {   intermediate = internaliseLearningPixel(0,0,0,0);
        }
        
        /* We find the best (nearby, underutilised) neuron to the target colour and train it towards the target colour.  If it
         * was a near-perfect match, we then repel its neighbours in colour space.  Otherwise, we also train its neighbours in
         * neuron space towards the target colour.  */
        j = contest(intermediate);
        
        /* Determine if the colour was a perfect match.  j contains a factor encoded boolean. Horrible code to extract it. */
        int was_perfect = (j < 0);
        j = (j < 0 ? -(j+1) : j);

        altersingle(learning_alpha,unisolate,j,intermediate);

        if(rad && !was_perfect) {
            alterneigh(rad,j,intermediate);
        }  else if(rad && was_perfect) {
            repelcoincident(j);  /* repel neighbours in colour space */
        }

        /* Identify the next target colour and decrease the learning rates for altersingle, repelcoincident and alterneigh. */
        p += step;
        while (p >= lim) p -= lengthcount;
    
        i++;
        if (i%delta == 0) {                    /* FPE here if delta=0*/ 
            /* We decay learning_alpha at half the rate of regular Neuquant so that the second learning phase still has a
             * viable learning rate.  */ 
            learning_alpha -= learning_alpha / (learning_extension*(double)alphadec);
            radius -= radius / (double)radiusdec;
            rad = radius;
            if (rad <= 1) rad = 0;
            for (j=0; j<rad; j++) { 
                radpower[j] = floor( 
                    learning_alpha*(((rad*rad - j*j)*radbias)/(rad*rad)) );
            }
        }
    }

    if(verbose) {
         fprintf(stderr,"finished 1D learning: final alpha=%f !\n",
                ((float)learning_alpha)/initalpha);
    }

}

