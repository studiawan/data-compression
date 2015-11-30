#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include "idc.h"


/**********************************************************************
*                                                                      *
*  File:  uqimg_enc.c                                                  *
*  Function:  encodes am image using uniform quantization.             *
*  Author  : K. Sayood                                                 *
*  Last mod: 5/12/95                                                   *
*  Usage:  see usage()                                                 *
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


void usage();

void main(int argc, char **argv)
{
    int row, col, row_size, col_size;
    char input_image[50], qimage[50];
    unsigned char **imgin;
    int *bound, *reco, input, label, output, numlev, numbits, c;
    int minvalue, maxvalue, range, level, stepsize, temp, end_flag;
    FILE *ofp;
  extern int  optind;
  extern char *optarg;




    fprintf(stderr,"\n\n\t\tUniform Quantization of Images - Encoder\n");

  ofp = stdout;
  strcpy(qimage,"standard out");
  numlev = -1;
  numbits = -1;
  minvalue = 0;
  maxvalue = 255;

  while((c=getopt(argc,argv,"i:o:l:b:m:t:h"))!=EOF)
  {
   switch (c){
   case 'i':
         strcpy(input_image,optarg);
         break;
   case 'o':
         strcpy(qimage,optarg);
         ofp = fopen(optarg,"wb");
         break;
   case 'l':
         sscanf(optarg,"%d", &numlev);
         break;
   case 'b':
         sscanf(optarg,"%d", &numbits);
         break;
   case 'm':
         sscanf(optarg,"%d", &maxvalue);
         break;
   case 't':
         sscanf(optarg,"%d", &minvalue);
         break;
   case 'x':
         sscanf(optarg,"%d", &row_size);
         break;
   case 'y':
         sscanf(optarg,"%d", &col_size);
         break;
   case 'h':
         usage();
         exit(1);
              }
   }



   if(numlev > 0 && numbits > 0)
   {
    temp = (int) pow((double) 2,(double) numbits);  
    if(temp != numlev)
    {
     fprintf(stderr,"\n You have entered values for the number of levels and\n");
     fprintf(stderr,"number of bits that are not compatible.  The number of\n");
     fprintf(stderr,"levels should be 2^(number of bits).  If you want to use\n");
     fprintf(stderr,"a number of levels which is not a power of 2 then only\n");
     fprintf(stderr,"enter a value for the number of levels.\n");
     exit(1);
    }
   }
     

    

   if(numlev < 0 && numbits < 0)
   {
    fprintf(stderr,"\n Enter number of bits per pixel: ");
    scanf("%d",&numbits);
    numlev = (int) pow((double) 2,(double) numbits);  
   }

   if(numlev < 0 && numbits > 0)
    numlev = (int) pow((double) 2,(double) numbits);  
  
   if(numlev > 0 && numbits < 0)
     numbits = (int) (log((double)numlev)/log((double) 2.) + 0.99999);
 
/* Determine range, and stepsize for the quantizer
*/

   range = maxvalue - minvalue + 1;
   stepsize = range/(numlev);

/* Allocate space for the boundary values */

   bound = (int *) calloc((numlev+1),sizeof(int));

/* Construct the boundary tables   */

   bound[0] = minvalue;
   for(level=1; level<=numlev; level++)
     {
      bound[level] = bound[level-1] + stepsize;
     }

   




/* Find out how large the image is */

    image_size(input_image, &row_size, &col_size);   

/* Allocate space for input image  */

    imgin  = (unsigned char **) calloc(row_size,sizeof(char *));  
    for(row=0; row<row_size; row++)
     imgin[row] = (unsigned char *) calloc((col_size),sizeof(char));


/* Read the input image */

    readimage(input_image, imgin, row_size, col_size);  

/*  Store coding parameters in the output file */

   fwrite(&numlev,1,sizeof(int),ofp);
   fwrite(&numbits,1,sizeof(int),ofp);
   fwrite(&maxvalue,1,sizeof(int),ofp);
   fwrite(&minvalue,1,sizeof(int),ofp);
   fwrite(&row_size,1,sizeof(int),ofp);
   fwrite(&col_size,1,sizeof(int),ofp);
  

/* encode each pixel into an integer label and store  */
   end_flag = 0;
   for(row=0; row<row_size; row++)
      for(col=0; col<col_size; col++)
         {
          if(row == row_size-1 && col == col_size - 1)
            end_flag = 1;
          input = imgin[row][col];
          label = encuqi(input,bound,numlev);
          stuffit(label,numbits,ofp,end_flag);
         }


   }
void usage()
{
  fprintf(stderr,"Usage: uqimg_enc [-i input file][-o output file][-l numlev][-b numbits][-m maxvalue][-t minvalue][-x row_size][-y col_size][-h]\n");
  fprintf(stderr,"\t input file : File containing the image to be quantized.\n");
  fprintf(stderr,"\t output file: File which will contain the compressed representation\n");
  fprintf(stderr,"\t including any header information. The default is standard out.\n");
  fprintf(stderr,"\t numlev  : Number of reconstruction values in the quantizer. If\n");
  fprintf(stderr,"\t this option is not used, the number of levels is computed as\n");
  fprintf(stderr,"\t 2^(number of bits).  If numlev is provided, numbits need not\n");
  fprintf(stderr,"\t be provided.\n");
  fprintf(stderr,"\t numbits  : Number of bits per pixel.\n");
  fprintf(stderr,"\t maxvalue : The maximum value that the input can take.  The\n");
  fprintf(stderr,"\t default value is 255.\n");
  fprintf(stderr,"\t minvalue : The minimum value that the input can take.  The\n");
  fprintf(stderr,"\t default value is 0.  The minvalue and maxvalue are used to compute\n");
  fprintf(stderr,"\t the stepsize for the uniform quantizer,\n");
  fprintf(stderr,"\t row_size : Number of pixels in a row.\n");
  fprintf(stderr,"\t col_size : Number of rows of pixels.  If row_size and col_size\n");
  fprintf(stderr,"\t\t are not provided, the program will check to see if the image \n");
  fprintf(stderr,"\t\t corresponds to any of the standard sizes it is familiar with.  \n");
  fprintf(stderr,"\t\t To add to the list of standard sizes, edit image_size.c\n\n");

 
}
