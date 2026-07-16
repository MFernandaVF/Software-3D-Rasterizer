#include <libosw/osw.h>
#include <string.h>
#include <math.h>
#include <gs.h>

#define VIEWPORT_X 0
#define VIEWPORT_Y 0
#define VIEWPORT_W 320
#define VIEWPORT_H 240 

/** 
 * ------------------------------------------------------------------
 * Framebuffer, Z-buffer y Buffer de IDs de partes
 * ------------------------------------------------------------------
 */
u32 framebuffer[FRAMEBUFFER_W * FRAMEBUFFER_H];
float zbuffer[FRAMEBUFFER_W * FRAMEBUFFER_H];
u32 gs_id_buffer[FRAMEBUFFER_W * FRAMEBUFFER_H];

/**
 * ------------------------------------------------------------------
 * Variables globales del pipeline gráfico: matrices de 
 * transformación y configuración del viewport
 * ------------------------------------------------------------------ 
 */
mat4 gs_model;
mat4 gs_view;
mat4 gs_projection;

u32 gs_clear_color = 0x000000;
f32 gs_z_clear = 1.0f;

u32 gs_vp_x = 0, 
    gs_vp_y = 0, 
    gs_vp_w = FRAMEBUFFER_W, 
    gs_vp_h = FRAMEBUFFER_H;

u32 gs_blend_enabled = 0;  // por defecto SIN blend: los modelos se ven opacos

/**
 * ------------------------------------------------------------------
 * Estado global de iluminación y texturizado
 * ------------------------------------------------------------------
 */
Light    gs_light0   = {{0}, {0}, 0.0f, 0};       // luz direccional apagada por defecto
Light    gs_light1   = {{0}, {0}, 0.0f, 0};       // luz secundaria apagada por defecto
Material gs_material = {0.3f, 0.9f, 0.1f, 16.0f}; // valores razonables por defecto
Texture  gs_texture  = {NULL, 0, 0};              // textura vacía por defecto
vec3     gs_part_tints[NUM_PARTS];                // tinte de color para cada parte
vec3     gs_ambient  = {0.3f, 0.3f, 0.3f};        // luz ambiental suave
vec3     gs_camera_pos = {0.0f, 0.0f, 0.0f};      // posición de la cámara

/**
 * ------------------------------------------------------------------
 * Buffers auxiliares para rasterización de triángulos
 * ------------------------------------------------------------------
 */
static int leftEdge[FRAMEBUFFER_H];
static int rightEdge[FRAMEBUFFER_H];
static float leftZ[FRAMEBUFFER_H];
static float rightZ[FRAMEBUFFER_H];

/**
 * ------------------------------------------------------------------
 * Buffers de color por arista, igual que leftZ/rightZ pero para 
 * interpolar el color a lo largo de las filas del triángulo
 * ------------------------------------------------------------------
 */
static float leftR[FRAMEBUFFER_H],  leftG[FRAMEBUFFER_H],  leftB[FRAMEBUFFER_H];
static float rightR[FRAMEBUFFER_H], rightG[FRAMEBUFFER_H], rightB[FRAMEBUFFER_H];

/**
 * ------------------------------------------------------------------
 * Funciones internas que se usan en gs_DrawArrays y gs_DrawElems 
 * para dibujar cada primitiva de 1 en 1
 * ------------------------------------------------------------------
 */
static void __gs_DrawPoint(Vert v0);
static void __gs_DrawLine(Vert v0, Vert v1);
static void __gs_DrawTriangle(Vert v0, Vert v1, Vert v2);
static void __gs_TransformVert(Vert v, float *sx, float *sy, float *sz);
static void __gs_BlendPixel  (u32 x, u32 y, u32 face_color);
static void __gs_EdgeBresenham(u32 x0, u32 y0, float z0, vec3 c0, u32 x1, u32 y1, float z1, vec3 c1);
static Vert __gs_VertexShader(Vert v);
static vec3 __gs_FragmentShader(float r, float g, float b);

/**
 * Función para inicializar OSW, configurar el viewport por defecto y 
 * limpiar el framebuffer
 */
u32 gs_Init(const char *title, u32 win_w, u32 win_h) {
    u32 err = OSW_Init(title, win_w, win_h, 0);
    if (err != OSW_OK) return err;

    gs_Viewport(0, 0, FRAMEBUFFER_W, FRAMEBUFFER_H);
    gs_SetClearColor(0x000000, 1.0f);

    mat4_identity(gs_model);
    mat4_identity(gs_view);
    mat4_identity(gs_projection);

    gs_Clear();

    for (u32 i = 0; i < NUM_PARTS; i++) {
        gs_part_tints[i] = (vec3){1.0f, 1.0f, 1.0f};
    }

    return OSW_OK;
}

