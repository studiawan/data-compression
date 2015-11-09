#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "idc.h"

/**********************************************************************
*                                                                      *
*  File: huff_enc.c                                                    *
*  Function:  Huffman encodes an input file assuming a 256 letter      *
*             alphabet                                                 *
*  Author  : S. Faltys                                                 *
*  Last mod: 7/21/95                                                   *
*  Usage:  see usage(), for details see man page or huff_enc.doc       *
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




void usage(void);

void main(int argc, char **argv)
{
	unsigned char *file; /*pointer to an array for file */
	char infile[80], outfile[80], codefile[80], scodefile[80];
			/* input and output files*/
	char temp [80],type,where,t;
	int size,num,c;
	int i,j,n; /* counters */
	FILE *ifp, *ofp, *cfp, *sfp, *tmp_fp;
	char *length,x; /*pointer to an array for code lengths*/
	int values[256], loc[256]; 
	unsigned int *code; 
		/* pointer to an array for code */
	float prob[256],p;
	extern int optint;
	extern char *optarg;

	ifp=stdin;
	t=0;  /*flag to see if an input filename was given*/
	ofp=stdout;
	x=0; /* flag if output is piped to decoder */
	cfp=NULL;
	sfp=NULL;
	num=256;

        code=(unsigned int *)malloc(num*sizeof(unsigned int));
        length=(char *)malloc(num*sizeof(char));

	while((c=getopt(argc,argv,"i:o:c:s:h"))!=EOF)
	{
	switch(c){

/* input file */

	case 'i':
		strcpy(infile,optarg);
		if((ifp=fopen(infile,"rb"))==NULL){
                fprintf(stderr,"Image file cannot be opened for input.\n");
                return;
		}
		t=1;
		break;

/* output file */

	case 'o':
		strcpy(outfile,optarg);
		if((ofp=fopen(outfile,"wb"))==NULL){
                fprintf(stderr,"Output file cannot be opened for output.\n");
                return;
		}
		x=1;
		break;
/* code file */

	case 'c':
		strcpy(codefile,optarg);
                if((cfp=fopen(codefile,"rb"))==NULL){
                fprintf(stderr,"Code file cannot be opened for input.\n");
                return;
		}
		getcode(cfp,num,code,length);
                break;

/* file to store code in */

	case 's':

		strcpy(scodefile,optarg);
                if((sfp=fopen(scodefile,"wb"))==NULL){
                fprintf(stderr,"Code file cannot be opened for output.\n");
                return;
		}
		break;

        case 'h':
              usage();
              exit(1);
              break;
		}
	}

/* get size of file */
//	fprintf(stderr,"After getopt\n");

   /* create a temporary file for input */

	if(t==0){
		strcpy(infile,"tmpf");
                tmp_fp=fopen(infile,"wb+");
                while((t=getc(ifp))!=EOF)
                        putc(t,tmp_fp);
                fclose(tmp_fp);
                ifp=fopen(infile,"rb");
		t=0;
	}

	fseek(ifp,0,2); /* set file pointer at end of file */
	size=ftell(ifp); /* gets size of file */
	++size;
	fseek(ifp,0,0); /* set file pointer to begining of file */
	
/* get memory for file */

        file=(unsigned char*)malloc(2*size*sizeof(unsigned char));
        if (file==NULL){
                printf("Unable to allocate memory for file.\n");
                exit(1);
	}

/* get file */

        fread(file,sizeof(unsigned char),size,ifp);
        fclose(ifp);

/* remove temporary file if one was used */

        if(t==0)
                remove("tmpf");

/* create code */

	if(cfp==NULL){

/* set values to zero */

        for(i=0;i<num;i++)
                values[i]=1;

//	fprintf(stderr,"Before values\n");
/* find values */

	value(values,file,size,num);
//	fprintf(stderr,"After value\n");

/* find probs */

	p=size+0.0;
	for(i=0;i<num;i++)
		prob[i]=values[i]/p;

/* set to zero */

	for(i=0;i<num;i++){
		code[i]=0;
		length[i]=0;
	}

/* sort prob array */

	sort(prob,loc,num);

//	fprintf(stderr,"After sort   \n");
/* make huff code */

	huff(prob,loc,num, code, length);
//	fprintf(stderr,"After huff   \n");

	}
//	fprintf(stderr,"After cfp==NULL\n");

/* encode file */

	size=files(size,code,length,file);
//	fprintf(stderr,"After size   \n");

/* write length of encoded file to the decoder */

/*	if(x==0)
	fwrite(&size,sizeof(int),1,ofp);
*/

	if(sfp==NULL){

/* write encoded file to file */
 
	        fwrite(code,sizeof(unsigned int),num,ofp);
		fwrite(length,sizeof(char),num,ofp);
	}

        fwrite(file,sizeof(unsigned char),size,ofp);
        fclose(ofp);

/* write code to a file */

	if(sfp!=NULL){
		fwrite(code,sizeof(unsigned int),num,sfp);
		fwrite(length,sizeof(char),num,sfp);
		fclose(sfp);
	}

}

void usage()
{
  fprintf(stderr,"Usage:\n");
  fprintf(stderr,"huff_enc [-i infile][-o outfile][-c codefile][-s storecode][-h]\n");
  fprintf(stderr,"\t imagein : file containing the input to be encoded.  If no\n");
  fprintf(stderr,"\t\t name is provided input is read from standard in.  This\n");
   fprintf(stderr,"\t\t feature can be used to directly encode the output of programs\n");
   fprintf(stderr,"\t\t such as jpegll_enc, and aqfimg_enc.\n");
  fprintf(stderr,"\t outfile : File to contain the encoded representation.  If no\n");
  fprintf(stderr,"\t\t name is provided the output is written to standard out.\n");
  fprintf(stderr,"\t codefile: If this option is used the program will read the\n");
   fprintf(stderr,"\t\t Huffman code from codefile.  If the option is not used the\n");
  fprintf(stderr,"\t\t program computes the Huffman code for the file being encoded\n");
  fprintf(stderr,"\t storecod: If this option is specified the Huffman code used to\n");
  fprintf(stderr,"\t\t encode the file is stored in codefile.  If this option is\n");
   fprintf(stderr,"\t\t not specified the code is stored in outfile\n");
}
