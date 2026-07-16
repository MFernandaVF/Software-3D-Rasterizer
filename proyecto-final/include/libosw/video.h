/*
 * video.h
 */

#ifndef __LIBOSW_VIDEO_H__
#define __LIBOSW_VIDEO_H__

#include <libosw/types.h>


#define OSW_VIDEO_MAX_WIDTH     1920						/* Max width of frame. */
#define OSW_VIDEO_MAX_HEIGHT    1080						/* Max height of frame. */

/* Filter type when not using a graphics API */
typedef enum ViFilter_e {
	OSW_VIDEO_FILTER_SHARP,
	OSW_VIDEO_FILTER_SOFT,
	OSW_VIDEO_MAX_FILTER
} ViFilter;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ============================================================================
 * Video Functions
 * ============================================================================
 */
void OSW_VideoSetSize(u32 w, u32 h);       /* Sets the window size */
void OSW_VideoGetSize(u32 *w, u32 *h);     /* Gets the window size */
void OSW_VideoTitleSet(const char* title); /* Sets the window title */
void OSW_VideoHintFPS(u32 fps);            /* Hints for window refresh FPS */
void OSW_VideoSwapBuffers(void);           /* Swaps the frame buffers */
void OSW_VideoDrawBuffer(const void *fb, u32 w, u32 h); /* Draws the framebuffer (Only when not using OpenGL) */

/*================================================*/
void OSW_VideoSetFilter(ViFilter filter); /*Unused*/

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LIBOSW_VIDEO_H__*/