/**
 * Función para configurar el viewport que se usará para transformar 
 * las coordenadas de los vértices a coordenadas de píxel en pantalla
 */
void gs_Viewport(u32 x, u32 y, u32 w, u32 h) {
    gs_vp_x = x;
    gs_vp_y = y;
    gs_vp_w = w;
    gs_vp_h = h;
}

/**
 * Función para dibujar el contenido del framebuffer y hacer swap de 
 * buffers para mostrar el siguiente frame
 */
void gs_DrawBuffer(void) {
    OSW_VideoDrawBuffer(framebuffer, FRAMEBUFFER_W, FRAMEBUFFER_H);
    OSW_VideoSwapBuffers();
}

/**
 * Función para limpiar el framebuffer y el z-buffer antes de dibujar 
 * cada frame
 */
void gs_Clear(void) {
    u32 total = FRAMEBUFFER_W * FRAMEBUFFER_H;

    for (u32 i = 0; i < total; i++) {
        // Limpiamos el framebuffer con el color de fondo (negro)
        framebuffer[i] = gs_clear_color;
        // Reiniciamos el z-buffer con el valor Z de limpieza (sin nada pintado)
        zbuffer[i] = gs_z_clear;
        // Limpiamos también el id_buffer: GS_NO_PART = "este pixel no tiene parte"
        gs_id_buffer[i] = GS_NO_PART;
    }
}

/**
 * Función que configura el color con el que gs_Clear limpia el framebuffer 
 * cada frame y el valor Z con el que se reinicia el z-buffer
 */
void gs_SetClearColor(u32 clear_color, f32 z_clear) {
    gs_clear_color = clear_color;
    gs_z_clear = z_clear;
}

/**
 * Función para pintar un color en la coordenada (x, y) en el framebuffer
 */
void gs_PokePixel(u32 x, u32 y, u32 color) {
    framebuffer[y * FRAMEBUFFER_W + x] = color;
}

/**
 * Función que dibuja un array de vértices como puntos, líneas o triángulos 
 * dependiendo del tipo de primitiva indicado
 */
void gs_DrawArrays(u32 prim_type, Vert *v_arr, u32 v_count) {
	switch (prim_type)
	{
	case GS_TYPE_POINTS: // Si cada vértice es un punto
		for (u32 i = 0; i < v_count; i++)
		{
			__gs_DrawPoint(v_arr[i]);
		}
		break;
	
	case GS_TYPE_LINES: // Si cada par de vértices forma una línea
		for (u32 i = 0; i < v_count; i += 2)
		{
			__gs_DrawLine(v_arr[i], v_arr[i + 1]);
		}
		break;
	
	case GS_TYPE_TRIANGLES: // Si cada tripleta de vértices forma un triángulo
		for (u32 i = 0; i < v_count; i += 3)
		{
			__gs_DrawTriangle(v_arr[i], v_arr[i + 1], v_arr[i + 2]);
		}
		break;
	}
}

/**
 * Función que dibuja un array de vértices usando un array de índices para 
 * formar puntos, líneas o triángulos dependiendo del tipo de primitiva indicado
 */
void gs_DrawElems(u32 prim_type, Vert *v_arr, u32 v_count, u32 *i_arr, u32 i_count) {
	switch (prim_type)
	{
	case GS_TYPE_POINTS: // Si cada vértice es un punto
		for (u32 i = 0; i < i_count; i++)
		{
			__gs_DrawPoint(v_arr[i_arr[i]]);
		}
		break;
	
	case GS_TYPE_LINES: // Si cada par de vértices forma una línea
		for (u32 i = 0; i < i_count; i += 2)
		{
			__gs_DrawLine(v_arr[i_arr[i]], v_arr[i_arr[i + 1]]);
		}
		break;
	
	case GS_TYPE_TRIANGLES: // Si cada tripleta de vértices forma un triángulo
		for (u32 i = 0; i < i_count; i += 3)
		{
			__gs_DrawTriangle(v_arr[i_arr[i]], v_arr[i_arr[i + 1]], v_arr[i_arr[i + 2]]);
		}
		break;
	}
}

