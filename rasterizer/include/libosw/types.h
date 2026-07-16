/*
 * types.h
 */


#ifndef __LIBOSW_TYPES_H__
#define __LIBOSW_TYPES_H__

#include <stdint.h>


#define OSW_TRUE       1
#define OSW_FALSE      0
#define OSW_ENABLE     1
#define OSW_DISABLE    0


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Integers */
typedef uint8_t   u8; 		/* 8bit unsigned integer  */
typedef uint16_t  u16;		/* 16bit unsigned integer */
typedef uint32_t  u32;		/* 32bit unsigned integer */
typedef uint64_t  u64;		/* 64bit unsigned integer */
typedef int8_t    s8; 		/* 8bit signed integer    */
typedef int16_t   s16;		/* 16bit signed integer   */
typedef int32_t   s32;		/* 32bit signed integer   */
typedef int64_t   s64;		/* 64bit signed integer   */

/* Volatile integers */
typedef volatile u8    vu8; 		/* 8bit unsigned volatile integer  */
typedef volatile u16   vu16;		/* 16bit unsigned volatile integer */
typedef volatile u32   vu32;		/* 32bit unsigned volatile integer */
typedef volatile u64   vu64;		/* 64bit unsigned volatile integer */
typedef volatile s8    vs8; 		/* 8bit unsigned volatile integer  */
typedef volatile s16   vs16;		/* 16bit unsigned volatile integer */
typedef volatile s32   vs32;		/* 32bit unsigned volatile integer */
typedef volatile s64   vs64;		/* 64bit unsigned volatile integer */

/* Floating point */
typedef float          f32; 		/* 32bit floating point */
typedef double         f64; 		/* 64bit floating point */
typedef volatile f32   vf32;		/* 32bit volatile floating point */
typedef volatile f64   vf64;		/* 64bit volatile floating point */

/* Boolean */
typedef u8    _bool_; 		/* Boolean    */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LIBOSW_TYPES_H__*/
