/* solve.c 

The command line arguments are as follows:

    -p <filename>          # input file (gInputFileName)
    -discount %f           # discount factor (gDiscount)
    -method %s             # solution method (gMethod)
    -epsilon %f            # convergence test (gEpsilon)
    -weight %f             # weighted heuristic (gWeight)
    -v                     # Turns on verbose mode (gVerbose)
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "solve.h"
#include "graph.h"
#include "lao.h"
#include "vi.h"
#include "rtdp.h"
#include <string.h>

/* Globals with default values */
char        gInputFileName[80] = "big.dat";
MethodType  gMethod =            lao;
double      gDiscountOverride =  -1.0;
double      gEpsilon =           0.001;
double      gWeight =            1.0; 

#ifdef __cplusplus
bool    gVerbose =
#else
MyBoolean    gVerbose =
#endif

#ifdef __cplusplus
	false;
#else
	MyFalse;
#endif

LaoBackupMethod gBackupMethod =  combine;/* separate; */

/* Other globals */
int         gNumStates;
int         gNumActions;
double      gDiscount; /*=1.0;*/
double      gError;
clock_t     gStartTime;
clock_t     gEndTime;
static void GetParamString(char *string, int argc, char **argv, const char *flag)
/* Returns the string in argv that follows the flag string.
   (i.e., it processes the command line arguments and returns
   the string that corresponds to the argument after a flag. */
{
   int i;
   string[0] = '\0';
   
   for(i = 0; i < argc; i++) {
      if (strcmp(flag, argv[i]) == 0) {
         strcpy(string, argv[i+1]);
         return;
      } /* if */
   }
}

static void ProcessCommandLine( int argc, char **argv ) {
   char str[80];
   int i;

   /* Get name of input file */
   GetParamString(str, argc, argv, "-p");
   if( str[0] != '\0' )
     strcpy(gInputFileName, str);

   /* Set discount factor */
   /* Will this be overwritten when input file is read? */
   GetParamString( str, argc, argv, "-discount" );
   if( str[0] != '\0' )
      gDiscountOverride = strtod( str, NULL );

   /* Set which algorithm to use */
   GetParamString(str, argc, argv, "-method");
   if( str[0] != '\0' ) 
      for( i = 0; i < NUM_METHODS; i++)
         if( strcmp( str, method_str[ (MethodType) i ]) == 0 )
            gMethod = (MethodType) i;

   /* Set backup method for LAO* */
   GetParamString(str, argc, argv, "-backup");
   if( str[0] != '\0' ) 
      for( i = 0; i < NUM_BACKUP_METHODS; i++)
         if( strcmp( str, backup_str[ (LaoBackupMethod) i ]) == 0 )
            gBackupMethod = (LaoBackupMethod) i;

   /* Set desired epsilon-optimality of solution */
   GetParamString( str, argc, argv, "-epsilon" );
   if( str[0] != '\0' )
     gEpsilon = strtod( str, NULL );

   /* Set weighted heuristic */
   GetParamString( str, argc, argv, "-weight" );
   if( str[0] != '\0' )
     gWeight = strtod( str, NULL );

   /* Set verbose mode if specified */
   for(i = 0; i < argc; i++) 
      if (strcmp("-v", argv[i]) == 0) 
#ifndef __cpluplus
         gVerbose = true;
#else
	gVerbose = MyTrue;
#endif

}

static void DisplayHeader( void )
{
  printf("\n***************************************************************");
  printf("\nSolve %s with %d states, %d actions and discount of %f", 
	 gInputFileName, gNumStates, gNumActions, gDiscount);
  printf("\nUsing %s with epsilon %f",
	 method_str[gMethod], gEpsilon);
  if (gMethod == lao) {
    if(gBackupMethod == combine)
      printf(", combined backup and expand");
    else 
      printf(", separate backup and expand");
  }
  if ( gWeight != 1.0 )
    printf(", and heuristic with weight %f", gWeight);
  printf("\n***************************************************************");
}
