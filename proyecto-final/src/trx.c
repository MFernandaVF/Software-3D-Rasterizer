#include <string.h>
#include <math.h>
#include <trx.h>

/**
 * Función que suma dos vectores componente a componente.
 * En este proyecto se usa para acumular la contribución de las dos luces
 * sobre un mismo vértice (luz1 + luz2).
 */
vec3 vec3_add(vec3 dest, const vec3 v, const vec3 u) {
    dest.x = v.x + u.x;
    dest.y = v.y + u.y;
    dest.z = v.z + u.z;
    return dest;
}

/**
 * Función que resta dos vectores (v - u).
 * Sirve para obtener direcciones: la dirección de un punto hacia la cámara
 * es: (posicion_camara - posicion_vertice).
 */
vec3 vec3_sub(vec3 dest, const vec3 v, const vec3 u) {
    dest.x = v.x - u.x;
    dest.y = v.y - u.y;
    dest.z = v.z - u.z;
    return dest;
}

/**
 * Función que multiplica un vector por un escalar k (escala su longitud).
 * Se usa para graduar la intensidad de cada componente de luz antes de
 * sumarla al color final.
 */
vec3 vec3_smul(vec3 dest, float k, const vec3 v) {
    dest.x = k * v.x;
    dest.y = k * v.y;
    dest.z = k * v.z;
    return dest;
}

/**
 * Función que recorta cada componente del vector al rango [a, b].
 * Se usa al final de la iluminación para que el color no se pase de 1.0
 * (lo que causaría colores "quemados" o valores inválidos al pasar a 0-255).
 */
vec3 vec3_clamp(vec3 v, float a, float b) {
    if (v.x < a) v.x = a;
    if (v.x > b) v.x = b;
    if (v.y < a) v.y = a;
    if (v.y > b) v.y = b;
    if (v.z < a) v.z = a;
    if (v.z > b) v.z = b;
    return v;
}

/**
 * Función que calcula el producto punto entre dos vectores.
 * Es el corazón de la iluminación: N·L mide qué tan de frente le pega
 * la luz a la superficie, y R·V qué tan alineado está el brillo con la cámara.
 */
float vec3_dot(const vec3 v, const vec3 u) {
    return v.x*u.x + v.y*u.y + v.z*u.z;
}

/**
 * Función que refleja el vector m respecto a la normal p, usando R = 2*(p·m)*p - m.
 * Es la R del modelo de Phong: dada la dirección hacia la luz y la normal,
 * obtiene la dirección en la que la luz "rebota", para el brillo especular.
 * OBS. p (la normal) debe venir normalizada.
 */
vec3 vec3_reflect(vec3 dest, const vec3 m, const vec3 p) {
    float d = vec3_dot(p, m);
    dest.x = 2.0f*d*p.x - m.x;
    dest.y = 2.0f*d*p.y - m.y;
    dest.z = 2.0f*d*p.z - m.z;
    return dest;
}

/**
 * Función que normaliza un vector para que mida exactamente 1 de longitud,
 * conservando su dirección.
 * La iluminación necesita que las normales y las direcciones de luz
 * sean unitarias para que los productos punto den el coseno del ángulo.
 */
void vec3_normalize(vec3 *v) {
    float len = sqrtf(v->x*v->x + v->y*v->y + v->z*v->z);
    if (len == 0.0f) return;   // Evitamos dividir entre cero
    v->x /= len;
    v->y /= len;
    v->z /= len;
}

/**
 * Función interna que transforma un vértice del espacio 3D 
 * al espacio de pantalla
 */
float vec3_mat4Mul(vec3 *dest, const mat4 m, vec3 p) {
    dest->x = m[0][0]*p.x + m[0][1]*p.y + m[0][2]*p.z + m[0][3]*1.0f;
    dest->y = m[1][0]*p.x + m[1][1]*p.y + m[1][2]*p.z + m[1][3]*1.0f;
    dest->z = m[2][0]*p.x + m[2][1]*p.y + m[2][2]*p.z + m[2][3]*1.0f;
    return m[3][0]*p.x + m[3][1]*p.y + m[3][2]*p.z + m[3][3]*1.0f;
}

/**
 * Función para inicializar la matriz m para que no tenga ninguna 
 * transformación aplicada
 */
void mat4_identity(mat4 m) {
    memset(m, 0, sizeof(mat4));
    m[0][0] = 1.0f;
    m[1][1] = 1.0f;
    m[2][2] = 1.0f;
    m[3][3] = 1.0f;
}

/**
 * Función que combina dos transformaciones en una sola matriz: dest = m1 * m2.
 * Se usa un temporal para evitar que los valores se pisen si dest == m1 o m2.
 */
void mat4_mul(mat4 dest, const mat4 m1, const mat4 m2) {
    mat4 tmp;
    memset(tmp, 0, sizeof(mat4));
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < 4; k++) {
                tmp[i][j] += m1[i][k] * m2[k][j];
            }
        }
    }
    memcpy(dest, tmp, sizeof(mat4));
}

