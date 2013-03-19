/* NeuQuant Neural-Net Quantization Algorithm Interface
 * ----------------------------------------------------
 *
 * Copyright (c) 1994 Anthony Dekker
 *
 * NEUQUANT Neural-Net quantization algorithm by Anthony Dekker, 1994.
 * See "Kohonen neural networks for optimal colour quantization"
 * in "Network: Computation in Neural Systems" Vol. 5 (1994) pp 351-367.
 * for a discussion of the algorithm.
 * See also  http://members.ozemail.com.au/~dekker/NEUQUANT.HTML
 *
 * Any party obtaining a copy of these files from the author, directly or
 * indirectly, is granted, free of charge, a full and unrestricted irrevocable,
 * world-wide, paid up, royalty-free, nonexclusive right and license to deal
 * in this software and documentation files (the "Software"), including without
 * limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 */

/* Modified to quantize 32bit RGBA images for the pngnq program.
 * Also modified to accept a numebr of colors arguement. 
 * Copyright (c) Stuart Coyle 2004-2006
 */

/*
 * Rewritten by Kornel Lesiński (2009)
 * Euclidean distance, color matching dependent on alpha channel 
 * and with gamma correction. code refreshed for modern compilers/architectures:
 * ANSI C, floats, removed pointer tricks and used arrays and structs.
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


/* Initialise network in range (0,0,0,0) to (255,255,255,255) and set parameters
   ----------------------------------------------------------------------- */
void initnet(unsigned char *thepic, unsigned int len, unsigned int colours, double gamma);
		
/* Unbias network to give byte values 0..255 and record position i to prepare for sort
   ----------------------------------------------------------------------------------- */
static inline double biasvalue(unsigned int temp);

/* Output colour map
   ----------------- */
void getcolormap(unsigned char *map);

/* Insertion sort of network and building of netindex[0..255] (to do after unbias)
   ------------------------------------------------------------------------------- */
void inxbuild();

/* Search for ABGR values 0..255 (after net is unbiased) and return colour index
   ---------------------------------------------------------------------------- */
unsigned int inxsearch( int al,  int b,  int g,  int r);
unsigned int slowinxsearch( int al, int b, int g, int r);

/* Main Learning Loop
   ------------------ */
void learn(unsigned int samplefactor, unsigned int verbose);

/* Program Skeleton
   ----------------
   	[select samplefac in range 1..30]
   	pic = (unsigned char*) malloc(4*width*height);
   	[read image from input file into pic]
	initnet(pic,4*width*height,samplefac,colors);
	learn();
	unbiasnet();
	[write output image header, using writecolourmap(f),
	possibly editing the loops in that function]
	inxbuild();
	[write output image using inxsearch(a,b,g,r)]		*/
