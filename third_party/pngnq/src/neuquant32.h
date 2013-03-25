/*  
 * pngnq-s9: neuquant32.h
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
 * neuquant32.h -- Implements a modified NeuQuant colour selection
 *                 procedure.
 *
 * Contributors: Dekker, Coyle, Lesiński, Pellas-Rice.
 *
 * ------------------------------------------------------------------------
 * 
 */

#include <stdio.h>
#include <string.h>


#define MAXNETSIZE	256    /* maximum number of colours that can be used. 
				  actual number is now passed to initcolors */


/* four primes near 500 - assume no image has a length so large */
/* that it is divisible by all four primes */
#define prime1		499
#define prime2		491
#define prime3		487
#define prime4		503

#define minpicturebytes	(4*prime4)		/* minimum size for input image */

/* Colour space constants */
#define RGB     1
#define YUV     3

/* Initialise network with a partially fixed palette plus freely learning nodes. 
   ----------------------------------------------------------------------- */
void palinitnet(unsigned char *thepal,unsigned int num_pal_colours,double gamma_p,
     unsigned char *thepic,unsigned int len,unsigned int colours,
     unsigned int i_colour_space, double gamma_c, 
     double i_alpha_class_correction, unsigned int i_force_alpha_class_correctness,
     double i_r_sens, double i_g_sens, double i_b_sens, double i_a_sens,
     double i_remap_r_sens, double i_remap_g_sens, double i_remap_b_sens, 
     double i_remap_a_sens, double i_exclusion_threshold,
     int i_use_alpha_importance_heuristic);	

/* Unbias network to give byte values 0..255 and record position i to prepare for sort
   ----------------------------------------------------------------------------------- */
static inline float biasvalue(unsigned int temp);

/* Output colour map and prepare network colous for remapping
   ----------------- */
void getcolormap(unsigned char *map, int strict_pal_rgba);

/* Search for ABGR values 0..255 (after net is unbiased) and return colour index
   ---------------------------------------------------------------------------- */
unsigned int inxsearch( int al,  int b,  int g,  int r);

/* Main Learning Loop
   ------------------ */
void learn(unsigned int samplefactor, double unisolate, unsigned int verbose);

/* Program Skeleton
   ----------------
   	[select samplefac in range 1..30]
   	pic = (unsigned char*) malloc(4*width*height);
   	[read image from input file into pic]
	palinitnet(pic,4*width*height,samplefac,colors);
	learn();
	getcolormap();
	[write output image header, using writecolourmap(f),
	possibly editing the loops in that function]
	[write output image using inxsearch(a,b,g,r)]		*/
