/**************************** Start of CARMAN.C *************************
*
* This is the main program for the simple Compressed Archive Manager.
* This program can be used to add, delete, extract, or list the files
* in a CAR archive. The code here should run under standard ANSI
* compilers under MS-DOS (with ANSI mode selected) or K&R compilers
* under UNIX. The code uses an LZSS compression algorithm identical to
* that used earlier in the book.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#ifdef __STDC__
 /* All Borland C/C++ versions */
 #ifdef __TURBOC__
 #define MSDOS 1
 #include <io.h>
 #include <dir.h>
 #define DIR_STRUCT struct ffblk
 #define FIND_FIRST(n, d, a ) findfirst( n, d, a )
 #define FIND_NEXT findnext
 #define DIR_FILE_NAME ff_name
#endif
 /*Microsoft, Watcom, Zortech */
#if defined( M__186 ) || defined ( __ZTC__ ) || defined ( __TSC__ )
 #define MSDOS 1
 #include <dos.h>
 #define DIR_STRUCT struct find_t
 #define FIND_FIRST( n, d, a) _dos_findfirst( n, a, d )
 #define FIND_NEXT _dos_findnext
 #define DIR_FILE_NAME name
 #endif
#endif
/*
* A few constants used throughout the program.
*/
#define BASE_HEADER_SIZE 19
#define CRC_MASK 0xFFFFFFFFL
#define CRC32_POLYNOMIAL 0xEDB88320L
/*
* The only data structure used inside the CAR file is the header block.
* Each file is preceded by a header, stored in a portable format.
* The header is read into and out of the structure defined below.
* The CAR file is structured as a series of header/data sequences, with
* the EOF being denoted as a header with a file name length of 0. Note
* that the length of each header will vary depending on the length of
* the file name.
*/
#ifndef FILENAME_MAX
#define FILENAME_MAX 128
#endif
typedef struct header {
 char file_name[ FILENAME_MAX ];
 char compression_method;
 unsigned long original_size;
 unsigned long compressed_size;
 unsigned long original_crc;
 unsigned long header_crc;
} HEADER;
/*
* Local function prototypes
*/
#ifdef __STDC__
void FatalError( char *message, ... );
void BuildCRCTable( void );
unsigned long CalculateBlockCRC32( unsigned int count, unsigned long crc,
 void *buffer );
unsigned long UpdateCharacterCRC32( unsigned long crc, int c );
int ParseArguments( int argc, char *argv[] );
void UsageExit( void );
void OpenArchiveFiles( char *name, int command );
void BuildFileList( int argc, char *argv[], int command );
int ExpandAndMassageMSDOSFileNames( int count, char *wild_name );
void MassageMSDOSFileName( int count, char *file );
int AddFileListToArchive( void );
int ProcessAllFilesInInputCar( int command, int count );
int SearchFileList( char *file_name );
int WildCardMatch( char *s1, char *s2 );
void SkipOverFileFromInputCar( void );
void CopyFileFromInputCar( void );
void PrintListTitles( void );
void ListCarFileEntry( void );
int RatioInPercent( unsigned long compressed, unsigned long original );
int ReadFileHeader( void );
unsigned long UnpackUnsignedData( int number_of_bytes,
 unsigned char *buffer );
void WriteFileHeader( void );
void PackUnsignedData( int number_of_bytes, unsigned long number,
 unsigned char *buffer );
void WriteEndOfCarHeader( void );
void Insert( FILE *input_text_file, char *operation );
void Extract( FILE *destination );
int Store( FILE *input_text_file );
unsigned long Unstore( FILE *destination );
int LZSSCompress( FILE *input_text_file );
unsigned long LZSSExpand( FILE *destination );
#else
void FatalError();
void BuildCRCTable();
unsigned long CalculateBlockCRC32();
unsigned long UpdateCharacterCRC32();
int ParseArguments();
void UsageExit();
void OpenArchiveFiles();
void BuildFileList();
int ExpandAndMassageMSDOSFileNames();
void MassageMSDOSFileName();
int AddFileListToArchive();
int ProcessAllFilesInInputCar();
int SearchFileList();
int WildCardMatch();
void SkipOverFileFromInputCar();
void CopyFileFromInputCar();
void PrintListTitles();
void ListCarFileEntry();
int RatioInPercent();
int ReadFileHeader();
unsigned long UnpackUnsignedData();
void WriteFileHeader();
void PackUnsignedData();
void WriteEndOfCarHeader();
void Insert();
void Extract();
int Store();
unsigned long Unstore();
int LZSSCompress();
unsigned long LZSSExpand();
#endif
/*
* All global variables are defined here.
*/
char *TempFileName[ FILENAME_MAX ]; /* The output archive is first */
 /* opened with a temporary name */
FILE * InputCarFile;  /* The input CAR file. This file */
  /* may not exist for 'A' commands */
char CarFileName[ FILENAME_MAX ]; /* Name of the CAR file, defined */
  /* on the command line */
FILE *OutputCarFile;  /* The output CAR, only exists for*/
  /* the 'A' and 'R' operations */
HEADER Header;  /* The Header block for the file */
  /* presently being operated on */
char *FileList[ 100 ]; /* The list of file names passed */
  /* on the command line */
unsigned long Ccitt32Table[ 256 ]; /* This array holds the CRC */
  /* table used to calculate the 32 */
  /* bit CRC values */
