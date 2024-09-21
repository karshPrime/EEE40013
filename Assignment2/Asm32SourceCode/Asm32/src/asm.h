/****************************/
/*******  ASM.H   ***********/
/****************************/

#if defined(SIM) || defined(MON)
extern int assem(address_t *, char *);
#endif

#ifdef ASM
extern int assem1(char *);
extern int assem2(char *);
extern void set_pass1(void);
extern void set_pass2(void);
extern int report_error_count(void);
#endif

typedef enum {TEXT_SEG,DATA_SEG,LAST_SEG=DATA_SEG} segment_type;