/**
 * Función que calcula el color iluminado de un vértice usando el modelo de Phong
 * (ambiente + difuso + especular), con dos luces direccionales.
 * 
 * Se llama una vez por vértice (no por píxel), porque hacemos Gouraud:
 * iluminamos en los vértices y el rasterizador interpola.
 */
static vec3 __gs_ComputeLighting(vec3 world_pos, vec3 world_normal, vec3 base_color) {
    // Empezamos con la componente AMBIENTAL: luz de fondo que siempre hay,
    // independiente de las luces y la orientación
    vec3 result;
    result.x = gs_material.ka * gs_ambient.x * base_color.x;
    result.y = gs_material.ka * gs_ambient.y * base_color.y;
    result.z = gs_material.ka * gs_ambient.z * base_color.z;

    // Vector hacia la cámara (V), necesario para el especular
    vec3 V = {0};
    V = vec3_sub(V, gs_camera_pos, world_pos);
    vec3_normalize(&V);

    // Procesamos las dos luces, sumando su contribución al resultado
    Light *lights[2] = { &gs_light0, &gs_light1 };

    for (int i = 0; i < 2; i++) {
        Light *light = lights[i];
        if (!light->enabled) continue;  // si está apagada, la saltamos

        // L: vector que apunta DESDE el vértice HACIA la luz
        // (es el opuesto de la dirección de la luz, porque dir va "saliendo" de la luz)
        vec3 L;
        L.x = -light->dir.x;
        L.y = -light->dir.y;
        L.z = -light->dir.z;
        vec3_normalize(&L);

        // DIFUSA: cuanto más de frente le pega la luz a la superficie, más brilla
        // N·L da el coseno del ángulo entre la normal y la luz
        float ndotl = vec3_dot(world_normal, L);

        if (ndotl > 0.0f) {  // si <= 0, la luz pega por detrás, no contribuye
            float diff = gs_material.kd * ndotl * light->intensity;

            result.x += diff * light->color.x * base_color.x;
            result.y += diff * light->color.y * base_color.y;
            result.z += diff * light->color.z * base_color.z;

            // ESPECULAR: brillo concentrado en el ángulo de reflexión
            // R es la dirección reflejada de L respecto a la normal
            vec3 R = {0};
            R = vec3_reflect(R, L, world_normal);
            vec3_normalize(&R);

            // R·V: qué tan alineado está el brillo con la cámara
            float rdotv = vec3_dot(R, V);
            if (rdotv > 0.0f) {
                // Elevado a shininess para concentrar el brillo en un punto pequeño
                float spec_pow = 1.0f;
                for (int k = 0; k < (int)gs_material.shininess; k++) spec_pow *= rdotv;

                float spec = gs_material.ks * spec_pow * light->intensity;
                result.x += spec * light->color.x;
                result.y += spec * light->color.y;
                result.z += spec * light->color.z;
            }
        }
    }

    // Limitamos a [0,1] para que no se desborde el color
    return vec3_clamp(result, 0.0f, 1.0f);
}

/**
 * ------------------------------------------------------------------
 * ------------------------ PIPELINE GRÁFICO ------------------------
 * ------------------------------------------------------------------
 */
/**
 * Función auxiliar que transforma un vértice desde espacio local 
 * hasta coordenadas de pantalla siguiendo el pipeline
 */
static void __gs_TransformVert(Vert v, float *sx, float *sy, float *sz) {
    vec3 pos = {v.pos.x, v.pos.y, v.pos.z};

    // MODELO: espacio local -> espacio global
    vec3_mat4Mul(&pos, gs_model, pos);

    // VISTA: espacio global -> espacio de vista (cámara)
    vec3_mat4Mul(&pos, gs_view, pos);

    // PROYECCIÓN: espacio de vista -> espacio de recorte (clipping)
    float pos_w = vec3_mat4Mul(&pos, gs_projection, pos);

    // Evitamos divisiones entre cero
    if (pos_w == 0.0f) pos_w = 0.0001f;

    // Normalizamos las coordenadas de recorte a coordenadas de pantalla
    pos.x /= pos_w;
    pos.y /= pos_w;
    pos.z /= pos_w;

    // VIEWPORT: espacio de recorte -> espacio de pantalla
    *sx = (pos.x + 1.0f) * 0.5f * gs_vp_w + gs_vp_x;
    *sy = (1.0f - pos.y) * 0.5f * gs_vp_h + gs_vp_y;
    *sz = (pos.z + 1.0f) * 0.5f;
}

/**
 * Función auxiliar para simular transparencia al mezclar el color 
 * de la cara con el color de fondo
 */
