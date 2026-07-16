/*
 * input.h
 */

#ifndef __LIBOSW_INPUT_H__
#define __LIBOSW_INPUT_H__

#include <libosw/types.h>


#define OSW_KEYEV_TYPE_PRESSED       0u
#define OSW_KEYEV_TYPE_RELEASED      1u

#define OSW_MOUSE_BTN0               0x1u
#define OSW_MOUSE_BTN1               0x2u

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct WSKeyEvent_t{
	u8  type;
	u8  keycode;
	u16 mod;	/*Modifiers*/
	u16 sym;	/*Leave?*/
} OSWKeyEvent;


typedef struct WSMouse_t{
	s16 y;
	s16 x;
	s16 dy;
	s16 dx;
	u8 btn;  /*Pressed buttons */
	u8 btn0; /*count for btn0*/
	u8 btn1; /*count for btn1*/
	s8 scroll;
} OSWMouse;


/*Keyboard Functions*/
void OSW_KeyboardSetPolling(u32 enable);		/*Enables/Disables Keyboard polling*/
u32  OSW_KeyboardGetEvent(OSWKeyEvent *kev);	/*Obtains one keyboard event, returns false when there are no more events left*/
u32  OSW_KeyboardFlushEvents(void);				/*Discards all keyboard events, returns number of*/

/*Mouse Functions*/
void OSW_MouseSetPolling(u32 enable);			/*Enables/Disables Mouse polling*/
u32  OSW_MouseGetState(OSWMouse *mouse);		/*Fills mouse state*/


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LIBOSW_INPUT_H__*/
