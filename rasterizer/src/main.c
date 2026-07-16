#include <libosw/osw.h>
#include <string.h>
#include <trx.h>
#include <gs.h>
#include <model.h>
#include <stdlib.h>   // para rand() y srand()
#include <time.h>     // para time() (semilla del random)

int main() {
    /**
     * Inicializamos el sistema gráfico
     * gs_Init reemplaza OSW_Init + gs_Viewport + gs_SetClearColor
     */
    u32 err = gs_Init("Visualizador de Modelos 3D - Fernanda", 640, 480);
    if (err != OSW_OK) return err;

    srand((unsigned int)time(NULL));   // semilla del random con la hora actual

    /**
     * Cargamos el modelo desde su archivo TDM
     */
    Model model;
    err = model_load_tdm(&model, "bob_esponja_v2.tdm");

    if (err != 0) {
        printf("Error al cargar el modelo: %u\n", err);
        return err;
    }
    printf("Modelo cargado: %u vertices, %u triangulos\n",
        model.vert_count, model.index_count / 3);

    /**
     * Cargamos las texturas del modelo desde su archivo BMP
     */
    Texture tex;
    err = texture_load_bmp(&tex, "bob_esponja_tex.bmp");

    if (err != 0) {
        printf("Error al cargar la textura: %u\n", err);
        return err;
    }
    printf("Textura cargada: %ux%u\n", tex.width, tex.height);

    gs_texture = tex;

    // Habilitamos el polling del teclado para poder detectar eventos de las teclas
    OSW_KeyboardSetPolling(1);
    OSW_MouseSetPolling(1); // NUEVO: habilitamos el polling del mouse para detectar eventos de movimiento y clicks
    OSWKeyEvent kev;
    OSWMouse mouse; // NUEVO: aquí leeremos el estado del mouse en cada frame

    float angle = 0.0f; // Ángulo de rotación del modelo
    float aspect = (float)FRAMEBUFFER_W / (float)FRAMEBUFFER_H; // Aspect ratio para las proyecciones

    // Ángulos de rotación controlados por el usuario
    float rot_y = 0.0f;            // rotación sobre Y (flechas izquierda/derecha)
    float rot_x = 0.0f;            // rotación sobre X (flechas arriba/abajo)
    float rot_speed = 0.05f;       // cuánto gira por tecla presionada
    u32 auto_rotate = 0;           // rotación automática arranca apagada; se prende con la tecla 'R'

    mat4_perspective(gs_projection, 1.0472f /* 60° */, aspect, 0.1f, 100.0f);

    // Configuramos la cámara para el cálculo de iluminación (necesaria para el componente especular)
    gs_camera_pos = (vec3){0.0f, 0.1f, 1.0f};  // coincide con el view: alejada en +Z

    // LUZ 0: principal, viene de arriba-frente, blanca
    gs_light0.dir = (vec3){-0.3f, -0.5f, -0.8f};  // dirección en la que VIAJA la luz
    vec3_normalize(&gs_light0.dir);
    gs_light0.color = (vec3){1.0f, 1.0f, 1.0f};
    gs_light0.intensity = 1.2f;
    gs_light0.enabled = 1;

    // LUZ 1: secundaria, viene de la derecha y abajo con un tinte cálido
    // Empieza ENCENDIDA por defecto, se apaga/prende con la tecla L
    gs_light1.dir = (vec3){0.7f, 0.3f, -0.5f};   // dirección en la que VIAJA
    vec3_normalize(&gs_light1.dir);
    gs_light1.color = (vec3){1.0f, 0.7f, 0.4f};  // anaranjada cálida, contrasta con la blanca
    gs_light1.intensity = 0.8f;
    gs_light1.enabled = 1;                       // empieza prendida

    // Sensibilidad: qué tanto rota el modelo por cada píxel que se mueve el mouse
    float mouse_sensitivity = 0.01f;

    // Distancia de la cámara al modelo: negativo = frente al modelo
    float camera_z = -1.5f;
    float zoom_speed = 0.15f;

    while (1) {
        OSW_Poll();
        
        // Revisamos los eventos del teclado
        while (OSW_KeyboardGetEvent(&kev)) {
            // Si se presionan las teclas 'A' o  'D', giramos el modelo sobre Y
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_A) {
                rot_y -= rot_speed;
            }
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_D) {
                rot_y += rot_speed;
            }

            // Si se presionan las teclas 'W' o 'S', acercamos o alejamos la cámara (zoom in/out)
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_W) {
                camera_z += zoom_speed;
                if (camera_z > -0.3f) camera_z = -0.3f;
            }
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_S) {
                camera_z -= zoom_speed;
                if (camera_z < -8.0f) camera_z = -8.0f;
            }

            // Si se presiona la tecla 'R', alternamos la rotación automática del modelo
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_R) {
                auto_rotate = !auto_rotate;
                printf("Rotacion automatica: %s\n", auto_rotate ? "ON" : "OFF");
            }

            // Si se presiona la tecla 'T', reseteamos la rotación manual a 0
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_T) {
                rot_x = 0.0f;
                rot_y = 0.0f;
                printf("Rotacion reseteada\n");
            }

            // Si se presiona la tecla 'L', alternamos la luz secundaria
            if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_L) {
                gs_light1.enabled = !gs_light1.enabled;
                printf("Luz 1: %s\n", gs_light1.enabled ? "ENCENDIDA" : "APAGADA");
            }

            // Si se presiona la tecla 'ESC', cerramos la ventana
			if (kev.type == OSW_KEYEV_TYPE_PRESSED && kev.keycode == OSW_KEY_ESC) {
				printf("Cerrando la ventana..... Nos vemos :)\n");
				OSW_Exit(OSW_EXIT);
                model_free(&model); // Liberamos la memoria del modelo antes de salir
                texture_free(&tex);  // Liberamos la memoria de la textura antes de salir
				return 0;
			}
        }

        // Leemos el estado actual del mouse
        OSW_MouseGetState(&mouse);

        // Si el botón izquierdo está presionado (bit 0 del campo btn),
        // usamos el desplazamiento del mouse para rotar el modelo
        if (mouse.btn & OSW_MOUSE_BTN0) {
            // dx: movimiento horizontal del mouse → rota sobre Y (gira de frente/perfil/espalda)
            rot_y += mouse.dx * mouse_sensitivity;

            // dy: movimiento vertical → rota sobre X (inclina como asentir)
            rot_x += mouse.dy * mouse_sensitivity;
        }

        /**
         * Si la rotación automática está activa, seguimos sumando al ángulo Y
         * OBS. Solo corre si NO se está arrastrando el modelo con el mouse
         */
        if (auto_rotate && !(mouse.btn & OSW_MOUSE_BTN0)) {
            rot_y += 0.0015f;
        }

        /**
         * PICKING con clic derecho: cambiar color de la parte clicada
         * Detectamos el "borde" del clic (el momento exacto que se presiona, no si se mantiene)
         */
        static u32 prev_btn1 = 0;   // estado del botón derecho en el frame anterior
        u32 curr_btn1 = (mouse.btn & OSW_MOUSE_BTN1) ? 1 : 0;

        if (curr_btn1 && !prev_btn1) {
            s32 fb_x = mouse.x;
            s32 fb_y = mouse.y;

            if (fb_x >= 0 && fb_x < FRAMEBUFFER_W && fb_y >= 0 && fb_y < FRAMEBUFFER_H) {
                // Leemos del id_buffer qué parte ocupa ese pixel
                u32 picked_id = gs_id_buffer[fb_y * FRAMEBUFFER_W + fb_x];

                if (picked_id < NUM_PARTS) {
                    // Generamos un color aleatorio en rango [0.3, 1.0] para que no sea muy oscuro
                    gs_part_tints[picked_id] = (vec3){
                        0.3f + (rand() % 700) / 1000.0f,
                        0.3f + (rand() % 700) / 1000.0f,
                        0.3f + (rand() % 700) / 1000.0f
                    };
                }
            }
        }
        prev_btn1 = curr_btn1;

        // MODELO: enderezamos 90° sobre X, luego giramos sobre Y (rot_y)
        // y finalmente inclinamos sobre X (rot_x) con el mouse vertical
        mat4_identity(gs_model);
        mat4_rotate(gs_model, (vec3){1.0f, 0.0f, 0.0f}, 1.5708f);   // enderezado fijo
        mat4_rotate(gs_model, (vec3){0.0f, 1.0f, 0.0f}, rot_y);     // rotación sobre Y (mouse horizontal)
        mat4_rotate(gs_model, (vec3){1.0f, 0.0f, 0.0f}, rot_x);     // rotación sobre X (mouse vertical)

        // VISTA: camera_z controla el zoom
        mat4_identity(gs_view);
        mat4_translate(gs_view, (vec3){0.0f, -0.3f, camera_z});

        // Dibujamos el modelo usando sus índices
        gs_Clear();
        gs_DrawElems(
            GS_TYPE_TRIANGLES,
            model.verts,
            model.vert_count,
            model.indices,
            model.index_count
        );
        gs_DrawBuffer();
    }

    return 0;
}