/*
* This is the main routine for processing CAR commands. Most of the
* major work involved here has been delegated to other functions.
* This routine first parses the command line, then opens up the input
* and possibly the output archive. It then builds a list of files
* to be processed by the current command. If the command was 'A', all
* of the files are immediately added to the output archives. Finally,
* the main processing loop is called. It scans through the entire
* archive, taking action on each file as necessary. Once that is
* complete, all that is left to do is optionally delete the input file,
* then rename the output file to have the correct CAR file name.
*/
int main( int argc, char *argv[] )
//int argc;
//char *argv[];
{
 int command;
 int count;
 setbuf( stdout, NULL );
 setbuf( stderr, NULL );
 fprintf( stderr, "CARMAN 1.0 : " );
 BuildCRCTable();
 command = ParseArguments( argc, argv );
 fprintf( stderr, "\n" );
 OpenArchiveFiles( argv[ 2 ], command );
 BuildFileList( argc - 3, argv + 3, command );
 if ( command == 'A' )
 count = AddFileListToArchive();
 else
 count = 0;
 if ( command == 'L' )
 PrintListTitles();
 count = ProcessAllFilesInInputCar( command, count );
 if ( OutputCarFile != NULL && count != 0 ) {
 WriteEndOfCarHeader();
 if ( ferror( OutputCarFile ) || fclose( OutputCarFile ) == EOF )
 FatalError( "Can't write" );
 #ifdef __STDC__
 remove( CarFileName );
 rename(TempFileName, CarFileName );
 #else
 unlink( CarFileName );
 link( TempFileName, CarFileName );
 unlink( TempFileName );
 #endif
 }
 if ( command != 'P' )
 printf( "\n%d file%s\n", count, ( count == 1 ) ? "" : "s" );
 else
 fprintf( stderr, "\n%d file%s\n", count, ( count == 1 ) ? "" : "s" );
 return( 0 );
}
/*
* FatalError provides a short way for us to exit the program when
* something bad happens, as well as printing a diagnostic message.
* If an output CAR file has been opened, it is deleted as well,
* which cleans up most of the traces of our work here. Note that
* K&R compilers handle variable length argument lists differently
* than ANSI compilers, so we have two different entries for the
* routines.
*/
#ifdef __STDC__
void FatalError( char *fmt, ... )
{
 va_list args;
 va_start( args, fmt );
#else
void FatalError( va_alist )
va_dcl
{
 va_list args;
 char *fmt;
 va_start( args );
 fmt = va_arg( args, char * );
#endif
 putc( '\n', stderr );
 vfprintf( stderr, fmt, args );
 putc( '\n', stderr );
 va_end( args );
 if ( OutputCarFile != NULL )
 fclose( OutputCarFile );
#ifdef __STDC__
 remove( TempFileName );
#else
 unlink( TempFileName );
#endif
 exit( 1 );
}
/*
* This routine simply builds the coefficient table used to calculate
* 32-bit CRC values throughout this program. The 256-long word table
* has to be set up once when the program starts. Alternatively, the
* values could be hard-coded in, which would offer a miniscule
* improvement in overall performance of the program.
*/
void BuildCRCTable()
{
 int i;
 int j;
 unsigned long value;
 for ( i = 0; i <= 255 ; i++ ) {
 value = i;
 for ( j = 8 ; j > 0; j-- ) {
 if ( value & 1 )
 value = ( value >> 1 ) ^ CRC32_POLYNOMIAL;
 else
 value >>= 1;
 }
 Ccitt32Table[ i ] = value;
 }
}
/*
* This is the routine used to calculate the 32-bit CRC of a block of
* data. This is done by processing the input buffer using the
* coefficient table that was created when the program was initialized.
* This routine takes an input value as a seed, so that a running
* calculation of the CRC can be used as blocks are read and written.
*/
unsigned long CalculateBlockCRC32( unsigned int count, unsigned long crc, void *buffer )
//unsigned int count;
//unsigned long crc;
//void *buffer;
{
 unsigned char *p = (unsigned char *) buffer;
 unsigned long temp1;
 unsigned long temp2;
 while ( count-- != 0 ) {
 temp1 = ( crc >> 8 ) & 0x00FFFFFFL;
 temp2 = Ccitt32Table[ ( (int) crc ^ *p++ ) & 0xff ];
 crc = temp1 ^ temp2;
 }
 return( crc );
}
/*
* If I/0 is being done on a byte-by-byte basis, as is the case with the
* LZSS code, it is easier to calculate the CRC of a byte at a time
* instead of a block at a time. This routine performs that function,
* once again taking a CRC value as input, so that this can be used to
* perform on the fly calculations. In situations where performance is
* critical, this routine could easily be recorded as a macro.
*/
unsigned long UpdateCharacterCRC32( unsigned long crc, int c )
//unsigned long crc;
//int c;
{
 unsigned long temp1;
 unsigned long temp2;
 temp1 = ( crc >> 8 ) & 0x00FFFFFFL;
 temp2 = Ccitt32Table[ ( (int) crc ^ c ) & 0xff ];
 crc = temp1 ^ temp2;
 return( crc );
}
/*
* When CARMAN first starts up, it calls this routine to parse the
* command line. We look for several things here. If any of the
* conditions needed to run CARMAN is not met, the routine opts for
* the usage printout exit. The first thing to be sure of is that
* the command line has at least three arguments, which should be
* the "CARMAN", a single character command, and an CAR archive name.
* After that, we check to be sure that the command name is a valid
* letter, and incidentally print out a short message based on it.
* Both the Addfiles and Delete commands require that some file names
* be listed as well, so a check is made for additional arguments when
* each of those arguments is encountered. Finally, the command itself
* is returned to main(), for use later in processing the command.
*/
int ParseArguments( int argc, char *argv[] )
//int argc;
//char *argv[];
{
 int command;
 if ( argc < 3 || strlen( argv[ 1 ] ) > 1 )
 UsageExit();
 switch( command = toupper( argv[ 1 ][ 0 ] ) ) {
 case 'X' :
 fprintf( stderr, "Extracting files\n";
 break;
 case 'R' :
 fprintf( stderr, "Replacing files\n" );
 break;
 case 'P' :
 fprintf( stderr, "Print files to stdout\n" );
 break;
 case 'T' :
 fprintf( stderr, "Testing integrity of files\n" );
 break;
 case 'L' :
 fprintf( stderr, "Listing archive contents\n" );
 break;
 case 'A' :
 if ( argc <= 3 )
 UsageExit();
 fprintf( stderr, "Adding/replacing files to archive\n" );
 break;
 case 'D' :
 if ( argc <= 3 )
 UsageExit();
 fprintf( stderr, "Deleting files from archive\n" );
 break;
 default :
 UsageExit();
 };
 return( command );
}
/*
* UsageExit just provides a universal point of egress for those
* times when there appears to be a problem on the command line.
* This routine prints a short help message then exits back to the OS.
*/
void UsageExit()
{
 fputs( "CARMAN - Compressed ARchive MANager\n", stderr );
 fputs( "Usage: carman command car-file [file ...]\n", stderr );
 fputs( "Commands:\n", stderr );
 fputs( " a: Add files to a CAR archive (replace if present)\n", stderr );
 fputs( " x: Extract files from a CAR archive\n", stderr );
 fputs( " r: Replace files in a CAR archive\n", stderr );
 fputs( " d: Delete files from a CAR archive\n", stderr );
 fputs( " p: Print files on standard output\n", stderr );
 fputs( " l: List contents of a CAR archive\n", stderr );
 fputs( " t: Test files in a CAR archive\n", stderr );
 fputs( "\n", stderr );
 exit( 1 );
}
/*
* After the command line has been parsed, main() has enough information
* to intelligently open the input and output CAR archive files. The
* name should have been specified on the command line, and passed to
* this routine by main(). As a convenience to the user, if the CAR
* suffix is left off the archive, this routine will add it on.
* There is one legitimate excuse for not being able to open the input
* file, which is if this is the 'Addfiles' command. There may not be
* an input archive when that command is called, in which case a failure
* is tolerated. Once the input file has been opened, an output file
* may have to be opened as well. The 'Addfiles', 'Delete', and
* 'Replace' commands all modify the CAR archive, which means the input
* CAR file is going to be processed and copied to the output. Initially,
* the output CAR file gets a temporary name. It will be renamed later
* after the input has been processed.
*
* Since we will probably be doing lots of bulk copies from the input
* CAR file to the output CAR file, it makes sense to allocate big
* buffers for the files. This is done with the two calls to setvbuf()
* right before the routine exits.
*
*/
void OpenArchiveFiles( char *name, int command )
//char *name;
//int command;
{
 char *s;
 int i;
 strncpy( CarFileName, name, FILENAME_MAX - 1 );
 CarFileName[ FILENAME_MAX - 1 ] = '\0';
 InputCarFile = fopen( CarFileName, "rb" );
 if ( InputCarFile == NULL ) {
#ifdef MSDOS
 s = strrchr( CarFileName, '\\' );
#else /* UNIX */
 s = strrchr( CarFileName, '/' );
#endif
 if ( s == NULL )
 s = CarFileName;
 if ( strrchr( s, '.' ) == NULL )
 if ( strlen( CarFileName ) < ( FILENAME_MAX - 4 ) ) {
 strcat( CarFileName, ".car" );
 InputCarFile = fopen( CarFileName, "rb" );
 }
 }
 if ( InputCarFile == NULL && command != 'A' )
 FatalError( "Can't open archive '%s'", CarFileName );
 if ( command == 'A' || command == 'R' || command == 'D' ) {
 strcpy( TempFileName, CarFileName );
 s = strrchr( TempFileName, '.');
 if ( s == NULL )
 s = TempFileName + strlen( TempFileName );
 for ( i = 0 ; i < 10 ; i++ ) {
 sprintf( s, ".$$%d", i );
 if ( ( OutputCarFile = fopen( TempFileName, "r" ) ) == NULL )
 break;
 fclose( OutputCarFile );
 OutputCarFile = NULL;
 }
 if ( i == 10 )
 FatalError( "Can't open temporary file %s", TempFileName );
 OutputCarFile = fopen( TempFileName, "wb" );
 if ( OutputCarFile == NULL )
 FatalError( "Can't open temporary file %s", TempFileName );
 }
 if ( InputCarFile != NULL )
 setvbuf( InputCarFile, NULL, _IOFBF, 8192 );
 if ( OutputCarFile != NULL )
 setvbuf( OutputCarFile, NULL, _IOFBF, 8192 );
}
/*
* Most of the commands given here take one or more file names as
* arguments. The list of files given on the command line needs to be
* processed here and put into a list that can easily be manipulated by
* other parts of the program. That processing is done here. An array
* called FileList is created, which will have a series of pointers to
* file names. If no file names were listed on the command line, which
* could be the case for commands like 'List' or 'Extract', a single
* file name of '*' is put on the start of the list. Since '*' is the
* ultimate wild card, matching everything, we don't have to have special
* processing anywhere else for an empty file list. The file names here
* are also massaged a bit further for MS-DOS file names. Under MS-DOS,
* case is not significant in file names. This means that CARMAN
* shouldn't get confused by thinking 'foo.c' and 'FOO.C' are two
* different files. To avoid this, all MS-DOS file names are converted
* here to lower case. Additionally, any file name without an extension
* is forced to end with a period, for similar reasons. This ensures that
* CARMAN knows 'FOO' and 'FOO.' are the same file. Note that I don't
* want to do this for wild card specifications. Finally, there is the
* problem of MS-DOS wild card file names. When using the 'Add' command,
* wild cards on the command line need to be expanded into real file
* names, then undergo the additional processing mentioned earlier. This
* is done with a call to a function that is MS-DOS specific. None of
* this special processing is done under UNIX, where case is significant,
* and wild cards are expanded by the shell.
*/
void BuildFileList( int argc, char *argv[], int command )
//int argc;
//char *argv[];
//int command;
{
 int i;
 int count;
 count = 0;
 if ( argc == 0 )
 FileList[ count++ ] = "*";
 else {
 for ( i = 0 ; i < argc ; i++ ) {
#ifdef MSDOS
 if ( command == 'A' )
 count = ExpandAndMassageMSDOSFileNames( count, argv[ i ] );
 else
 MassageMSDOSFileName( count++, argv[ i ] );
#endif
#ifdef MSDOS
 FileList [ count ] = malloc( strlen( argv[ i ] ) + 2 );
 if ( FileList[ count ] == NULL )
 FatalError( "Ran out of memory storing file names" );
 strcpy( FileList[ count++ ], argv[ i ] );
#endif
 if ( count > 99 )
 FatalError( "Too many file names" );
 }
 }
 FileList[ count ] = NULL;
}
/*
* Under MS-DOS, wildcards on the command line are not expanded to
* a list of file names, so it is up to application programs to do the
* expansion themselves. This routine takes care of that, by using
* the findfirst and findnext routines. Unfortunately, each MS-DOS
* compiler maker has implemented this function slightly differently, so
* this may need to be modified for your particular compiler. However,
* this routine can be replaced with a call to MassageMSDOSFileName(),
* and the program will work just fine, without the ability to handle
* wild card file names.
*/
#ifdef MSDOS
#include <dos.h>
#include <dir.h>
int ExpandAndMassageMSDOSFileNames( int count, char *wild_name )
//int count;
//char *wild_name;
{
 int done;
 DIR_STRUCT file_info_block;
 char *leading_path;
 char *file_name;
 char *p;
 leading_path = malloc( strlen( wild_name ) + 1 );
 file_name = malloc( strlen( wild_name ) + 13 );
 if ( leading_path == NULL || file_name == NULL )
 FatalError( "Ran out of memory storing file names" );
 strcpy( leading_path, wild_name );
 p = strrchr( leading_path, '\\' );
 if ( p != NULL )
 p[ 1 ] = '\0';
 else {
 p = strrchr( leading_path, ';' );
 if ( p != NULL )
 p[ 1 ] = '\0';
 else
 leading_path[ 0 ] = '\0';
 }
 done = FIND_FIRST( wild_name, & file_info_block, 0 );
 while ( !done ) {
 strcpy( file_name, leading_path );
 strcat( file_name, file_info_block.DIR_FILE_NAME );
 MassageMSDOSFileName( count++, file_name );
 done = FIND_NEXT( & file_info_block );
 if ( count > 99 )
 FatalError( "Too many file names" );
 }
 free( leading_path );
 free( file_name );
 return( count );
}
/*
* As was discussed earlier, this routine is called to perform a small
* amount of normalization on file names. Under MS_DOS, case is not
* significant in file names. In order to avoid confusion later, we force
* all file names to be all lower case, so we can't accidentally add two
* files with the same name to a CAR archive. Likewise, we need to
* prevent confusion between files that end in a period, and the same
* file without the terminal period. We fix this by always forcing the
* file name to end in a period.
*/
void MassageMSDOSFileName( int count, char *file )
//int count;
//char *file;
{
 int i;
 char *p;
 FileList[ count ] = malloc( strlen( file ) + 2 );
 if ( FileList[ count ] == NULL )
 FatalError( "Ran out of memory storing file names" );
 strcpy( FileList[ count ], file );
 for ( i = 0 ; FileList[ count ] [ i ] != '\0' ; i++ )
 FileList[ count ][ i ] = (char)
 tolower(FileList[ count ][ i ]);
 if ( strpbrk( FileList [ count ], "*?" ) == NULL ) {
 p = strrchr( FileList[ count ], '\\' );
 if ( p == NULL )
 p = FileList[ count ];
 if ( strrchr( p, '.' ) == NULL )
 strcat( FileList[ count ], "." );
 }
}
#endif
/*
* Once all of the argument processing is done, the main() procedure
* checks to see if the command is 'Addfiles'. If it is, it calls
* this procedure to add all of the listed files to the output buffer
* before any other processing is done. That is taken care of right
* here. This routine basically does three jobs before calling the
* Insert() routine, where the compression actually takes place. First,
* it tries to open the file, which ought to work. Second, it strips the
* leading drive and path information from the file, since we don't keep
* that information in the archive. Finally, it checks to see if the
* resulting name is one that has already been added to the archive.
* If it has, the file is skipped so that we don't end up with an
* invalid archive.
*/
int AddFileListToArchive()
{
 int i;
 int j;
 int skip;
 char *s;
 FILE *input_text_file;
 for ( i = 0 ; FileList[ i ] != NULL ; i++ ) {
 input_text_file = fopen( FileList[ i ], "rb" );
 if ( input_text_file == NULL )
 FatalError( "Could not open %s to add to CAR file",
 FileList[ i ] );
#ifdef MSDOS
 s = strrchr( FileList[ i ], '\\' );
 if ( s == NULL )
 s = strrchr( FileList[ i ], ':' );
#endif
#ifndef MSDOS /* Must be UNIX */
 s = strrchr( FileList[ i ], '/' );
#endif
 if ( s != NULL )
 s++;
 else
 s = FileList[ i ];
 skip = 0;
 for ( j = 0 ; j < i ; j++ )
 if ( strcmp( s, FileList[ j ] ) == 0 ) {
 fprintf( stderr, "Duplicate file name: %s", FileList[ i ] );
 fprintf( stderr, " Skipping this file...\n" );
 skip = 1;
 break;
 }
 if (s != FileList[ i ] ) {
 for ( j = 0 ; s[ j ] != '\0' ; j++ )
 FileList[ i ][ j ] = s[ j ];
 FileList[ i ][ j ] = '\0';
 }
 if ( !skip ) {
 strcpy( Header.file_name, FileList[ i ] );
 Insert( input_text_file, "Adding" );
 } else
 fclose( input_text_file );
 }
 return( i );
}
/*
* This is the main loop where all the serious work done by this
* program takes place. Essentially, this routine starts at the
* beginning of the input CAR file, and processes every file in
* the CAR. Depending on what command is being executed, that might
* mean expanding the file, copying it to standard output,
* adding it to the output CAR, or skipping over it completely.
*/
int ProcessAllFilesInInputCar( int command, int count )
//int command;
//int count;
{
 int matched;
 FILE *input_text_file;
 FILE *output_destination;
 if ( command == 'P' )
 output_destination = stdout;
 else if ( command == 'T' )
#ifdef MSDOS
 output_destination = fopen( "NUL", "wb" );
#else
 output_destination = fopen( "/dev/null", "wb" );
#endif
 else
 output_destination = NULL;
/*
* This is the loop where it all happens. I read in the header for
* each file in the input CAR, then see if it matches any of the file
* and wildcard specifications in the FileList created earlier. That
* information, combined with the command, tells me what I need to
* know in order to process the file. Note that if the 'Addfiles' command
* is being executed, the InputCarFile will be NULL, so this loop
* can be safely skipped.
*/
 while ( InputCarFile != NULL && ReadFileHeader() != 0 ) {
 matched = SearchFileList( Header.file_name );
 switch ( command ) {
 case 'D' :
 if ( matched ) {
 SkipOverFileFromInputCar();
 count++;
 } else
 CopyFileFromInputCar();
 break;
 case 'A' :
 if ( matched )
 SkipOverFileFromInputCar();
 else
 CopyFileFromInputCar();
 break;
 case 'L' :
 if ( matched ) {
 ListCarFileEntry();
 count++;
 } else
 SkipOverFileFromInputCar();
 break;
 case 'P' :
 case 'X' :
 case 'T' :
 if ( matched ) {
 Extract( output_destination );
 count++;
 } else
 SkipOverFileFromInputCar();
 break;
 case 'R' :
 if ( matched ) {
 input_text_file = fopen( Header.file_name, "rb" );
 if ( input_text_file == NULL ) {
 fprintf( stderr, "Could not find %s", Header.file_name );
 fprintf( stderr, " for replacement, skipping\n" );
 CopyFileFromInputCar();
 } else {
 SkipOverFileFromInputCar();
 Insert( input_text_file, "Replacing" );
 count++;
 fclose(input_text_file );
 }
 } else
 CopyFileFromInputCar();
 break;
 }
 }
 return( count );
}
/*
* This routine looks through the entire list of arguments to see if
* there is a match with the file name currently in the header. As each
* new file in InputCarFile is encountered in the main processing loop,
* this routine is called to determine if it has an appearance anywhere
* in the FileList[] array. The results is used to in the main loop
* to determine what action to take. For example, if the command were
* the 'Delete' command, the match result would determine whether to
* copy the file form the InputCarFile to the OutputCarFile, or skip
* over it.
*
* The actual work in this routine is really performed by the
* WildCardMatch() routine which checks the file name against one of the
* names in the FileList[] array. Since most of the commands can use
* wild cards to specify file names inside the CAR file, we need a
* special comparison routine.
*/
int SearchFileList( char *file_name )
//char *file_name;
{
 int i;
 for ( i = 0 ; FileList[ i ] != NULL ; i++ ) {
 if ( WildCardMatch( file_name, FileList[ i ] ) )
 return( 1 );
 }
 return( 0 );
}
/*
* WildCardMatch() compares string to wild_string, looking for a match.
* Wild card characters supported are only '*' and '?', where '*'
* represents a string of any length, including 0, and '?' represents any
* single character.
*/
int WildCardMatch( char *string, char *wild_string )
//char *string;
//char *wild_string;
{
 for ( ; ; ) {
 if ( *wild_string == '*' ) {
 wild_string++;
 for ( ; ; ) {
 while ( *string != '\0' && *string != *wild_string )
 string++;
 if ( WildCardMatch( string, wild_string ) )
 return( 1 );
 else if ( *string == '\0' )
 return( 0 );
 else
 string++;
 }
 } else if ( *wild_string == '?' ) {
 wild_string++;
 if ( * string++ == '\0' )
 return( 0 );
 } else {
 if ( * string != *wild_string )
 return( 0 );
 if ( *string == '\0' )
 return( 1 );
 string++;
 wild_string++;
 }
 }
}
/*
* When the main processing loop reads in a header, it checks to see
* if it is going to copy that file either to the OutputCarFile or expand
* it. If neither is going to happen, we need to skip past this file and
* go on to the next header. This can be done by seeking past the
* compressed file. Since the compressed size is stored in the header
* information, it is easy to do. Note that this routine assumes that the
* file pointer has not been modified since the header was read in. This
* means it should be located at the first byte of the compressed data.
*/
void SkipOverFileFromInputCar()
{
 fseek( InputCarFile, Header.compressed_size, SEEK_CUR );
}
/*
* When performing an operation that modifies the input CAR file,
* compressed files will frequently need to be copied from the input CAR
* file to the output CAR file. This routine does that using simple
* repeated block copy operations. Since it is writing directly to the
* output CAR file, the first thing it needs to do is write out the
* current Header so that the CAR file will be properly structured.
* Following that, the compressed file is copied one block at a time to
* the output. When this routine completes, the input file pointer is
* positioned at the next header in the input CAR file, and the output
* file pointer is positioned at the EOF position in the output file.
* This is the proper place for the next record to begin.
*/
void CopyFileFromInputCar()
{
 char buffer[ 256 ];
 int count;
 WriteFileHeader();
 while ( Header.compressed_size != 0 ) {
 if ( Header.compressed_size < 256 )
 count = (int) Header.compressed_size;
 else
 count = 256;
 if ( fread( buffer, 1, count, InputCarFile ) != count )
 FatalError( "Error reading input file %s", Header.file_name );
 Header.compressed_size -= count;
 if ( fwrite( buffer, 1, count, OutputCarfile) != count )
 FatalError( "Error writing to output CAR file" );
 }
}
/*
* When the operation requested by the user is 'List', this routine is
* called to print out the column headers. List output goes to standard
* output, unlike most of the other messages in this program, which go
* to stderr.
*/
void PrintListTitles()
{
 printf( "\n" );
 printf( " Original Compressed\n" );
 printf( "Filename Size Size Ratio CRC-32 Method\n" );
 printf( "________ ____ ____ _________________\n" );
}
/*
* When the List command is given, the main loop reads in each header
* block, then tests to see if the file name in the header block matches
* one of the file names (including wildcards) in the FileList. If it is,
* this routine is called to print out the information on the file.
*/
void ListCarFileEntry()
{
 static char *methods[] = { "Stored", "LZSS" };
printf( "%-20s %10lu %10lu %4d%% %081x %s\n", Header.file_name, Header.original_size, Header.compressed_size, RatioInPercent( Header.compressed_size, Header.original_size ), Header.original_crc, methods[ Header.compression_method - 1 ] );
}
/*
* The compression figure used in this book is calculated here. The value
* is scaled so that a file that has just been stored has a compression
* ratio of 0%, while one that has been shrunk down to nothing would have
* a ratio of 100%.
*/
int RatioInPercent( unsigned long compressed, unsigned long original )
//unsigned long compressed;
//unsigned long original;
{
 int result;
 if ( original == 0 )
 return( 0 );
 result = (int) ( ( 100L * compressed ) / original );
 return( 100 - result );
}
/*
* This routine is where all the information about the next file in
* the archive is read in. The data is read into the global Header
* structure. To preserve portability of CAR files across systems,
* the data in each file header is packed into an unsigned char array
* before it is written out to the file. To read this data back in
* to the Header structure, we first read it into another unsigned
* character array, then employ an unpacking routine to convert that
* data into ints and longs. This helps us avoid problems with
* big/little endian conflicts, as well as incompatibilities in structure
* packing, which show up even between different compilers targetted for
* the same architecture.
*
* To avoid causing any additional confusion, the data members for the
* header structure are at least stored in exactly the same order as
* they appear in the structure definition. The primary difference is
* that the entire file name character array is not stored, which would
* waste a lot of space. Instead, we just store the number of characters
* in the name, including the null termination character. The file name
* serves the additional purpose of identifying the end of the CAR file
* with a file name length of 0 bytes.
*/
int ReadFileHeader()
{
 unsigned char header_data[ 17 ];
 unsigned long header_crc;
 int i;
 int c;
 for ( i = 0 ; ; ) {
 c = getc( InputCarFile );
 Header.file_name[ i ] = (char) c;
 if ( c == '\0' )
 break;
 if ( ++i == FILENAME_MAX )
 FatalError ( "File name exceeded maximum in header" );
}
if ( i == 0 )
 return( 0 );
header_crc = CalculateBlockCRC32( i + 1, CRC_MASK, Header.file_name );
fread( header_data, 1, 17, InputCarFile );
Header.compression_method = (char) UnpackUnsignedData( 1, header_data + 0 );
Header.original_size = UnpackUnsignedData( 4, header_data + 1 );
Header.compressed_size = UnpackUnsignedData( 4, header_data + 5 );
Header.original_crc = UnpackUnsignedData( 4, header_data + 9 );
Header.header_crc = UnpackUnsignedData( 4, header_data + 13 );
header_crc = CalculateBlockCRC32(13, header_crc, header_data );
header_crc ^= CRC_MASK;
if ( Header.header_crc != header_crc )
 FatalError( "Header checksum error for file %s", Header.file_name );
return( 1 );
}
/*
* This routine is used to transform packed characters into unsigned
* integers. Its only purpose is to convert packed character data
* into integers and longs.
*/
unsigned long UnpackUnsignedData( int number_of_bytes, unsigned char *buffer )
//int number_of_bytes;
//unsigned char *buffer;
{
 unsigned long result;
 int shift_count;
 result = 0;
 shift_count = 0;
 while ( number_of_bytes-- > ) {
 result |= (unsigned long) * buffer++ << shift_count;
 shift_count += 8;
 }
 return( result );
}
/*
* This routine is called to write out the current Global header block
* to the output CAR file. It employs the same packing mechanism
* discussed earlier. This routine also calculates the CRC of the
* header, which is sometimes not necessary.
*/
void WriteFileHeader()
{
 unsigned char header_data[ 17 ];
 int i;
 for ( i = 0 ; ; ) {
 putc( Header.file_name[ i ], OutputCarFile );
 if ( Header.file_name[ i++ ] == '\0' )
 break;
}
Header.header_crc = CalculateBlockCRC32( i, CRC_MASK, Header.file_name );
PackUnsignedData( 1, (long)Header.compression_method,header_data + 0 );
PackUnsignedData( 4, Header.original_size, header_data + 1 );
PackUnsignedData( 4, Header.compressed_size, header_data + 5 );
PackUnsignedData( 4, Header.original_crc, header_data + 9 );
Header.header_crc = CalculatedBlockCRC32( 13, Header.header_crc, header_data );
Header.header_crc ^= CRC_MASK;
 PackUnsignedData( 4, Header.header_crc, header_data + 13 );
 fwrite( header_data, 1, 17, OutputCarFile );
}
/*
* This is the routine used to pack integers and longs into a character
* array. The character array is what eventually gets written out to the
* CAR file. The data is always written out with the least significant
* bytes of the integers or long integers going first.
*/
void PackUnsignedData( int number_of_bytes, unsigned long number, unsigned char buffer )
//int number_of_bytes;
//unsigned long number;
//unsigned char *buffer;
{
 while ( number_of_bytes-- > 0 ) {
 *buffer++ = ( unsigned char ) number & Oxff;
 number >>= 8;
 }
}
/*
* The last header in a CAR file is defined by the fact that it has
* a file name length of zero. Since the file name is the
* first element to be written out, we can create the final header
* by just writing out a null termination character. This technique
* saves a little bit of space.
*/
void WriteEndOfCarHeader()
{
 fputc( 0, OutputCarFile );
}
/*
* This is the routine called by the main processing loop and the
* Addfiles routine. It takes an input file and writes the header and
* file data to the Output CAR file. There are several complications that
* the routine has to deal with. First of all, the header information
* it gets when it first starts is incomplete. For instance, we don't
* know how many bytes the file will take up when it is compressed.
* Because of this, the position of the header is stored, and the
* incomplete copy is written out. After the compression routine finishes,
* the header is now complete. In order to put the correct header into
* the output CAR file, this routine seeks back in the file to the
* original header position and rewrites it.
*
* The second complication lies in the fact that some files are not very
* compressible. In fact, for some files the LZSS algorithm may actually
* cause the file to expand. In these cases, the compression routine
* gives up and passes a failure code back to Insert(). When this
* happens, the routine has to seek back to the start of the file, rewind
* the input file, and store it instead of compressing it. Because of
* this, the starting position of the file in the output CAR file is also
* stored when the routine starts up.
*/
void Insert( FILE *input_text_file, char operation )
//FILE *input_text_file;
//char *operation;
{
 long saved_position_of_header;
 long saved_position_of_file;
 fprintf( stderr, "%s %-20s", operation, Header, file_name );
 saved_position_of_header = ftell( OutputCarFile );
 Header.compression_method = 2;
 WriteFileHeader();
 saved_position_of_file = ftell(OutputCarFile);
 fseek( input_text_file, OL, SEEK_END );
 Header.original_size = ftell( input_text_file );
 fseek( input_text_file, OL, SEEK_SET );
 if ( !LZSSCompress( input_text_file ) ) {
 Header.compression_method = 1;
 fseek( OutputCarFile, saved_position_of_file, SEEK_SET );
 rewind( input_text_file );
 Store( input_text_file );
 }
 fclose( input_text_file );
 fseek( OutputCarFile, saved_position_of_header, SEEK_SET );
 WriteFileHeader();
 fseek( OutputCarFile, OL, SEEK_END );
 printf( "%d%%\n", RatioInPercent( Header.compressed_size, Header.original_size ) );
}
/*
* The Extract routine can be called for one of three reasons. If the
* file in the CAR is truly being extracted, Extract() is called with
* no destination specified. In this case, the Extract routine opens the
* file specified in the header and either unstores or decompresses the
* file from the CAR file. If the archive is being tested for veracity,
* the destination file will have been opened up earlier and specified as
* the null device. Finally, the 'Print' option may have been selected,
* in which case the destination file will be extracted to stdout.
*/
void Extract( FILE *destination )
//FILE *destination;
{
 FILE *output_text_file;
 unsigned long crc;
 int error;
 fprintf( stderr, "%-20s ", Header.file_name );
 error = 0;
 if ( destination == NULL ) {
 if ( ( output_text_file = fopen(Header.file_name, "wb")
 ) == NULL ) {
 fprintf( stderr, "Can't open %s\n", Header.file_name );
 fprintf( stderr, "Not extracted\n" );
 SkipOverFileFromInputCar();
 return;
 }
} else
 output_text_file = destination;
switch( Header.compression_method ) {
 case 1 :
 crc = Unstore( output_text_file );
 break;
 case 2 :
 crc = LZSSExpand( output_text_file );
 break;
 default :
 fprintf( stderr, "Unknown method: %c\n", Header.compression_method );
 SkipOverFileFromInputCar();
 error = 1;
 crc = Header.original_crc;
 break;
 }
 if ( crc != Header.original_crc ) { 
 fprintf( stderr, "CRC error reading data\n" );
 error = 1;
 }
 if ( destination == NULL ) {
 fclose( output_text_file );
 if ( error )
#ifdef __STDC__
 remove( Header.file_name );
#else
 unlink( Header.file_name );
#endif
 }
 if ( !error )
 fprintf( stderr, " OK\n" );
}
/*
* The CAR manager program is capable of handling many different forms of
* compression. All the compression program has to do is obey a few
* simple rules. First of all, the compression routine is required
* to calculate the 32-bit CRC of the uncompressed data, and store the
* result in the file Header, so it can be written out by the Insert()
* routine. The expansion routine calculates the CRC of the file it
* creates, and returns it to Extract() for a check against the Header
* value. Second, the compression routine is required to quit if its
* output is going to exceed the length of the input file. It needs to
* quit *before* the output length passes the input, or problems will
* result. The compression routine is required to return a true or false
* value indicating whether or not the compression was a success. And
* finally, the expansion routine is expected to leave the file pointer
* to the Input CAR file positioned at the first byte of the next file
* header. This means it has to read in all the bytes of the compressed
* data, no more or less.
*
* All these things are relatively easy to accomplish for Store() and
* Unstore(), since they do no compression or expansion.
*
*/
int Store( FILE *input_text_file )
//FILE *input_text_file;
{
 unsigned int n;
 char buffer[ 256 ];
 int pacifier;
 pacifier = 0;
 Header.original_crc = CRC_MASK;
 while ( ( n = fread( buffer, 1, 256, input_text_file ) ) != 0 ) {
 fwrite( buffer, 1, n, OutputCarFile );
 Header.original_crc = CalculateBlockCRC32( n, Header.original_crc, buffer );
 if ( ( ++pacifier & 15 ) == 0 )
 putc( '.', stderr );
 }
 Header.compressed_size = Header.original_size;
 Header.original_crc ^= CRC_MASK;
 return( 1 );
}
unsigned long Unstore( FILE *destination )
//FILE *destination;
{
 unsigned long crc;
 unsigned int count;
 unsigned char buffer[ 256 ];
 int pacifier;
 pacifier = 0;
 crc = CRC_MASK;
 while ( Header.original_size != 0 ) {
 if ( Header.original_size > 256 )
 count = 256;
 else
 count = (int) Header.original_size;
 if ( fread( buffer, 1, count, InputCarFile ) != count )
 FatalError( "Can't read from input CAR file" );
 if ( fwrite( buffer, 1, count, destination ) != count ) {
 fprintf( stderr. "Error writing to output file" );
 return( ~Header.original_crc );
 }
 crc = CalculateBlockCRC32( count, crc, buffer );
 if ( destination != stdout && ( pacifier++ & 15 ) == 0 )
 putc( '.', stderr );
 Header.original_size -= count;
}
 return( crc ^ CRC_MASK );
}
/*
* The second set of compression routines is found here. These
* routines implement LZSS compression and expansion using 12-bit
* index pointers and 4-bit match lengths. These values were
* specifically chosen because they allow for "blocked I/O". Because
* of their values, we can pack match/length pairs into pairs of
* bytes, with characters that don't have matches going into single
* bytes. This helps increase I/O since single bit input and
* output does not have to be employed. Other than this single change,
* this code is identical to the LZSS code used earlier in the book.
*/
/*
* Various constants_used to define the compression parameters. The
* INDEX_BIT_COUNT tells how many bits we allocate to indices into the
* text window. This directly determines the WINDOW_SIZE. The
* LENGTH_BIT_COUNT tells how many bits we allocate for the length of
* an encode phrase. This determines the size of the look ahead buffer.
* The TREE_ROOT is a special node in the tree that always points to
* the root node of the binary phrase tree. END_OF_STREAM is a special
* index used to flag the fact that the file has been completely
* encoded, and there is no more data. UNUSED is the null index for
* the tree. MOD_WINDOW() is a macro used to perform arithmetic on tree
* indices.
*
*/
#define INDEX_BIT_COUNT 12
#define LENGTH_BIT_COUNT 4
#define WINDOW_SIZE ( 1 << INDEX_BIT_COUNT )
#define RAW_LOOK_AHEAD_SIZE ( 1 << LENGTH_BIT_COUNT )
#define BREAK_EVEN ( ( 1 + INDEX_BIT_COUNT + LENGTH_BIT_COUNT ) \
 / 9 )
#define LOOK_AHEAD_SIZE ( RAW_LOOK_AHEAD_SIZE + BREAK_EVEN )
#define TREE_ROOT WINDOW_SIZE
#define END_OF_STREAM 0
#define UNUSED 0
#define MOD_WINDOW( a ) ( ( a ) & ( WINDOW_SIZE - 1 ) )
/*
* These are the two global data structures used in this program.
* The window[] array is exactly that, the window of previously seen
* text, as well as the current look ahead text. The tree[] structure
* contains the binary tree of all of the strings in the window sorted
* in order.
*/
unsigned char window[ WINDOW_SIZE ];
struct {
 int parent;
 int smaller_child;
 int larger_child;
} tree[ WINDOW_SIZE + 1 ];
/*
* Function prototypes for both ANSI C compilers and their K&R brethren.
*/
#ifdef __STDC__
void InitTree( int r );
void ContractNode( int old_node, int new_node );
void ReplaceNode( int old_node, int new_node );
int FindNextNode( int node );
void DeleteString( int p );
int AddString( int new_node, int *match_position );
void InitOutputBuffer( void );
int FlushOutputBuffer( void );
int OutputChar( int data );
int OutputPair( int position, int length );
void InitInputBuffer( void );
int InputBit( void );
#else
void InitTree();
void ContractNode();
void ReplaceNode();
int FindNextNode();
void DeleteString();
int AddString();
void InitOutputBuffer();
int FlushOutputBuffer();
int OutputChar();
int OutputPair();
void InitInputBuffer();
int InputBit();
#endif
void InitTree( int r )
//int r;
{
 int i;
 for ( i = 0 ; i < ( WINDOW_SIZE + 1 ) ; i++ ) {
 tree[ i].parent = UNUSED;
 tree[ i].larger_child = UNUSED;
 tree[ i].smaller_child = UNUSED;
 }
 tree[ TREE_ROOT ].larger_child = r;
 tree[ r].parent = TREE_ROOT;
 tree[ r ].larger_child = UNUSED;
 tree[ r ].smaller_child = UNUSED;
}
/*
* This routine is used when a node is being deleted. The link to
* its descendant is broken by pulling the descendant in to overlay
* the existing link.
*/
void ContractNode( int old_node, int new_node )
//int old_node;
//int new_node;
{
 tree[ new_node ].parent = tree[ old_node ].parent;
 if ( tree[ tree[ old_node ].parent ].larger_child == old_node )
 tree[ tree[ old_node ].parent ].larger_child = new_node;
 else
 tree[ tree[ old_node ].parent ].smaller_child = new_node;
 tree[ old_node ].parent = UNUSED;
}
/*
* This routine is also used when a node is being deleted. However,
* in this case, it is being replaced by a node that was not previously
* in the tree.
*/
void ReplaceNode( int old_node, int new_node )
//int old_node;
//int new_node;
{
 int parent;
 parent = tree[ old_node ].parent;
 if ( tree [ parent ].smaller_child == old_node )
 tree[ parent ].smaller_child = new_node;
 else
 tree[ parent ].larger_child = new_node;
 tree[ new_node ] = tree[ old_node ];
 tree[ tree[ new_node ].smaller_child ].parent = new_node;
 tree[ tree[ new_node ].larger_child ].parent = new_node;
 tree[ old_node ].parent = UNUSED;
}
/*
* This routine is used to find the next smallest node after the node
* argument. It assumes that the node has a smaller child. We find
* the next smallest child by going to the smaller_child node, then
* going to the end of the larger_child descendant chain.
*/
int FindNextNode( int node )
//int node;
{
 int next;
 next = tree[ node ].smaller_child;
 while ( tree [ next ].larger_child != UNUSED )
 next = tree[ next ].larger_child;
 return( next );
}
/*
* This routine performs the classic binary tree deletion algorithm.
* If the node to be deleted has a null link in either direction, we
* just pull the non-null link up one to replace the existing link.
* If both links exist, we instead delete the next link in order, which
* is guaranteed to have a null link, then replace the node to be deleted
* with the next link.
*/
void DeleteString( int p )
//int p;
{
 int replacement;
 if ( tree[ p ].parent == UNUSED )
 return;
 if ( tree[ p ].larger_child == UNUSED )
 ContractNode( p, tree [ p ].smaller_child );
 else if (tree[ p ].smaller_child == UNUSED )
 ContractNode( p , tree[ p ].larger_child );
 else {
 replacement = FindNextNode( p );
 DeleteString( replacement );
 ReplaceNode( p , replacement );
 }
}
/*
* This is where most of the work done by the encoder takes place. This
* routine is responsible for adding the new node to the binary tree.
* It also has to find the best match among all the existing nodes in
* the tree, and return that to the calling routine. To make mattes
* even more complicated, if the new_node has a duplicate in the tree,
* the old_node is deleted, for reasons of efficiency.
*/
int AddString( int new_node, int *match_position )
//int new_node;
//int *match_position;
{
 int i;
 int test_node;
 int delta;
 int match_length;
 int *child;
 if ( new_node == END_OF_STREAM )
 return( 0 ) ;
 test_node = tree[ TREE_ROOT ].larger_child;
 match_length = 0;
 for ( ; ; ) {
 for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ ) {
 delta = window[ MOD_WINDOW( new_node + 1 ) ] - window[ MOD_WINDOW( test_node + i ) ];
 if ( delta != 0 )
 break;
 }
 if ( i >= match_length ) {
 match_length = i;
 *match_position = test_node;
 if ( match_length >= LOOK_AHEAD_SIZE ) {
 ReplaceNode( test_node, new_node );
 return( match_length );
 }
 }
 if ( delta >= 0 )
 child = &tree[ test_node ].larger_child;
 else
 child = &tree[ test_node ].smaller_child;
 if ( *child == UNUSED ) {
 *child = new_node;
 tree[ new_node].parent = test_node;
 tree[ new_node].larger_child = UNUSED;
 tree[ new_node].smaller_child = UNUSED;
 return( match_length );
 }
 test_node = *child;
 }
}
/*
* This section of code and data makes up the blocked I/O portion of the
* program. Every token output consists of a single flag bit, followed
* by either a single character or a index/length pair. The flag bits
* are stored in the first byte of a buffer array, and the characters
* and index/length pairs are stored sequentially in the remaining
* positions in the array. After every eight output operations, the
* first character of the array is full of flag bits, so the remaining
* bytes stored in the array can be output. This can be done with a
* single fwrite() operation, making for greater efficiency.
*
* All that is needed to implement this is a few routines, plus three
* data objects, which follow below. The buffer has the flag bits
* packed into its first character, with the remainder consisting of
* the characters and index/length pairs, appearing in the order they
* were output. The FlagBitMask is used to indicate where the next
* flag bit will go when packed into DataBuffer[ 0 ]. Finally, the
* BufferOffset is used to indicate where the next token will be stored
* in the buffer.
*/
char DataBuffer[ 17 ];
int FlagBitMask;
int BufferOffset;
/*
* To initialize the output buffer, we set the FlagBitMask to the first
* bit position, can clear DataBuffer[0], which will hold all the
* Flag bits. Finally, the BufferOffset is set to 1, which is where the
* first character or index/length pair will go.
*/
void InitOutputBuffer()
{
 DataBuffer[ 0 ] = 0;
 FlagBitMask = 1;
 BufferOffset = 1;
}
/*
* This routine is called during one of two different situations. First,
* it can potentially be called right after a character or a length/index
* pair is added to the DataBuffer[]. If the position of the bit in the
* FlagBitMask indicates that it is full, the output routine calls this
* routine to flush data into the output file, and reset the output
* variables to their initial state. The other time this routine is
* called is when the compression routine is ready to exit. If there is
* any data in the buffer at that time, it needs to the flushed.
*
* Note that this routine checks carefully to be sure that it doesn't
* ever write out more data than was in the original uncompressed file.
* It returns a 0 if this happens, which filters back to the compression
* program, so that it can abort if this happens.
*
*/
int FlushOutputBuffer()
{
 if ( BufferOffset == 1 )
 return( 1 );
 Header.compressed_size += BufferOffset;
 if ( ( Header.compressed_size ) >= Header.original_size )
 return( 0 );
 if ( fwrite( DataBuffer, 1, BufferOffset, OutputCarFile )
 !=BufferOffset )
 FatalError( "Error writing compressed data to CAR file" );
 InitOutputBuffer();
 return( 1 );
}
/*
* This routine adds a single character to the output buffer. In this
* case, the flag bit is set, indicating that the next character is an
* uncompressed byte. After setting the flag and storing the byte,
* the flag bit is shifted over, and checked. If it turns out that all
* eight bits in the flag bit character are used up, then we have to
* flush the buffer and reinitialize the data. Note that if the
* FlushOutputBuffer() routine detects that the output has grown larger
* than the input, it returns a 0 back to the calling routine.
*/
int OutputChar( int data )
//int data;
{
 DataBuffer[ BufferOffset++ ] = (char) data;
 DataBuffer[ 0 ] |= FlagBitMask;
 FlagBitMask <<= 1;
 if ( FlagBitMask == 0x100 )
 return( FlushOutputBuffer() );
 else
 return( 1 );
}
/*
* This routine is called to output a 12-bit position pointer and a 4-bit
* length. The 4-bit length is shifted to the top four bits of the first
* of two DataBuffer[] characters. The lower four bits contain the upper
* four bits of the 12-bit position index. The next of the two DataBuffer
* characters gets the lower eight bits of the position index. After
* all that work to store those 16 bits, the FlagBitMask is shifted over,
* and checked to see if we have used up all our bits. If we have,
* the output buffer is flushed, and the output data elements are reset.
* If the FlushOutputBuffer routine detects that the output file has
* grown too large, it passes and error return back via this routine,
* so that it can abort.
*/
int OutputPair( int position, int length )
//int position;
//int length;
{
 DataBuffer[ BufferOffset ] = (char) ( length << 4 );
 DataBuffer[ BufferOffest++ ] |= ( position >> 8 );
 DataBuffer[ BufferOffset++ ] = (char) ( position & Oxff );
 FlagBitMask <<= 1;
 if ( FlagBitMask == 0x100 )
 return( FlushOutputBuffer() );
 else
 return( 1 );
}
/*
* The input process uses the same data structures as the blocked output
* routines, but it is somewhat simpler, in that it doesn't actually have
* to read in a whole block of data at once. Instead, it just reads in
* a single character full of flag bits into DataBuffer[0], and passes
* individual bits back to the Expansion program when asked for them.
* The expansion program is left to its own devices for reading in the
* characters, indices, and match lengths. They can be read in
* sequentially using normal file I/0.
*/
void InitInputBuffer()
{
 FlagBitMask = 1;
 DataBuffer[ 0 ] = (char) getc( InputCarFile );
}
/*
* When the Expansion program wants a flag bit, it calls this routine.
* This routine has to keep track of whether or not it has run out of
* flag bits. If it has, it has to go back and reinitialize so as to
* have a fresh set.
*/
int InputBit()
{
 if ( FlagBitMask == 0x00 )
 InitInputBuffer();
 FlagBitMask <<= 1;
 return( DataBuffer[ 0 ] & ( FlagBitMask >> 1 ) );
}
/*
* This is the compression routine. It has to first load up the look
* ahead buffer, then go into the main compression loop. The main loop
* decides whether to output a single character or an index/length
* token that defines a phrase. Once the character or phrase has been
* sent out, another loop has to run. The second loop reads in new
* characters, deletes the strings that are overwritten by the new
* character, then adds the strings that are created by the new
* character. While running it has the additional responsibility of
* creating the checksum of the input data, and checking for when the
* output data grows too large. The program returns a success or failure
* indicator. It also has to update the original_crc and compressed_size
* elements in Header data structure.
*
*/
int LZSSCompress( FILE *input_text_file )
//FILE *input_text_file;
{
 int i;
 int c;
 int look_ahead_bytes;
 int current_position;
 int replace_count;
 int match_length;
 int match_position;
 Header.compressed_size = 0;
 Header.original_crc = CRC_MASK;
 InitOutputBuffer();
 current_position = 1;
 for ( i = 0 ; i < LOOK_AHEAD_SIZE ; i++ ) {
 if ( ( c = getc( input_text_file ) ) == EOF )
 break;
 window[ current_position + i ] = (unsigned char) c;
 Header.original_crc = UpdateCharacterCRC32( Header.original_crc, c );
 }
 look_ahead_bytes = i;
 InitTree( current_position );
 match_length = 0;
 match_position = 0;
 while ( look_ahead_bytes > 0 ) {
 if ( match_length > look_ahead_bytes )
 match_length = look_ahead_bytes;
 if ( match_length <= BREAK_EVEN ) {
 replace_count = 1;
 if ( ! OutputChar( window[ current_position ] ) )
 return( 0 );
 } else {
 if ( !OutputPair( match_position, match_length - ( BREAK_EVEN + 1 ) ) )
 return( 0 );
 replace_count = match_length;
 }
 for ( i = 0 ; i < replace_count ; i++ ) {
 DeleteString( MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) );
 if ( ( c = getc( input_text_file ) ) == EOF ) {
 look ahead bytes--;
 } else {
 Header.original_crc = UpdateCharacterCRC32( Header.original_crc, c );
 window[ MOD_WINDOW( current_position + LOOK_AHEAD_SIZE ) ] = (unsigned char) c;
 }
 current_position = MOD_WINDOW( current_position + 1 );
 if ( current_position == 0 )
 putc( '.', stderr );
 if ( look_ahead_bytes )
 match_length = AddString( current_position, &match_position );
 }
 };
 Header.original_crc ^= CRC_MASK;
 return( FlushOutputBuffer() );
}
/*
* This is the expansion routine for the LZSS algorithm. All it has to do
* is read in flag bits, decide whether to read in a character or a
* index/length pair, and take the appropriate action. It is responsible
* for keeping track of the crc of the output data, and must return it
* to the calling routine, for verification.
*/
unsigned long LZSSExpand( FILE *output )
//FILE *output;
{
 int i;
 int current_position;
 int c;
 int match_length;
 int match_position;
 unsigned long crc;
 unsigned long output_count;
 output_count = 0;
 crc = CRC_MASK;
 InitInputBuffer();
 current_position = 1;
 while ( output_count < Header.original_size ) {
 if ( InputBit() ) {
 c = getc( InputCarFile );
 putc( c, output );
 output_count++;
 crc = UpdateCharacterCRC32( crc, c );
 window[ current_position ] = (unsigned char) c;
 current_position = MOD_WINDOW( current_position + 1 );
 if ( current_position == 0 && output != stdout )
 putc( '.', stderr );
 } else {
 match_length = getc( InputCarFile );
 match_position = getc( InputCarFile );
 match_position |= (match_length & Oxf ) << 8;
 match_length >>= 4; match_length += BREAK_EVEN;
 output_count += match_length + 1;
 for ( i = 0 ; i <= match_length ; i++ ) {
 c = window[ MOD_WINDOW( match_position + i ) ];
 putc( c, output );
 crc = UpdateCharacterCRC32( crc, c );
 window[ current_position ] = (unsigned char) c;
 current_position = MOD_WINDOW( current_position + 1 );
 if ( current_position == 0 && output != stdout )
 putc( '.', stderr );
 }
 }
 }
 return( crc ^ CRC_MASK );
}
/*************************** End of CARMAN.C ***************************/