static void __gs_BlendPixel(u32 x, u32 y, u32 face_color) {
    // Si el blending está apagado, escribimos el color tal cual
    if (!gs_blend_enabled) {
        framebuffer[y * FRAMEBUFFER_W + x] = face_color;
        return;
    }

    /**
     * Modo blend: mezclamos 30% figura + 70% fondo (para transparencia)
     */
    // Obtenemos el color actual del framebuffer (fondo)
    u32 bg = framebuffer[y * FRAMEBUFFER_W + x];

    // Extraemos componentes RGB del fragmento
    u32 fr = (face_color >> 16) & 0xFF;
    u32 fg = (face_color >> 8) & 0xFF;
    u32 fb = face_color & 0xFF;

    // Extraemos componentes RGB del fondo
    u32 br = (bg >> 16) & 0xFF;
    u32 bgc = (bg >> 8) & 0xFF;
    u32 bb = bg & 0xFF;

    // Mezclamos los colores para simular transparencia
    u32 nr = (fr * 30 + br * 70) / 100;
    u32 ng = (fg * 30 + bgc * 70) / 100;
    u32 nb = (fb * 30 + bb * 70) / 100;
    framebuffer[y * FRAMEBUFFER_W + x] = (nr << 16) | (ng << 8) | nb;
}

/**
 * Función interna que dibuja un punto en pantalla a partir 
 * de un vértice, usando el color del vértice
 */
static void __gs_DrawPoint(Vert v0) {
    float sx, sy, sz;
    
    // Transformamos el vértice a coordenadas de pantalla
    __gs_TransformVert(v0, &sx, &sy, &sz);

    u32 color = ((u32)(v0.color.x * 255) << 16) 
              | ((u32)(v0.color.y * 255) << 8) 
              | (u32)(v0.color.z * 255);

    // Dibujamos el pixel en pantalla con el color del vértice
    gs_PokePixel((u32)sx, (u32)sy, color);
}

/**
 * Función interna que dibuja una línea entre dos vértices 
 * usando el algoritmo de Bresenham
 */
static void __gs_DrawLine(Vert v0, Vert v1) {
    float sx0, sy0, sz0, sx1, sy1, sz1;

    // Transformamos ambos vértices a coordenadas de pantalla
    __gs_TransformVert(v0, &sx0, &sy0, &sz0);
    __gs_TransformVert(v1, &sx1, &sy1, &sz1);

    // Convertimos a enteros para la rasterización
    int x0 = (int)sx0, y0 = (int)sy0;
    int x1 = (int)sx1, y1 = (int)sy1;

    // Distancias en X y Y para el algoritmo de Bresenham
    int dx = abs(x1 - x0), dy = abs(y1 - y0);

    // Dirección de la línea (hacia dónde se mueve el algoritmo)
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    // Error acumulado para decidir cuándo mover en Y
    int err = dx - dy;

    while(1) {
        // Dibujamos el pixel actual de la línea
        gs_PokePixel(x0, y0, 0xFFFFFF);
        if (x0 == x1 && y0 == y1) break;

        // Variable auxiliar para decidir cuándo mover en Y
        int e2 = 2*err;

        if (e2 >- dy) { // Ajuste en X
            err -= dy; 
            x0 += sx;
        }

        if (e2 < dx) { // Ajuste en Y
            err += dx;
            y0 += sy;
        }
    }
}

/**
 * VERTEX SHADER: procesa cada vértice individualmente.
 * 
 * Toma un vértice con sus atributos (posición, normal, UV, color, part_id)
 * y devuelve el mismo vértice con su color reemplazado por el color iluminado
 * que el rasterizador interpolará entre vértices (Gouraud shading).
 * 
 * Hace:
 *   1. Transforma la posición y la normal al espacio del mundo
 *   2. Muestrea la textura usando las UV
 *   3. Aplica el tinte de la parte (si está asignada)
 *   4. Calcula la iluminación (ambiente + difuso + especular)
 */
