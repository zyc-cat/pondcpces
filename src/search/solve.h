/* solve.h */

#ifndef __SOLVE_H
#define __SOLVE_H

/* A boolean type will be useful */

#ifndef __cplusplus
typedef enum { MyFalse, MyTrue } MyBoolean;
#endif


/* Change this as more algorithms are added */
#define NUM_METHODS  4
static const char *method_str[] = {
   "pi", "vi", "rtdp", "lao"
};
typedef enum {pi, vi, rtdp, lao} MethodType;

#define NUM_BACKUP_METHODS 2
static const char *backup_str[] = {
  "combine", "separate"
};
typedef enum {combine, separate} LaoBackupMethod;

extern char        gInputFileName[];
extern MethodType  gMethod;
extern double      gDiscount;
extern double      gEpsilon;
extern double      gWeight;

#ifndef __cplusplus
extern MyBoolean   
#else
extern bool
#endif
	gVerbose;

extern int         gNumStates;
extern int         gNumActions;
extern clock_t     gStartTime;
extern clock_t     gEndTime;
extern LaoBackupMethod gBackupMethod;
extern double gDiscountOverride;

#endif
