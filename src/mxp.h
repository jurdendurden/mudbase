/*
 * header file for mxp.c
 *
 * Brian Graversen
 */
#include <arpa/telnet.h>

/* MXP defs */
#define MXP_SAFE                        1
#define MXP_ALL                         2
#define MXP_NONE                        3

// Protocol defs:
#define TELOPT_MXP                     91 //mud extension protocol
#define TELOPT_MSP					   90 //mud sound protocol
#define TELOPT_COMPRESS 			   85 //mccp

/* global strings */
extern const char mxp_do[];
extern const char mxp_dont[];
extern const char mxp_will[];


//Mud Sound Protocol Defines
extern const char msp_do[];
extern const char msp_dont[];
extern const char msp_will[];