static Vert __gs_VertexShader(Vert v) {
    // Transformamos posición y normal al mundo (model * pos, model * normal)
    // Como solo usamos rotación (no escalado no uniforme), aplicar gs_model
    // a la normal directamente es suficiente
    vec3 wp, wn;
    vec3_mat4Mul(&wp, gs_model, v.pos);
    vec3_mat4Mul(&wn, gs_model, v.normal);
    vec3_normalize(&wn);

    // Color base: si hay textura, lo muestreamos con las UV.
    // Si no, usamos el color que trae el vértice
    vec3 base = v.color;
    if (gs_texture.pixels) {
        base = texture_sample(&gs_texture, v.uv.x, v.uv.y);
    }

    // Tinte por parte: multiplicamos el color base por el tinte de la parte.
    // Si la parte no tiene tinte (es blanco 1,1,1), el color no cambia.
    // Si el part_id es inválido (0xFFFFFFFF, "sin asignar"), no se aplica.
    if (v.part_id < NUM_PARTS) {
        base.x *= gs_part_tints[v.part_id].x;
        base.y *= gs_part_tints[v.part_id].y;
        base.z *= gs_part_tints[v.part_id].z;
    }

    // Iluminación Gouraud: calculamos el color final del vértice usando 
    // ambiente + difuso + especular sobre el color base
    v.color = __gs_ComputeLighting(wp, wn, base);

    return v;
}

/**
 * FRAGMENT SHADER: procesa cada píxel del triángulo.
 * 
 * Recibe el color (r, g, b) interpolado entre los tres vértices del triángulo
 * y devuelve el color final del píxel listo para escribir al framebuffer.
 * 
 * Por ahora solo hace clamp para evitar desbordes al empaquetar a u32.
 */
static vec3 __gs_FragmentShader(float r, float g, float b) {
    vec3 c;

    // Recortamos cada canal a [0,1] antes de empaquetar a entero,
    // porque la interpolación puede producir valores ligeramente fuera de rango
    // y al castear a u32 se desbordarían al canal de al lado, causando ruido
    c.x = r < 0 ? 0 : (r > 1 ? 1 : r);
    c.y = g < 0 ? 0 : (g > 1 ? 1 : g);
    c.z = b < 0 ? 0 : (b > 1 ? 1 : b);

    return c;
}

/**
 * Función interna que dibuja un triángulo entre tres vértices usando
 * un algoritmo de rasterización basado en el método de los bordes y
 * el z-buffer para manejar la profundidad
 */
static void __gs_DrawTriangle(Vert v0, Vert v1, Vert v2) {
    /**
     * VERTEX SHADER: procesa cada vértice (transformación, iluminación, textura, tinte)
     */
    v0 = __gs_VertexShader(v0);
    v1 = __gs_VertexShader(v1);
    v2 = __gs_VertexShader(v2);
    
    // El part_id del triángulo: usamos el del v0 (los 3 vértices suelen compartirlo
    // porque agrupamos por partes en Blender)
    u32 tri_part_id = v0.part_id;

    float sx0, sy0, sz0, sx1, sy1, sz1, sx2, sy2, sz2;

    // Transformamos los tres vértices a coordenadas de pantalla
    __gs_TransformVert(v0, &sx0, &sy0, &sz0);
    __gs_TransformVert(v1, &sx1, &sy1, &sz1);
    __gs_TransformVert(v2, &sx2, &sy2, &sz2);

    // Calculamos el área con signo en pantalla.
    // El flip de Y en el viewport invierte el winding: area >= 0 significa cara trasera.
    float area = (sx1 - sx0) * (sy2 - sy0) - (sx2 - sx0) * (sy1 - sy0);
    if (area >= 0.0f) return;  // backface culling: descartamos caras traseras y degeneradas

    int x0 = (int)sx0, y0 = (int)sy0;
    int x1 = (int)sx1, y1 = (int)sy1;
    int x2 = (int)sx2, y2 = (int)sy2;

    // Calculamos el rango vertical (min | max) que ocupa el triángulo en pantalla
    int min_y = y0;
    if (y1 < min_y) min_y = y1;
    if (y2 < min_y) min_y = y2;

    int max_y = y0;
    if (y1 > max_y) max_y = y1;
    if (y2 > max_y) max_y = y2;

    if (min_y < 0) min_y = 0;
    if (max_y >= FRAMEBUFFER_H) max_y = FRAMEBUFFER_H - 1;

    // Inicializamos los buffers de los bordes del triángulo 
    // para la rasterización
    for (int i = min_y; i <= max_y; i++) {
        leftEdge[i] = 999999;
        rightEdge[i] = -999999;
    }

    // Recorremos las 3 aristas del triángulo usando Bresenham
    // y guardamos los límites izquierdo/derecho por fila
    __gs_EdgeBresenham(x0, y0, sz0, v0.color, x1, y1, sz1, v1.color);
    __gs_EdgeBresenham(x1, y1, sz1, v1.color, x2, y2, sz2, v2.color);
    __gs_EdgeBresenham(x2, y2, sz2, v2.color, x0, y0, sz0, v0.color);

    // RELLENO POR SCANLINE con interpolación de Z y color
    for (int y = min_y; y <= max_y; y++) {
        int lx = leftEdge[y];
        int rx = rightEdge[y];
        if (lx > rx) continue;

        int span = rx - lx;

        // Valores iniciales: los del borde izquierdo
        float z = leftZ[y];
        float r = leftR[y], g = leftG[y], b = leftB[y];

        // Incrementos por píxel: cuánto cambia cada valor de izquierda a derecha
        float dz = (span > 0) ? (rightZ[y] - leftZ[y]) / span : 0;
        float dr = (span > 0) ? (rightR[y] - leftR[y]) / span : 0;
        float dg = (span > 0) ? (rightG[y] - leftG[y]) / span : 0;
        float db = (span > 0) ? (rightB[y] - leftB[y]) / span : 0;

        for (int x = lx; x <= rx; x++) {
            if (x < 0 || x >= FRAMEBUFFER_W) continue;

            int idx = y * FRAMEBUFFER_W + x;
            if (z < zbuffer[idx]) {
                zbuffer[idx] = z;
                gs_id_buffer[idx] = tri_part_id;   // NUEVO: registramos qué parte ocupa este pixel

                /**
                 * FRAGMENT SHADER: procesa el color interpolado de este píxel
                 */
                vec3 final = __gs_FragmentShader(r, g, b);

                // Empaquetamos el color final a u32 (RGB) y lo escribimos al framebuffer
                u32 color = ((u32)(final.x * 255) << 16)
                          | ((u32)(final.y * 255) <<  8)
                          | ((u32)(final.z * 255));
                __gs_BlendPixel(x, y, color);
            }

            // Avanzamos todos los valores hacia el siguiente píxel
            z += dz;
            r += dr;
            g += dg;
            b += db;
        }
    }
}

