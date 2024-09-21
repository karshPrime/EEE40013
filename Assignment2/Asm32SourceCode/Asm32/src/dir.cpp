#include <stdio.h>
#include <string.h>
#include "dir.h"

void fnmerge(char *p,
	    char *drive,
	    char *directory,
	    char *filename,
	    char *ext)
{
  strncpy(p,drive,MAXDRIVE);
  p+=strlen(p);
  strncpy(p,directory,MAXDIR);
  p+=strlen(p);
  strncpy(p,filename,MAXFILE);
  p+=strlen(p);
  strncpy(p,ext,MAXEXT);
}

int fnsplit(char *p,
	    char *drive,
	    char *directory,
	    char *filename,
	    char *ext)
{
char path[MAXPATH];
char *t;
int flags=0;

  strcpy(directory,"directory");
  strcpy(filename,"filename");
  strcpy(ext,".ext");

  strncpy(path,p,MAXPATH); /* make a copy to hack */
  path[MAXPATH-1]='\0'; /* in case longer than MAXPATH */

  if ((t=strrchr(path,'.'))!= NULL) /* any extension */
    {
    strncpy(ext,t,MAXEXT);
    flags |= EXTENSION;
    *t = '\0';
    }
  else
    *ext='\0';
  if ((t=strrchr(path,'/'))!= NULL) /* any directory */
    {
    strncpy(filename,t+1,MAXFILE);
    *(t+1) = '\0';
    }
  else
    {
    strncpy(filename,path,MAXFILE);
    *path='\0';
    }
  if (*filename != '\0')
    flags |= FILENAME;

  strncpy(directory,path,MAXDIR);
  if (*directory != '\0')
    flags |= DIRECTORY;

  *drive = '\0'; /* no drives in UNIX */

  return(flags);
}

#if 0
void main(void)
{
char path[MAXPATH];
char drive[MAXDRIVE];
char dir[MAXDIR];
char fname[MAXFILE];
char ext[MAXEXT];


for(;;)
  {
  if (gets(path)==NULL)
    break;
  printf("path ==> \'%s\'\n",path);
  fnsplit(path,drive,dir,fname,ext);
  printf("\'%s\' ==> \'%s\'%s\'%s\'%s\'\n",path,drive,dir,fname,ext);
  }
}
#endif
