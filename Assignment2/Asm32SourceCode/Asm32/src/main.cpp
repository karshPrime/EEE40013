/*
**  main.c
*/
/**************************************************************
**	Revision History
**
** 13/8/92 - Added quiet option, banner() & changed usage()
**  2/6/92 - Added print of symbol table to list file
**************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>     /* exit, EXIT_FAILURE */

#if defined(__TURBOC__) || defined(WIN32)
#include <stdlib.h>
#endif
#if defined(__TURBOC__)
#include <dir.h>
#else
#include "dir.h"
#endif
#include "symbol.h"
#include "main.h"
#include "asm.h"

#undef debug

#define MAX_BYTE (0x20)      /*  Maximum length of data record */

FILE *sourcefile;  /* input source file */
FILE *objfile;     /* object file Motorola (S1-S9) format */
FILE *listfile;    /* listing file */

char sourcefilename[MAXPATH];
char objfilename[MAXPATH];
char listfilename[MAXPATH];

char *executename=NULL;
int  quiet;

void usage(void)
{
  fprintf(stderr,
    "Usage %s [source_filename] [options]\n\n"
    " Options -o filename : send output to filename\n"
    "         -l filename : send list output to filename\n"
    "         -q          : quiet - no banner\n"
    ,executename);
  exit(EXIT_FAILURE);
}

void f_header(char *data)
/*
   Writes a Motorola S1 record with the data in 'data'.

   Entry : data : a '\0' terminated string.
*/
{
int check_sum;
int data_size;
int count;

  data_size = strlen(data);

  fprintf(objfile,"S0%2.2X%4.4X",data_size+3,0);

  check_sum = data_size+3;

  for (count = 0;count < data_size;count++)
    {
    fprintf(objfile,"%2.2X",(int) *data);
    check_sum += ((int) *data++) & 0xff;
    }

  fprintf(objfile,"%2.2X\015\012",(-check_sum-1)&0xff);
}

static int       data_count=0;         /* # of bytes in data_buff */
static uint8_t    data_buff[MAX_BYTE];  /* buffer of bytes in S record */
static uint32_t data_address;         /* address of 1st byte in S record */

void flush_objfile(void)
{
int count;
uint8_t check_sum;

  if (data_count > 0) /* data in buffer ? */
    {
    fprintf(objfile,"S1%2.2X%4.4X",data_count+3,data_address);

	 check_sum = (char) (data_count + 3 +
							  (data_address>>8) +
							  (data_address & 0xff));

    for (count = 0; count<data_count; count++)
      {
      fprintf(objfile,"%2.2X",(uint8_t)data_buff[count]);
      check_sum += data_buff[count];
      }

    fprintf(objfile,"%2.2X\015\012",(-check_sum-1)&0xff);
    data_address += data_count;
    data_count = 0;
    }
}

void out_objfile(uint32_t address, uint8_t data)
{
  if ((address != data_address+data_count) || /* non-consecutive byte ? */
      (data_count >= MAX_BYTE))               /* or record full ? */
    {
    flush_objfile();                          /* yes - write data buffer */
    data_address = address;
    }

  data_buff[data_count++] = data; /* add byte to buffer */
}

void f_start(uint32_t start_address)
/*
    Writes a Motorola start address record.  This will
    be either a S8 or S9 record according to how large
    start_address is (3 or 2 bytes).
*/
{
uint8_t check_sum;
static int done_term;	/* 1 if called more than once */

  flush_objfile();             /* write any data in buffer */

  if (done_term)	/* ignore if called more than once */
    return;

  done_term=true;

  if (start_address <= 0xffff) /* two byte start address */
    {
	 check_sum = (char)(3 + (start_address>>8) + (start_address&0xff));

    fprintf(objfile,"S903%4.4X%2.2X\015\012",
	    start_address,(int)(-check_sum-1)&0xff);
    }
  else /* three byte start address */
    {
	 check_sum = (char)(4 + (start_address>>16)
								 + (start_address>>8)
								 +  start_address);

    fprintf(objfile,"S804%6.6X%2.2X\015\012",
	    start_address,(uint8_t)(~check_sum));
    }
}