/**
 * Función interna que recorre una arista del triángulo usando el
 * algoritmo de Bresenham y actualiza los buffers auxiliares de bordes
 * (leftEdge/rightEdge, leftZ/rightZ, leftRGB/rightRGB) para la
 * rasterización posterior por scanline
 */
static void __gs_EdgeBresenham(u32 x0, u32 y0, float z0, vec3 c0, u32 x1, u32 y1, float z1, vec3 c1) {
    // Convertimos a enteros con signo para evitar problemas de dirección
    int ix0 = (int)x0;
    int iy0 = (int)y0;
    int ix1 = (int)x1;
    int iy1 = (int)y1;

    // Distancias en X y Y
    int dx = abs(ix1 - ix0);
    int dy = abs(iy1 - iy0);

    // Dirección de la línea
    int sx = (ix0 < ix1) ? 1 : -1;
    int sy = (iy0 < iy1) ? 1 : -1;

    int err = dx - dy;

    int steps = (dx > dy) ? dx : dy; // Número de pasos para recorrer la línea
    float dz = (steps > 0) ? (z1 - z0) / steps : 0.0f; // Incremento de Z por paso
    
    // Junto al cálculo de dz, agregamos las deltas de cada canal de color
    float dr = (steps > 0) ? (c1.x - c0.x) / steps : 0.0f;
    float dg = (steps > 0) ? (c1.y - c0.y) / steps : 0.0f;
    float db = (steps > 0) ? (c1.z - c0.z) / steps : 0.0f;

    float z = z0;
    float r = c0.x, g = c0.y, b = c0.z;

    // Actualizamos los buffers de los bordes del triángulo para la rasterización
    while (1) {
        if (iy0 >= 0 && iy0 < (int)FRAMEBUFFER_H) {
            if (ix0 < leftEdge[iy0]) {
                leftEdge[iy0] = ix0;
                leftZ[iy0] = z;
                leftR[iy0] = r;  
                leftG[iy0] = g;  
                leftB[iy0] = b;
            }
            if (ix0 > rightEdge[iy0]) {
                rightEdge[iy0] = ix0;
                rightZ[iy0] = z;
                rightR[iy0] = r;  
                rightG[iy0] = g;  
                rightB[iy0] = b;
            }
        }

        if (ix0 == ix1 && iy0 == iy1) break;

        int e2 = 2 * err;

        if (e2 > -dy) { err -= dy; ix0 += sx; }
        if (e2 <  dx) { err += dx; iy0 += sy; }

        z += dz;
        r += dr;
        g += dg;
        b += db;
    }
}