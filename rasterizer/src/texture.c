/*
 * texture.c: Carga y muestreo de texturas BMP
 * v1.0 - public domain
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <texture.h>

/**
 * Estructura de un BMP de 24 bits sin compresión:
 *   - Cabecera de 14 bytes + DIB header de 40 bytes (total 54 bytes)
 *   - En la cabecera, el offset 10 dice dónde empiezan los datos
 *   - Los datos vienen como filas de píxeles BGR (no RGB), de ABAJO HACIA ARRIBA,
 *     con cada fila rellenada con padding hasta que su tamaño sea múltiplo de 4
 */
u32 texture_load_bmp(Texture *tex, const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 1;  // no se pudo abrir

    // Leemos la cabecera completa (54 bytes en BMPs comunes)
    unsigned char hdr[54];
    if (fread(hdr, 1, 54, f) != 54) { fclose(f); return 2; }

    // Verificamos que sea un BMP ("BM" al inicio)
    if (hdr[0] != 'B' || hdr[1] != 'M') { fclose(f); return 3; }

    // Extraemos información del header
    u32 data_off = hdr[10] | (hdr[11] << 8) | (hdr[12] << 16) | (hdr[13] << 24);
    u32 width    = hdr[18] | (hdr[19] << 8) | (hdr[20] << 16) | (hdr[21] << 24);
    u32 height   = hdr[22] | (hdr[23] << 8) | (hdr[24] << 16) | (hdr[25] << 24);
    u32 bpp      = hdr[28] | (hdr[29] << 8);

    // Solo soportamos BMPs de 24 bits sin compresión
    if (bpp != 24) { fclose(f); return 4; }

    // Reservamos memoria para los píxeles: 3 floats (RGB) por píxel
    float *pixels = malloc(sizeof(float) * 3 * width * height);
    if (!pixels) { fclose(f); return 5; }

    // Calculamos el padding: cada fila debe ser múltiplo de 4 bytes
    int row_bytes = width * 3;
    int padding   = (4 - (row_bytes % 4)) % 4;

    // Saltamos al inicio de los datos
    fseek(f, data_off, SEEK_SET);

    // Leemos cada fila. BMP las guarda DE ABAJO HACIA ARRIBA, así que volteamos Y
    // al guardarlas en nuestro arreglo (que va de arriba hacia abajo, más intuitivo)
    for (int y = (int)height - 1; y >= 0; y--) {
        for (u32 x = 0; x < width; x++) {
            unsigned char bgr[3];
            fread(bgr, 1, 3, f);

            // BMP guarda los colores como BGR, IGUAL que como los espera nuestro framebuffer,
            // así que NO hacemos conversión a RGB — solo normalizamos de 0-255 a 0.0-1.0
            u32 dst = (y * width + x) * 3;
            pixels[dst + 0] = bgr[0] / 255.0f;  // B
            pixels[dst + 1] = bgr[1] / 255.0f;  // G
            pixels[dst + 2] = bgr[2] / 255.0f;  // R
        }
        // Saltamos el padding al final de la fila
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);

    tex->pixels = pixels;
    tex->width  = width;
    tex->height = height;

    return 0;
}

/**
 * Función para liberar la memoria de una textura. 
 * Después de llamar a esta función, el puntero de píxeles se pone a NULL y 
 * el ancho y alto a 0 para evitar accesos accidentales.
 */
void texture_free(Texture *tex) {
    free(tex->pixels);
    tex->pixels = NULL;
    tex->width  = 0;
    tex->height = 0;
}

/**
 * Función para muestrear la textura en coordenadas UV.
 * Las UV pueden estar fuera de [0,1], así que aplicamos "wrap": las repetimos.
 * Por ejemplo, u=1.3 equivale a u=0.3, y u=-0.2 equivale a u=0.8.
 */
vec3 texture_sample(const Texture *tex, float u, float v) {
    // WRAP: traemos u y v al rango [0,1] restando la parte entera
    u = u - floorf(u);
    v = v - floorf(v);

    // Convertimos UV a coordenadas de píxel
    // Nota: V se invierte porque en las UV V=0 es la base y V=1 es el tope,
    // pero en nuestro arreglo de píxeles la fila 0 es la de arriba
    int px = (int)(u * (tex->width - 1));
    int py = (int)((1.0f - v) * (tex->height - 1));

    // Por seguridad, recortamos si quedaran fuera por algún error de redondeo
    if (px < 0) px = 0;
    if (py < 0) py = 0;
    if (px >= (int)tex->width)  px = tex->width - 1;
    if (py >= (int)tex->height) py = tex->height - 1;

    // Leemos los 3 floats RGB del píxel
    u32 idx = (py * tex->width + px) * 3;
    vec3 c;
    c.x = tex->pixels[idx + 0];
    c.y = tex->pixels[idx + 1];
    c.z = tex->pixels[idx + 2];
    
    return c;
}