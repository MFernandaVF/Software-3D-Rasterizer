/*
 * joypad.h
 */


#ifndef __LIBOSW_JOYPAD_H__
#define __LIBOSW_JOYPAD_H__

#include <libosw/types.h>


#define JOY_LEFT       	0x0001u
#define JOY_RIGHT       0x0002u
#define JOY_UP          0x0004u
#define JOY_DOWN       	0x0008u
#define JOY_SOUTH       0x0010u
#define JOY_EAST        0x0020u
#define JOY_NORTH       0x0040u
#define JOY_WEST        0x0080u
#define JOY_L       	0x0100u
#define JOY_R           0x0200u
#define JOY_TL          0x0400u
#define JOY_TR       	0x0800u
#define JOY_STR       	0x1000u
#define JOY_SEL         0x2000u

#define JOY_BIT_LEFT      0x0u
#define JOY_BIT_RIGHT     0x1u
#define JOY_BIT_UP        0x2u
#define JOY_BIT_DOWN      0x3u
#define JOY_BIT_SOUTH     0x4u
#define JOY_BIT_EAST      0x5u
#define JOY_BIT_NORTH     0x6u
#define JOY_BIT_WEST      0x7u
#define JOY_BIT_L         0x8u
#define JOY_BIT_R         0x9u
#define JOY_BIT_TL        0xAu
#define JOY_BIT_TR        0xBu
#define JOY_BIT_STR       0xCu
#define JOY_BIT_SEL       0xDu

#define JOY_STICK_LX    0x00u
#define JOY_STICK_LY    0x01u
#define JOY_STICK_RX    0x02u
#define JOY_STICK_RY    0x03u


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


enum JoyId {
	JOY0,
	JOY1,
	JOY2,
	JOY3,
	MAX_JOYPADS					/* Max number of joypads. */
};


u32  OSW_JoyIsActive(u32 joy_id);	         /*Returns 1 if joypad is active, else 0*/
u32  OSW_JoyButtonHeld(u32 joy_id);          /*Returns held buttons*/
u32  OSW_JoyButtonUp(u32 joy_id);	         /*Returns released buttons*/
u32  OSW_JoyButtonDown(u32 joy_id);          /*Returns pressed buttons*/
s8   OSW_JoyStick(u32 joy_id, u32 stick_id); /*Returns stick values*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LIBOSW_OYPAD_H__*/
