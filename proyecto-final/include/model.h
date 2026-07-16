/*
 * model.h: Carga de modelos 3D en formato TDM
 * v1.0 - public domain
 */

#ifndef __MODEL_H__
#define __MODEL_H__

#include <gs.h>

/**
 * Un modelo cargado en memoria: el arreglo de vértices y el de índices
 * que dicen cómo conectarlos en triángulos.
 */
typedef struct Model_t {
    Vert *verts;          // arreglo de vértices (pos, normal, uv, color, part_id)
    u32  *indices;        // arreglo de índices: cada 3 forman un triángulo
    u32   vert_count;     // cuántos vértices hay
    u32   index_count;    // cuántos índices hay (index_count / 3 = triángulos)
} Model;

/**
 * Función que carga un modelo desde un archivo .TDM.
 * Reserva memoria para los vértices e índices (¡¡ recuerda liberar con model_free !!).
 * Devuelve 0 si todo salió bien, o un código de error distinto de 0 si falló.
 */
u32 model_load_tdm(Model *model, const char *path);

/**
 * Función que libera la memoria que reservó la función ''model_load_tdm''.
 */
void model_free(Model *model);

#endif /*__MODEL_H__*/