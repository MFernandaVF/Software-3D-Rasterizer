/*
 * osw.h
 */

#ifndef __LIBOSW_H__
#define __LIBOSW_H__

#include <libosw/types.h>
#include <libosw/video.h>
#include <libosw/input.h>
#include <libosw/joypad.h>
#include <libosw/osw_keys.h>

#define OSW_OK					0x00000000u
#define OSW_ERROR_NO_WINDOW		0x00000001u
#define OSW_ERROR_NO_JOYPAD		0x00000002u

#define OSW_FLAG_USE_OPENGL		0x1u

#define MAX(a, b)		((a) > (b) ? (a) : (b))
#define MIN(a, b)		((a) > (b) ? (b) : (a))


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum KtExitCode {
	OSW_EXIT,			/* Close the window */
	OSW_EXIT_CANCEL,	/* Cancels the window closing procedure */
	OSW_EXIT_MAX_CODES
};

typedef u32 (*WSExitFunc)(u32 status);


/*Main functions*/
u32  OSW_Init(const char* title, u32 w, u32 h, u32 flags); /*Inits the OS window*/
void OSW_ExitFuncSet(WSExitFunc func);                     /*Set exit function when OS window closes*/
void OSW_Exit(u32 status);                                 /*Closes the OS window*/
void OSW_Poll(void);                                       /*Gathers input*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LIBOSW_H__*/