void do_args(int  argc,  char *argv[])
{
char drive[MAXDRIVE],dir[MAXDIR],name[MAXFILE],ext[MAXEXT];
int  fparts;

  executename=argv[0];
  quiet = 0;	/* default is banner */

  while (--argc > 0) /* process each argument */
    {
#ifdef debug
    fprintf(stderr,"processing %s\n",*(argv+1));
#endif
    if (**++argv != '-') /* must be input filename */
      {
      if (sourcefilename[0] != '\0') /* sourcefile already given ? */
	{
	fprintf(stderr,"Too many filenames - %s\n",*argv);
        usage();
        }
      strncpy(sourcefilename,*argv,MAXPATH-1);
      }
    else
      {
      switch (*((*argv)+1))  /* char following '-' */
        {
	case 'o' :  /* output file name */
	    if (objfilename[0] != '\0') /* object file already given ? */
	      {
	      fprintf(stderr,"Too many output filenames\n");
	      usage();
	      }
	    if (*((*argv)+2) != '\0')
	      strncpy(objfilename,(*argv)+2,MAXPATH-1);
	    else if (argc <= 1)
	      {
	      fprintf(stderr,"-o option missing filename\n");
	      usage();
	      }
	    else
	      {
	      ++argv; --argc; /* get next arg */
	      strncpy(objfilename,(*argv),MAXPATH-1);
	      }
	    break;

	case 'l' :  /* list file name */
	    if (listfilename[0] != '\0') /* list file already given ? */
	      {
	      fprintf(stderr,"Too many list filenames\n");
	      usage();
	      }
	    if (*((*argv)+2) != '\0')
	      strncpy(listfilename,(*argv)+2,MAXPATH-1);
	    else if (argc <= 1)
	      {
	      fprintf(stderr,"-l option missing filename\n");
	      usage();
	      }
	    else
	      {
	      ++argv; --argc; /* get next arg */
	      strncpy(listfilename,(*argv),MAXPATH-1);
	      }
	    break;
	case 'q' :  /* quiet - no banner */
	    quiet = 1;
	    break;
	default :
            fprintf(stderr,"illegal argument - %s\n",*argv);
            usage();
	    break;
	}
      }
    }

  /*
  ** open input file
  */
  fparts=fnsplit(sourcefilename,drive,dir,name,ext);
  if (!(fparts&FILENAME)) /* must have input filename */
    {
    fprintf(stderr,"Input filename missing or invalid\n");
    usage();
    }
  if (!(fparts&EXTENSION)) /* default extension */
    strcpy(ext,".s");
  fnmerge(sourcefilename,drive,dir,name,ext);
  if ((sourcefile = fopen(sourcefilename,"rt")) == NULL)
    {
    fprintf(stderr,"Unable to open input file - %s\n",sourcefilename);
    usage();
    }

  /*
  ** open listing file
  */
  if (listfilename[0] == '\0') /* default listing file is infilename+".lst" */
    {
    strcpy(ext,".lst");
    fnmerge(listfilename,drive,dir,name,ext);
    }
  if ((listfile = fopen(listfilename,"wt")) == NULL)
    {
    fprintf(stderr,"Unable to open listing file - %s\n",listfilename);
    usage();
    }
  /*
  ** open object file
  */
  if (objfilename[0] == '\0') /* default object file is sourcefilename+".mot" */
    {
    strcpy(ext,".mot");
    fnmerge(objfilename,drive,dir,name,ext);
    }
  if ((objfile = fopen(objfilename,"wt")) == NULL)
    {
    fprintf(stderr,"Unable to open object file - %s\n",objfilename);
    usage();
    }

#ifdef debug
  printf("input  file = %s\n",sourcefilename);
  printf("list   file = %s\n",listfilename);
  printf("object file = %s\n",objfilename);
  listfile  = stderr; /* force output to screen */
#endif
}

static constexpr unsigned MAXLINE = 100;

void pass1(void) {

   char buff[MAXLINE+1];

   set_pass1();

   while ((fgets(buff,MAXLINE,sourcefile) != NULL) &&
         (assem1(buff)>=0))
      ;
}

int pass2(void) {

   char buff[MAXLINE+1];
   int err_count;

   rewind(sourcefile);

   f_header(sourcefilename);
   set_pass2();

   while (fgets(buff,MAXLINE,sourcefile) != NULL) {
      if (assem2(buff)<0) {
         break;
      }
   }
   f_start(0);

   err_count = report_error_count();
   print_symbol_table(listfile);

   fclose(listfile);
   fclose(objfile);
   fclose(sourcefile);
   return(err_count);
}

void banner(void) {

  if (!quiet)
    printf("ASM32 - Version date " __DATE__ "\n");
}

int main(int argc, char *argv[]) {

  do_args(argc,argv);
  banner();
  pass1();
  return((pass2()>0)?EXIT_SUCCESS:EXIT_FAILURE);
}
