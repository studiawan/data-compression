#include <stdio.h>
#include <unistd.h>
#include "idc.h"



/**********************************************************************
*                                                                      *
*  File: uqimg_dec.c                                                   *
*  Function:  reconstructs an image compressed using uqimg_enc.        *
*  Author  : K. Sayood                                                 *
*  Last mod: 5/12/95                                                   *
*  Usage:  see usage(), for details see uqimg_dec.doc or the man page. *
*                                                                      *
***********************************************************************/


/*******************************************************************************
*NOTICE:                                                                       *
*This code is believed by the author to be bug free.  You are free to use and  *
*modify this code with the understanding that any use, either direct or        *
*derivative, must contain acknowledgement of its author and source.  The author*
*makes no warranty of any kind, expressed or implied, of merchantability or    *
*fitness for a particular purpose.  The author shall not be held liable for any*
*incidental or consequential damages in connection with or arising out of the  *
*furnishing, performance, or use of this software.  This software is not       *
*authorized for use in life support devices or systems.                        *
********************************************************************************/

void usage ();

void main(int argc, char **argv)

   {int row, col, row_size, col_size;
    char output_image[50], qimage[50];
    int *bound, *reco, pixel, label, count, numlev, numbits;
    int minvalue, maxvalue, range, level, stepsize, *buffer, c;
    FILE *ifp, *ofp;
    extern int optind;
    extern char * optarg;



    fprintf(stderr,"\n\n\t\tUniform Quantization of Images - Decoder\n");
  ifp = stdin;
  strcpy(qimage,"standard in");
  ofp = stdout;
  strcpy(output_image,"standard out");

  while((c=getopt(argc,argv,"i:o:h"))!=EOF)
  {
   switch (c){
   case 'o':
         strcpy(output_image,optarg);
         ofp = fopen(optarg,"wb");
         break;
   case 'i':
         strcpy(qimage,optarg);
         ifp = fopen(optarg,"rb");
         break;
   case 'h':
         usage();
         exit(1);
              }
   }



   fread(&numlev,1,sizeof(int),ifp);
   fread(&numbits,1,sizeof(int),ifp);
   fread(&maxvalue,1,sizeof(int),ifp);
   fread(&minvalue,1,sizeof(int),ifp);

   range = maxvalue - minvalue + 1;
   stepsize = range/(numlev);



/* Allocate space for the boundary and reconstruction values
*/

   bound = (int *) calloc((numlev+1),sizeof(int));
   reco = (int *) calloc((numlev+1),sizeof(int));

/* Construct the boundary and reconstruction tables
*/

   bound[0] = minvalue;
   for(level=1; level<=numlev; level++)
     {
      bound[level] = bound[level-1] + stepsize;
      reco[level-1] = bound[level-1] + (bound[level] - bound[level-1])/2;
     }

   




/* Find out how large the image is */

   fread(&row_size,1,sizeof(int),ifp);
   fread(&col_size,1,sizeof(int),ifp);

/* Allocate space for input buffer */

    buffer = (int *) calloc(row_size*col_size+8,sizeof(int));

/* Read the coded image */

    unstuff(numbits,ifp,buffer,&count);
    if(count != row_size*col_size)
    {
     fprintf(stderr,"Mismatch in amount of data\n");
     fprintf(stderr,"Row size = %d, Column size = %d, Number of data points read = %d\n",row_size,col_size,count);
    }

/*  Generate recounstructed image */

   count = 0;
   for(row=0; row<row_size; row++)
      for(col=0; col<col_size; col++)
         {
          label = buffer[count];
          pixel = decuqi(label,reco);
          putc(pixel,ofp);
          count++;
         }


   }

void usage()
{
  fprintf(stderr,"Usage: uqimg [-i infile] [-o outfile] [-h]\n");
  fprintf(stderr,"\t infile  : File containing the compressed representation. The\n");
  fprintf(stderr,"\t default is standard in.\n");
  fprintf(stderr,"\t outfile : File containing the reconstructed image.  The default\n");
  fprintf(stderr,"\t is standard out.\n");
  fprintf(stderr,"\t -h   : This option will get you this message.\n");
}
