#include <stdio.h>  
#include <sys/stat.h>  
#include <malloc.h>          /* for halloc */  
  
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
  
  
int image_size(char file[], int *r, int *c)  
{  
/* 
Written by James R. Nau, 6-Oct-89 
 
Attempt to find the size of an image in file pointed to by *file. 
Number of rows (down), and the number of columns (across) is returned. 
Uses a stat call to find the size of the file.  Then, attempts to find 
a pertinent number of rows and columns that we normally use.  Currently 
 
supported formats: 
 
   row x column  size        Format 
 
    128x128    16,384        Small Image 
    320x200    64,000        IBM VGA Style 
    256x256    65,536        Standard NASA Image (small) 
    256x384    98,304        IVG Low-Res Image 
    512x384   196,608        IVG High-Res Image 
    512x512   262,144        Standard NASA Image (large) 
    640x480   307,200        Super VGA (from GIF usually) 
    720x480   345,600 
    576x720   414,720        JPEG images 
 
Parameters: 
         *file: pointer to filename containing image 
          *row: pointer to the number of rows we decided on 
       *column: pointer to number of columns we decided on 
 
Return values: 
        0: ok 
       -1: Couldn't use stat function 
       -2: Unknown image size 
*/  
  
   struct stat status;  
   int res;
   float sz;  
   long img_len;  
  
   res = stat (file, &status);  
   if (res != 0)  
   {  
      return (-1);  
   }  
   img_len = status.st_size;  
  
   if (img_len == 16384L)       /* For BUD's images */  
   {  
      *r = 128;  
      *c = 128;  
      sz = 16.384;
   }  
   else if (img_len == 64000L)  /* IBM VGA Screen */  
   {  
      *r = 200;  
      *c = 320;  
      sz = 64.000;
   }  
   else if (img_len == 65536L)  /* Standard */  
   {  
      *r = 256;  
      *c = 256;  
      sz = 65.536;
   }  
   else if (img_len == 98304L)  /* Low-Res IVG images */  
   {  
      *r = 256;  
      *c = 384;  
      sz = 98.304;
   }  
   else if (img_len == 196608L) /* Hi-Res IVG images */  
   {  
      *r = 512;  
      *c = 384;  
      sz = 196.608;
   }  
   else if (img_len == 262144L) /* Standard, but large... */  
   {  
      *r = 512;  
      *c = 512;  
      sz = 262.144;
   }  
   else if (img_len == 307200L) /* SVGA Hi-Res from GIFs */  
   {  
      *r = 480;  
      *c = 640;  
      sz = 307.200;
   }  
   else if (img_len == 345600L) /* mpeg frame 720 x 480 */  
   {  
      *r = 720;  
      *c = 480;  
      sz = 345.600;
   }  
   else if (img_len == 414720L) /* jpeg test image 576 x 720 */  
   {  
      *r = 576;  
      *c = 720;  
      sz = 414.720;
   }  
   else                         /* oops */  
   {  
      fprintf(stderr, "\nError, unknown image size: %ld\n", img_len);  
      return (-2);  
   }  
   //fprintf(stderr, "\nError, unknown image size: %ld\n", img_len);  
   return sz;  
} 
