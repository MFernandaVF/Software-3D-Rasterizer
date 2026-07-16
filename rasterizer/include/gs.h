/*
 * gc.h: Graphics system functions
 * v1.0 - public domain
 */

#ifndef __GS_H__
#define __GS_H__

#include <trx.h>
#include <libosw/types.h>
#include <texture.h>

#define SG_DISP_W 320
#define SG_DISP_H 240

#define FRAMEBUFFER_W 320
#define FRAMEBUFFER_H 240

// Número total de partes definidas en el modelo (hechas en Blender)
#define NUM_PARTS 18

// Valor que indica "este pixel no tiene parte" (igual al "sin asignar" del modelo)
#define GS_NO_PART 0xFFFFFFFF

/* ==========================
   Temporal typedefs 
   ========================== */
typedef void VertShader;
typedef void FragShader;
/* ========================== */

/**
 * Estructura de un vértice: posición, normal, coordenada de textura y color base
 */
typedef struct Vert_t {
    vec3 pos;     // posición del vértice en espacio local
    vec3 normal;  // dirección hacia donde "apunta" la superficie, para la iluminación
    vec2 uv;      // coordenada de textura: qué punto de la imagen le corresponde
    vec3 color;   // color base del vértice cargado del modelo
    u32  part_id; // ID de la parte (cabeza=0, ojo_blanco_izq=1, etc.)
} Vert;

/**
 * Una luz direccional: como el sol, ilumina con una dirección fija
 * desde el infinito (no tiene posición, solo dirección)
 */
typedef struct Light_t {
    vec3 dir;       // dirección DESDE la que viene la luz (apuntando a la escena)
    vec3 color;     // color de la luz (1,1,1 = blanco puro)
    f32  intensity; // intensidad: 0 = apagada, 1 = normal, >1 = sobreexpuesta
    u32  enabled;   // 1 si está prendida, 0 si está apagada
} Light;

/**
 * Material: cómo reacciona la superficie a la luz.
 * Estos coeficientes vienen del modelo de Phong que vimos en clase.
 */
typedef struct Material_t {
    f32 ka;        // coeficiente ambiental (luz de fondo que siempre hay)
    f32 kd;        // coeficiente difuso (luz que rebota igual en todas direcciones)
    f32 ks;        // coeficiente especular (brillo concentrado)
    f32 shininess; // qué tan concentrado es el brillo (n alto = brillo pequeño y fuerte)
} Material;

/**
 * Estado global del sistema de iluminación y texturizado
 */
extern Light    gs_light0;                // luz principal (fija, siempre encendida)
extern Light    gs_light1;                // luz secundaria (toggleable con L)
extern Material gs_material;              // material actual del modelo
extern Texture  gs_texture;               // textura actual del modelo
extern vec3     gs_part_tints[NUM_PARTS]; // tinte de color para cada parte del modelo
extern vec3     gs_ambient;               // color de la luz ambiental de la escena
extern vec3     gs_camera_pos;            // posición de la cámara (para el especular)

enum PrimType {
	GS_TYPE_POINTS,
	GS_TYPE_LINES,
	GS_TYPE_TRIANGLES,
	GS_TYPE_MAX
};

/**
 * ------------------------------------------------------------------
 * Variables globales del pipeline
 * ------------------------------------------------------------------ 
 */
// Framebuffer donde se dibujan todos los píxeles
extern u32 framebuffer[FRAMEBUFFER_W * FRAMEBUFFER_H];

// Z-buffer paralelo al framebuffer: guarda la profundidad del píxel más cercano
extern float zbuffer[FRAMEBUFFER_W * FRAMEBUFFER_H];

// Buffer de IDs de partes paralelo al framebuffer, para picking con clic
extern u32 gs_id_buffer[FRAMEBUFFER_W * FRAMEBUFFER_H];

// Matriz model: transformaciones del objeto (rotación, traslación, escalamiento)
extern mat4 gs_model;

// Matriz view: transformaciones de la cámara (traslación, rotación)
extern mat4 gs_view;

// Matriz projection: perspectiva u ortogonal
extern mat4 gs_projection;

// Color de fondo usado por gs_Clear (formato 0xRRGGBB)
extern u32 gs_clear_color;

// Valor Z de limpieza
extern f32 gs_z_clear;

// Valores actuales del viewport
extern u32 gs_vp_x, gs_vp_y, gs_vp_w, gs_vp_h;

// Flag para activar/desactivar el blending (transparencia)
extern u32 gs_blend_enabled;

/** 
 * ------------------------------------------------------------------
 * Funciones del sistema gráfico
 * ------------------------------------------------------------------ 
 */
// Función para inicializar el sistema gráfico, configurar el viewport y limpiar el framebuffer
u32  gs_Init(const char *title, u32 win_w, u32 win_h);

// Función para configurar el viewport
void gs_Viewport(u32 x, u32 y, u32 w, u32 h);

// Función para mostrar el contenido del framebuffer en pantalla
void gs_DrawBuffer(void); 

// Función para limpiar la pantalla con el color asignado
void gs_Clear(void); 

// Función para asignar un color constante de limpieza
void gs_SetClearColor(u32 clear_color, f32 z_clear);

/* NO LO VEMOS AÚN */
void gs_UseProgram(VertShader *vsh, FragShader *fsh);

// Función para pintar un color en la coordenada (x, y) en el framebuffer
void gs_PokePixel(u32 x, u32 y, u32 color); 

// Función para dibujar un arreglo de primitivas (líneas, puntos, triángulos) --> va de 1 en 1
void gs_DrawArrays(u32 prim_type, Vert *v_arr, u32 v_count);

// Función para dibujar un arreglo de primitivas de índices (vértices a los que hacen referencia los índices)
void gs_DrawElems(u32 prim_type, Vert *v_arr, u32 v_count, u32 *i_arr, u32 i_count); 

#endif /*__GS_H__*/