/**
 * Función para trasladar el objeto v.x, v.y, v.z unidades en cada eje.
 * Se usa para alejar el cubo de la cámara (Z negativo) y centrarlo en pantalla.
 */
void mat4_translate(mat4 m, vec3 v) {
    mat4 t;
    mat4_identity(t);
    t[0][3] = v.x; // Desplazamiento en X
    t[1][3] = v.y; // Desplazamiento en Y
    t[2][3] = v.z; // Desplazamiento en Z
    mat4_mul(m, t, m); // Acumulamos la traslación sobre m (t * m)
}

/**
 * Función para rotar el objeto `angle` radianes alrededor del eje v.
 * 1. Normalizamos el eje para que tenga longitud 1
 * 2. Construimos la matriz de rotación
 * 3. Acumulamos sobre m
 */
void mat4_rotate(mat4 m, vec3 v, float angle) {
    // Normalizamos el eje de rotación
    float len = sqrtf(v.x*v.x + v.y*v.y + v.z*v.z);
    if (len == 0.0f) return;
    float x = v.x / len;
    float y = v.y / len;
    float z = v.z / len;

    // Términos del ángulo de rotación
    float c = cosf(angle);
    float s = sinf(angle);
    float t = 1.0f - c;

    // Matriz de rotación para el eje y el ángulo dados
    mat4 r;
    r[0][0] = t*x*x + c;    r[0][1] = t*x*y - s*z;  r[0][2] = t*x*z + s*y;  r[0][3] = 0.0f;
    r[1][0] = t*x*y + s*z;  r[1][1] = t*y*y + c;    r[1][2] = t*y*z - s*x;  r[1][3] = 0.0f;
    r[2][0] = t*x*z - s*y;  r[2][1] = t*y*z + s*x;  r[2][2] = t*z*z + c;    r[2][3] = 0.0f;
    r[3][0] = 0.0f;         r[3][1] = 0.0f;         r[3][2] = 0.0f;         r[3][3] = 1.0f;

    mat4_mul(m, r, m); // Acumulamos la rotación sobre m (r * m)
}

/**
 * Función para cambiar el tamaño del objeto en cada eje.
 * OBS. Un valor > 1 agranda, entre 0 y 1 encoge, negativo voltea el eje.
 */
void mat4_scale(mat4 m, vec3 v) {
    mat4 s;
    mat4_identity(s);
    s[0][0] = v.x; // Escalamiento en X
    s[1][1] = v.y; // Escalamiento en Y
    s[2][2] = v.z; // Escalamiento en Z
    mat4_mul(m, s, m); // Acumulamos el escalamiento sobre m (s * m)
}

/**
 * Función para crear una matriz de proyección ortogonal.
 * OBS. Los objetos se ven igual sin importar su distancia.
 */
void mat4_ortho(mat4 m, float left, float right, float bottom, float top, float near, float far) {
    memset(m, 0, sizeof(mat4)); // Limpiamos la matriz
    m[0][0] =  2.0f / (right - left); // Comprimimos el ancho del mundo para que quepa en pantalla
    m[1][1] =  2.0f / (top - bottom); // Comprimimos la altura del mundo para que quepa en pantalla
    m[2][2] = -2.0f / (far - near);   // Comprimimos la profundidad (negativo porque Z apunta hacia atrás)

    m[0][3] = -(right + left) / (right - left); // Centramos el mundo en X si no está en el origen
    m[1][3] = -(top + bottom) / (top - bottom); // Centramos el mundo en Y si no está en el origen
    m[2][3] = -(far + near)   / (far - near);   // Centramos el mundo en Z si no está en el origen
    m[3][3] =  1.0f;
}

/**
 * Función para crear una matriz de proyección perspectiva.
 * OBS. Los objetos lejanos se ven más pequeños.
 */
void mat4_perspective(mat4 m, float fovy, float aspect, float znear, float zfar) {
    memset(m, 0, sizeof(mat4)); // Limpiamos la matriz

    /**
     * OBS. fovy grande = se ven más cosas pero pequeñas
     * OBS. fovy pequeño = zoom
     */
    float f = 1.0f / tanf(fovy * 0.5f);

    m[0][0] = f / aspect; // Evitamos que el cubo se vea aplastado o estirado según el tamaño de la ventana
    m[1][1] = f; // Controlamos qué tan grande se ve el objeto verticalmente
    m[2][2] = (zfar + znear) / (znear - zfar); // Mapeamos la profundidad al rango del z-buffer [0,1]

    /**
     * OBS. znear mapea a 0
     * OBS. zfar mapea a 1
     */
    m[2][3] = (2.0f * zfar * znear) / (znear - zfar);

    m[3][2] = -1.0f;  // Copiamos Z a W para que al dividir entre W los objetos lejanos se vean más pequeños
}