#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/*
:--------------------------------------------------------:
| Revision History                                       |
|--------------------------------------------------------|
|  10 Oct 2001   |  Did the fix below properly!          |
|                |  Added enable pin to ROM              |
|--------------------------------------------------------|
|   2 Oct 2001   |  Changed to reverse search for '.'    |
|                |  in myOpen()                          |
|--------------------------------------------------------|
| 30 Sept 2001   |  Initial Release                      |
:--------------------------------------------------------:
*/

typedef unsigned char BYTE;

char *commandName = 0;

const int maxInputLineSize = 200; // Input lines (S1S9 records)
const int maxDataLineSize  = 16;  // # bytes per data line in .coe file

const int   startAddress = 0x0000;
const int   endAddress   = 0x03FF;
const int   romSize      = endAddress - startAddress + 1;
const int   romWidth     = 32; // in bits
const int   romHeight    = romSize * 32 / romWidth;
const char *deviceName   = "codememory";

BYTE romImage[0x10000] = {0};

unsigned int htoi( char ch ) {

   if ((ch >= '0')  && (ch <= '9'))
      return ch - '0';
   else if ((ch >= 'a')  && (ch <= 'f'))
      return ch - 'a' + 10;
   else if ((ch >= 'A')  && (ch <= 'F'))
      return ch - 'A' + 10;

   assert (0);

   return 0;
}

void readFile( FILE *ifile ) {

   char buffer[maxInputLineSize];

   while ( fgets( buffer, maxInputLineSize-1, ifile ) != 0) {
//    fputs( buffer, stdout ); // echo to standard output
      if ( (( buffer[0] == 's' ) || ( buffer[0] == 'S' ) ) &&
           ( buffer[1] == '1' )) {
         // data record
         int size        = (htoi(buffer[2])<<4)  + (htoi(buffer[3]));
         int destination = (htoi(buffer[4])<<12) + (htoi(buffer[5])<<8) +
                           (htoi(buffer[6])<<4)  + (htoi(buffer[7]));
         size -= 3; // adjust for pre-amble & checksum
//       printf( ":%2.2d:%4X:", size, destination );
         for (int count = 0; count < 2*size; count+=2, destination++) {
            romImage[destination] = (htoi(buffer[count+8])<<4) +
                                    (htoi(buffer[count+9]));
//          printf( "%2.2X", romImage[destination] );
            }
//       printf("\n");
         }

      }

}

void writePreamble( FILE *coeFile ) {

   fprintf( coeFile,
      "Component_Name                = %s;\n"
      "Width                         = %d;\n"
      "Depth                         = %d;\n"
      "Enable_Pin                    = False;\n"
      "Handshaking_Pins              = False;\n"
      "Register_Inputs               = False;\n"
      "Additional_Output_Pipe_Stages = 0;\n"
      "Init_Pin                      = False;\n"
      "Init_Value                    = 0;\n"
      "Has_Limit_Data_Pitch          = False;\n"
      "Port_configuration            = read_only;\n"
      "Memory_Initialization_Radix   = 16;\n"
      "Memory_Initialization_Vector  =\n",

      deviceName,
      romWidth,
      romHeight
      );
}

void printBinary( unsigned long value, FILE *mifFile ) {

	for (int i = 7, mask=0x80; i >= 0; i--, mask >>= 1)
		fprintf( mifFile, "%d", (value&mask)?1:0 );
}

void writeData( FILE *coeFile, FILE *mifFile ) {

   int byteCount = 0;

   for (int address = startAddress; address <= endAddress; address++) {
      fprintf( coeFile, "%2.2X", romImage[address] );
      printBinary( romImage[address], mifFile );
      if ((address != endAddress) &&
		  ((address & 0x03) == 3) ) {
         fprintf( coeFile, "," );
         fprintf( mifFile, "\n" );
	  }
      byteCount++;
      if (byteCount >= maxDataLineSize) {
         byteCount = 0;
         fprintf( coeFile, "\n" );
         }
      }

   fprintf( coeFile, "\n\n");
   fprintf( mifFile, "\n\n");
}

FILE *myopen( const char *path, const char *ext, const char *mode) {

   const int  maxPath = 200;
   char       pathBuffer[maxPath];
   FILE      *file = 0;

   if (strchr( mode, 'r') != 0) {// open for read ?
      // try opening without any fiddles
      file = fopen( path, mode );
      if (file != 0)
         return file;
      }

   // add default extention if necessary (no 'dot')
   strncpy( pathBuffer, path, maxPath );
   if (( ext != 0) && (strrchr( pathBuffer, '.') == 0))
      strcat( pathBuffer, ext );

   file = fopen( pathBuffer, mode );
   //printf( "Trying to open \"%s\", rc=%d\n", pathBuffer, file );

   return file;
}

void usage( void ) {

   printf( "Usage: %s InputFile[.s19] OutputFile[.coe]\n", commandName );
   exit( -1 );
}

int main( int argc, char *argv[]) {

   char *coeFilename;
   char *mifFilename;
   char *ifilename;

   commandName = argv[0];

   if (argc == 2) { // need to dummy output file name
      ifilename = argv[1];

      // Make output filename same as imput but without extention
      coeFilename = strdup( argv[1] );
      char *dotPtr = strchr( coeFilename, '.' );
      if (dotPtr != 0)
         *dotPtr = '\0';
      mifFilename = strdup( coeFilename );
      }
   else
      usage();

   FILE *ifile;

   if (((ifile = myopen( ifilename, ".s19", "r" )) == 0) &&
       ((ifile = myopen( ifilename, ".a07", "r" )) == 0)) {

      perror("Unable to open input file ");
      usage();
      }

   FILE *coeFile = myopen( coeFilename, ".coe", "w" );
   if (coeFile == 0) {
      perror("Unable to open output coe file ");
      usage();
      }

   if (coeFilename != argv[2])
      free( coeFilename );

   FILE *mifFile = myopen( mifFilename, ".mif", "w" );
   if (mifFile == 0) {
      perror("Unable to open output mif file ");
      usage();
      }

   if (mifFilename != argv[2])
      free( mifFilename );

   readFile( ifile );
   fclose( ifile );

   writePreamble( coeFile );

   writeData( coeFile, mifFile );

   fclose( coeFile );

   return 0;
}
