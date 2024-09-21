#define MAXDRIVE	(10)
#define MAXDIR		(200)
#define MAXFILE		(100)
#define MAXEXT		(20)
#define MAXPATH		(MAXDRIVE+MAXDIR+MAXFILE+MAXEXT)

#define DRIVE		0x1
#define DIRECTORY 	0x2
#define FILENAME	0x4
#define EXTENSION	0x8


void fnmerge(char *p,
	    char *drive,
	    char *directory,
	    char *filename,
	    char *ext);

int fnsplit(char *p,
	    char *drive,
	    char *directory,
	    char *filename,
	    char *ext);
