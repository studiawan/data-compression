#include <stdio.h>  
#include <stdlib.h>  
  
/*********************************************************************** 
*                                                                      * 
*  File: readimage.c                                                   * 
*  Function:  reads an image (what else :?)                            * 
*  Author  : K. Sayood                                                 * 
*  Last mod: 5/12/95                                                   * 
*  Usage:  see usage()                                                 * 
*                                                                      * 
*                                                                      * 
*  filename[] : character string containing filename                   * 
*  **image    : array of pointers pointing to the rows in an image     * 
*  row_size   : number of rows in the image                            * 
*  col_size   : number of columns in the image.                        * 
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
void readimage(char filename[], unsigned char **image, int row_size, int col_size)  
{
	int r, c;  
     FILE *infile;  
  
     infile = fopen(filename,"rb");  
     if (infile == NULL)  
	    {
			fprintf(stderr, "Unable to open file %s", filename);  
	     	exit(1);  
	    }  
  
     for(r=0; r<row_size; r++)  
     	fread(image[r],col_size,sizeof(char),infile);  
           
     fclose(infile);  
}  
