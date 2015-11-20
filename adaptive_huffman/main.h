/********************** Start of MAIN.H ***********************/
#ifndef _MAIN_H
#define _MAIN_H

#ifdef __STDC__
void CompressFile( FILE *input, BIT_FILE *output, int argc, char *argv[] );
void ExpandFile( BIT_FILE *input, FILE *output, int argc, char *argv[] );

#else /* __STDC__ */
void CompressFile();
void ExpandFile();

#endif /* __STDC__ */
extern char *Usage;
extern char *CompressionName;

#endif /* _MAIN_H */

/************************* End of MAIN.H ************************/
