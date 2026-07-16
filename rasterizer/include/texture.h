/*
 * texture.h: Carga y muestreo de texturas BMP
 * v1.0 - public domain
 */

#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#include <libosw/types.h>
#include <trx.h>

/**
 * Una textura cargada en memoria: los píxeles como floats BGR en [0,1]
 * para que sea fácil multiplicarla por el color iluminado del vértice.
 */
typedef struct Texture_t {
    float *pixels;   // arreglo de píxeles, 3 floats (BGR, orden nativo del BMP) por píxel, fila por fila
    u32    width;    // ancho en píxeles
    u32    height;   // alto en píxeles
} Texture;

/**
 * Función para cargar una textura desde un archivo BMP (24 bits, sin compresión).
 * Devuelve 0 si todo OK, código de error distinto de 0 si falla.
 */
u32 texture_load_bmp(Texture *tex, const char *path);

/**
 * Función para liberar la memoria de la textura
 */
void texture_free(Texture *tex);

/**
 * Función para muestrear la textura en coordenadas UV (de 0 a 1) y devolver el color.
 * Las UV fuera de [0,1] hacen "wrap" (se repiten), porque las UV del modelo
 * pueden estar fuera de ese rango.
 */
vec3 texture_sample(const Texture *tex, float u, float v);

#endif /*__TEXTURE_H__*/