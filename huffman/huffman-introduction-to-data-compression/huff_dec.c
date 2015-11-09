#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "idc.h"

/**********************************************************************
*                                                                      *
*  File: huff_dec.c                                                    *
*  Function:  this program decodes files encoded using huff_enc        *
*  Author  : S. Faltys                                                 *
*  Last mod: 7/21/95                                                   *
*  Usage:  see usage(), for details see man page or huff_dec.doc       *
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
	unsigned char *file,*efile; 
		/*pointer to an array for file
		  pointer to an array for encoded image*/
	unsigned char w,p,n; /* place holders */
	char infile[80], outfile[80], codefile[80]; /* input and output files*/
	char temp [80],where,size;
	int num,s,x,c;
	int i,k,l,count,f; /* counters */
	FILE *ifp,*ofp,*cfp,*tmp_fp; 
	unsigned int *code,word,d;
	char *length,t;  /* pointer to an array for code lengths */
	extern int optint;
	extern char *optarg;

	ifp=stdin;
	t=0; /* flag to see if an input filename was given */
	ofp=stdout;
	cfp=NULL;
	num=256;
	l=0;
	word=0;
        n=1<<7; /* set highest bit equal to one */
	size=0;
	f=10000;
	x=0;

/* get memory */

        code=(unsigned int *)malloc(num*sizeof(unsigned int));
        length=(char *)malloc(num*sizeof(char));
        file=(unsigned char *)malloc(f*sizeof(unsigned char));
        if (file==NULL){
                printf("Unable to allocate memory for image.\n");
                exit(1);
	}

        while((c=getopt(argc,argv,"i:o:c:h"))!=EOF)
        {
        switch(c){

/* input file */

        case 'i':
                strcpy(infile,optarg);
                if((ifp=fopen(infile,"rb"))==NULL){
                printf("Cannot open file for input!\n");
                return;
		}
		t=1;
		break;

/* output file */

        case 'o':
                strcpy(outfile,optarg);
                ofp=fopen(outfile,"wb");
                break;

/* code file */

        case 'c':
                strcpy(codefile,optarg);
                cfp=fopen(optarg,"rb");
                getcode(cfp,num,code,length);
                break;
        case 'h':
                usage();
                exit(1);
                break;
	}
	}

   fprintf(stderr,"\t Patience -- This may take a while\n");

/* get file size */

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
	s=ftell(ifp); /* gets size of file */
	fseek(ifp,0,0); /* set file pointer to begining of file */

	if(cfp==NULL){
		fread(code,sizeof(unsigned int),num,ifp);
		fread(length,sizeof(char),num,ifp);
		s=s-ftell(ifp); /* s = the size of the encoded file */
		}

/* get memory for encoded file */

	efile=(unsigned char *)malloc(s*sizeof(unsigned char));

/* get encoded file */

	fread(efile,sizeof(unsigned char),s,ifp);
	fclose(ifp);

/* remove temporary file if one was used */

	if(t==0)
		remove("tmpf");

/* decode file */

	count=0;
	w=*(efile+count); /* w equals encoded word */
	++count; /* counter to keep track of which word is being decoded */
    for( ;count<s; ){ 
        for(k=0;k<8;k++){
		if((w & n) == n){ /* checks bit value of encoded word */
			++word;
			}
		++size; /* counter to keep track of length of word */

	/* check to see if word equals a code */

		for(i=0;i<num;i++){
			if(word==code[i] && size==length[i]){
				if(l<f){
					p=i; /* pixal value is a char */
					*(file+l)=p; /* decoded image */
					++l; /* counter of length of image */
					i=num; /* ends loop */
					}
				else{
					fwrite(file,sizeof(unsigned char),f,ofp);
					l=0;
                                        p=i; /* pixal value is a char */
                                        *(file+l)=p; /* decoded image */
                                        ++l; /* counter of length of image */
                                        i=num; /* ends loop */
					x=1;
				}
				word=0; /* reset word */
				size=0; /* reset size */
					    }
			}
	word<<=1;
	w<<=1;
		}
		w=*(efile+count); /* get next encoded value */
		++count;
	}
	*(file+l)=EOF;
	fwrite(file,sizeof(unsigned char),l,ofp);
	fclose(ofp);

}		

void usage()
{
  fprintf(stderr,"Usage:\n");
  fprintf(stderr,"huff_enc [-i infile][-o outfile][-c codefile][-h]\n");
  fprintf(stderr,"\t imagein : file containing the Huffman-encoded data.  If no\n");
  fprintf(stderr,"\t\t name is provided input is read from standard in.\n"
);
  fprintf(stderr,"\t outfile : File to contain the reconstructed representation.  If no\n");
  fprintf(stderr,"\t\t name is provided the output is written to standard out.\n
");
  fprintf(stderr,"\t codefile: This option is required if the Huffman encoded file\n");
  fprintf(stderr,"\t\t does not contain the Huffman code as part of the header\n");
  fprintf(stderr,"\t\t If this option is specified the program will read the\
n");
   fprintf(stderr,"\t\t Huffman code from codefile.\n");
}

