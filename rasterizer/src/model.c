/*
 * model.c: Carga de modelos 3D en formato TDM
 * v1.0 - public domain
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <model.h>

/**
 * Estructura del archivo TDM (todo en little-endian):
 *   - Cabecera (16 bytes): "TDMD" + version + num_vertices + num_indices
 *   - Vértices (40 bytes c/u): pos(3 floats) + normal(3 floats) + uv(2 floats) + color(1 u32) + part_id(1 u32)
 *   - Índices: num_indices enteros u32, cada 3 forman un triángulo
 * El color viene empaquetado con el byte bajo = R, luego G, luego B.
 */
u32 model_load_tdm(Model *model, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 1;  // No se pudo abrir el archivo

    // Leemos el "número mágico" y verificamos que sea un TDM válido
    char magic[4];
    fread(magic, 1, 4, f);
    if (memcmp(magic, "TDMD", 4) != 0) {
        fclose(f);
        return 2;  // El archivo no es un TDM
    }

    // Leemos el resto de la cabecera
    u32 version, vcount, icount;
    fread(&version, sizeof(u32), 1, f);
    fread(&vcount,  sizeof(u32), 1, f);
    fread(&icount,  sizeof(u32), 1, f);

    // Reservamos memoria para los vértices y los índices
    Vert *verts   = malloc(sizeof(Vert) * vcount);
    u32  *indices = malloc(sizeof(u32)  * icount);
    if (!verts || !indices) {
        free(verts);
        free(indices);
        fclose(f);
        return 3;  // No hubo memoria suficiente
    }

    // Leemos los vértices uno por uno
    for (u32 i = 0; i < vcount; i++) {
        float buf[8];   // pos(3) + normal(3) + uv(2)
        u32 packed;     // color empaquetado
        u32 part_id;    // NUEVO: id de la parte a la que pertenece este vértice

        fread(buf, sizeof(float), 8, f);
        fread(&packed, sizeof(u32), 1, f);
        fread(&part_id, sizeof(u32), 1, f);

        verts[i].pos.x = buf[0];
        verts[i].pos.y = buf[1];
        verts[i].pos.z = buf[2];

        verts[i].normal.x = buf[3];
        verts[i].normal.y = buf[4];
        verts[i].normal.z = buf[5];

        verts[i].uv.x = buf[6];
        verts[i].uv.y = buf[7];

        // Desempaquetamos el color a valores 0.0-1.0 (byte bajo = R)
        verts[i].color.x = (packed         & 0xFF) / 255.0f;
        verts[i].color.y = ((packed >> 8)  & 0xFF) / 255.0f;
        verts[i].color.z = ((packed >> 16) & 0xFF) / 255.0f;

        verts[i].part_id = part_id;   // NUEVO: asignar el part_id al vértice
    }

    // Leemos todos los índices de golpe
    fread(indices, sizeof(u32), icount, f);
    fclose(f);

    // Entregamos el modelo cargado
    model->verts       = verts;
    model->indices     = indices;
    model->vert_count  = vcount;
    model->index_count = icount;
    
    return 0;
}

/**
 * Función que libera la memoria del modelo y deja los punteros en NULL
 * para evitar usarlos por accidente después de liberarlos
 */
void model_free(Model *model) {
    free(model->verts);
    free(model->indices);
    model->verts       = NULL;
    model->indices     = NULL;
    model->vert_count  = 0;
    model->index_count = 0;
}