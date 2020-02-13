/*
 * header file for mxp.c
 *
 * Brian Graversen
 * Updated by:			Upro 2010. Changed to protocols.h
 */
 
 
 /*
 
 
#include <arpa/telnet.h>

// MXP Defines:
#define MXP_SAFE                        1
#define MXP_ALL                         2
#define MXP_NONE                        3

// MSDP Defines:
#define MSDP_VAR         1
#define MSDP_VAL         2
#define MSDP_OPEN        3
#define MSDP_CLOSE       4

// MSSP Defines:
#define MSSP_FILE "../dev/mssp.dat"

#define MSSP_MINAGE  0
#define MSSP_MAXAGE  21

#define MSSP_MINCREATED 1970
#define MSSP_MAXCREATED 2100

#define MSSP_MAXVAL 20000
#define MAX_MSSP_VAR1 4
#define MAX_MSSP_VAR2 3

#define COMPRESS_BUF_SIZE 16384

// Protocol Defines:
#define TELOPT_MXP                     	91 //mud extension protocol
#define TELOPT_MSP					   	90 //mud sound protocol
#define TELOPT_COMPRESS 			   	85 //mccp
#define TELOPT_MSDP						69 



*/


/* global strings */
extern const char mxp_do[];
extern const char mxp_dont[];
extern const char mxp_will[];


// Mud Sound Protocol Defines
extern const char msp_do[];
extern const char msp_dont[];
extern const char msp_will[];




