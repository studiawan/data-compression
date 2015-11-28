#include <stdio.h>
#include <unistd.h>

/**********************************************************************
*                                                                      *
*  File: jpegll_dec.c                                                  *
*  Function:  reconstructs an image which was decorrelated using the   *
*             lossless JPEG predictors                                 *
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
    char outimage[50], resimage[50];
    unsigned char **image_out, **res_image;
    int c;
    FILE *ifp, *ofp;
  extern int  optind;
  extern char *optarg;

/* Obtain the filename for the residual image */



  ifp = stdin;
  ofp = stdout;
  while((c=getopt(argc,argv,"i:o:m:h"))!=EOF)
  {
   switch (c){
   case 'i':
         strcpy(resimage,optarg);
         ifp = fopen(optarg,"rb");
         break;
   case 'o':
         strcpy(outimage,optarg);
         ofp = fopen(optarg,"wb");
         break;
   case 'h':
         usage();
         exit(1);
              }
   }


/* Find the JPEG lossless mode and the image dimensions   */

  fread(&mode,1,sizeof(int),ifp);
  fread(&col_size,1,sizeof(int),ifp);
  fread(&row_size,1,sizeof(int),ifp);



    fprintf(stderr,"\n\n\t\tJPEG Lossless Compression \n");
    fprintf(stderr,"This program reconstructs an image from its residuals\n");
    fprintf(stderr,"stored in %s\n",resimage);
    fprintf(stderr,"\nThe residuals were obtained using JPEG lossless \n");
    fprintf(stderr,"mode %d and the reconstructed %d X %d image is stored in %s\n",mode,col_size,row_size,outimage);


/* Assign space for the output image */

    image_out  = (unsigned char **) calloc(row_size,sizeof(char *));

    for(row=0; row<row_size; row++)
    {
     image_out[row]  = (unsigned char *) calloc(col_size,sizeof(char));
    }




/* Generate prediction using the prediction mode selected */

   for(row=0; row<row_size; row++)
      for(col=0; col<col_size; col++)
         {
          c = getc(ifp);
          switch(mode) {
           case 0:
            pred = 0;
            break;
           case 1:
            if(row == 0 && col == 0)
              pred = 0;
            else if(row==0)
              pred = image_out[row][col -1];
            else
              pred = image_out[row-1][col];
            break;

           case 2:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_out[row-1][col];
            else
              pred = image_out[row][col -1];
            break;

           case 3:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_out[row-1][col];
            else if(row==0)
              pred = image_out[row][col -1];
            else
              pred = image_out[row-1][col - 1];
            break;

           case 4:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_out[row-1][col];
            else if(row==0)
              pred = image_out[row][col -1];
            else
              pred = image_out[row][col-1] + image_out[row-1][col] - image_out[row-1][col- 1];
            break;

           case 5:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_out[row-1][col];
            else if(row==0)
              pred = image_out[row][col -1];
            else
              pred = image_out[row][col-1] + (image_out[row-1][col] - image_out[row-1][col- 1])/2;
            break;

           case 6:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_out[row-1][col];
            else if(row==0)
              pred = image_out[row][col -1];
            else
              pred = image_out[row-1][col] + (image_out[row][col-1] - image_out[row-1][col- 1])/2;
            break;

           case 7:
            if(row == 0 && col == 0)
              pred = 0;
            else if(col==0)
              pred = image_out[row-1][col];
            else if(row==0)
              pred = image_out[row][col -1];
            else
              pred = (image_out[row][col-1] + image_out[row-1][col])/2;
            break;
            default:
             fprintf(stderr,"No JPEG mode was specified");
             exit(1);
                    }

/* Generate the reconstruction */

           temp = c + pred;

/* Represent the residual modulo 256 */

           while(temp>255)
              temp = temp - 255;

           image_out[row][col] = temp;
         }


    for(row=0; row<row_size; row++)
     for(col=0; col<col_size; col++)
       putc(image_out[row][col],ofp);


   }

void usage(void)
{
  fprintf(stderr,"Usage: jpegll_dec [-i infile] [-o outfile] \n");
  fprintf(stderr,"\t infile is the file containing the residuals, and \n");
  fprintf(stderr,"\t outfile is the file which will contain the reconstructed\n");
  fprintf(stderr,"\t image.  If infile and/or outfile are not specified\n");
  fprintf(stderr,"\t the defaults are standard input and output\n");

}
