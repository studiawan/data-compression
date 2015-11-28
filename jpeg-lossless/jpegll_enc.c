#include <stdio.h>
#include <unistd.h>
#include "idc.h"

/**********************************************************************
*                                                                      *
*  File: jpegll_enc.c                                                  *
*  Function:  decorrelates an image using the lossless JPEG predictors *
*  Author (aka person to blame) : K. Sayood                            *
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

void usage(void);


void main(int argc, char **argv)
{
    int row, col, row_size, col_size, temp, mode,pred;
    char inimage[50], resimage[50];
    unsigned char **image_in, **res_image;
    int c;
    FILE *ifp, *ofp;
  extern int  optind;
  extern char *optarg;

  ofp = stdout;
  strcpy(resimage,"standard out");

  mode = -1;
  row_size = -1;
  col_size = -1;
  while((c=getopt(argc,argv,"i:o:m:x:y:h"))!=EOF)
  {
   switch (c){
   case 'i':
         strcpy(inimage,optarg);
         break;
   case 'o':
         strcpy(resimage,optarg);
         ofp = fopen(optarg,"wb");
         break;
   case 'm':
         sscanf(optarg,"%d", &mode);
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

    if(mode < 0)
    {
      fprintf(stderr,"Enter lossless JPEG mode (0 .. 7) :");
      scanf("%d",&mode);
    }


    fprintf(stderr,"\n\n\t\tJPEG Lossless Compression \n");
    fprintf(stderr,"This program generates the prediction error (residuals)\n");
    fprintf(stderr,"using the different JPEG lossless predictive modes.\n");
    fprintf(stderr,"The selected JPEG mode is %d and the residual image is written\n",mode);
    fprintf(stderr,"to %s\n",resimage);
    fprintf(stderr,"\n In order to obtain compression, these residuals should\n");
    fprintf(stderr,"be encoded using a variable rate coder.\n");


/* If the image dimensions have not been provided find the image dimensions   */

    if(row_size < 0 || col_size < 0)
    image_size(inimage, &row_size, &col_size);

    fprintf(stderr,"\n Image size: %d X %d\n",col_size,row_size);

/* Assign space for the input and residual images */

    image_in  = (unsigned char **) calloc(row_size,sizeof(char *));

    for(row=0; row<row_size; row++)
    {
     image_in[row]  = (unsigned char *) calloc(col_size,sizeof(char));
    }


    res_image  = (unsigned char **) calloc(row_size,sizeof(char *));

    for(row=0; row<row_size; row++)
    {
     res_image[row]  = (unsigned char *) calloc(col_size,sizeof(char));
    }

/* Read the image to be decorrelated */

    readimage(inimage, image_in, row_size, col_size);

/* Generate prediction using the prediction mode selected */

   for(row=0; row<row_size; row++)
      for(col=0; col<col_size; col++)
         {
          switch(mode) {
           case 0:
            pred = 0;
            break;
           case 1:
            if(row == 0 && col == 0)
              pred = 0;
            else if(row==0)
              pred = image_in[row][col -1];
            else
              pred = image_in[row-1][col];
            break;

           case 2:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_in[row-1][col];
            else
              pred = image_in[row][col -1];
            break;

           case 3:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_in[row-1][col];
            else if(row==0)
              pred = image_in[row][col -1];
            else
              pred = image_in[row-1][col - 1];
            break;

           case 4:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_in[row-1][col];
            else if(row==0)
              pred = image_in[row][col -1];
            else
              pred = image_in[row][col-1] + image_in[row-1][col] - image_in[row-1][col- 1];
            break;

           case 5:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_in[row-1][col];
            else if(row==0)
              pred = image_in[row][col -1];
            else
              pred = image_in[row][col-1] + (image_in[row-1][col] - image_in[row-1][col- 1])/2;
            break;

           case 6:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_in[row-1][col];
            else if(row==0)
              pred = image_in[row][col -1];
            else
              pred = image_in[row-1][col] + (image_in[row][col-1] - image_in[row-1][col- 1])/2;
            break;

           case 7:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_in[row-1][col];
            else if(row==0)
              pred = image_in[row][col -1];
            else
              pred = (image_in[row][col-1] + image_in[row-1][col])/2;
            break;
            default:
             fprintf(stderr,"No JPEG mode was specified");
             exit(1);
                    }

/* Generate the residual */

           temp = image_in[row][col] - pred;

/* Represent the residual modulo 256 */

           if(temp < 0)
             temp += 255;
           if(temp > 255)
             temp -= 255;
           res_image[row][col] = temp;
         }

/* Store JPEG mode and dimensions of the image */
     
    fwrite(&mode,1,sizeof(int),ofp);
    fwrite(&col_size,1,sizeof(int),ofp);
    fwrite(&row_size,1,sizeof(int),ofp);

/* Store residual image  */

    for(row=0; row<row_size; row++)
     for(col=0; col<col_size; col++)
       putc(res_image[row][col],ofp);


   }

void usage(void)
{
  fprintf(stderr,"Usage: jpegll_enc [-i infile] [-o outfile] [-m mode]\n");
  fprintf(stderr,"\t [-x row_size] [-y [col_size]\n");
  fprintf(stderr,"\t infile  : The file containing the image.  It is assumed\n");
  fprintf(stderr,"\t\t that the image is a grey level (8 bits/pixel) image\n");
  fprintf(stderr,"\t\t stored in raw format.  To read from standard in modify\n");
  fprintf(stderr,"\t\t readimage.c\n");
  fprintf(stderr,"\t outfile : The file containing the residual or decorrelated\n");
  fprintf(stderr,"\t\t image.  No variable lengrh coding has been performed so\n");
  fprintf(stderr,"\t\t the outfile will be bigger than the infile.  To get a\n");
  fprintf(stderr,"\t\t compressed file use a variable length coder such as huff_enc\n");
  fprintf(stderr,"\t\t on outfile.  To pipe the output of this program directly\n");
  fprintf(stderr,"\t\t to another program omit this option. This will result in\n");
  fprintf(stderr,"\t\t the output being written to standard out.\n");

  fprintf(stderr,"\t mode:  an integer between 0 and 7 corresponding\n");
  fprintf(stderr,"\t\t to the eight lossless JPEG modes\n");
  fprintf(stderr,"\t row_size: Length of a row of the image\n");
  fprintf(stderr,"\t col_size: Length of a column of the image\n");
  fprintf(stderr,"\t\t If the column or row size is not provided the program\n");
  fprintf(stderr,"\t\t will check to see if the iamge corresponds to any of the\n");
  fprintf(stderr,"\t\t standard sizes it is familiar with.  To add to the list of\n");
  fprintf(stderr,"\t\t standard sizes, edit image_size.c\n\n");
}
