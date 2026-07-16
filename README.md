# Rasterizador 3D por Software / Software 3D Rasterizer

🇲🇽 [Español](#español) | 🇺🇸 [English](#english)

---

## Español

Motor de renderizado **por software** escrito en C que carga un modelo 3D exportado desde Blender y lo dibuja sin utilizar el pipeline de la GPU para rasterizar. Todo el pipeline gráfico — transformaciones, rasterización, shading e iluminación — está implementado desde cero.

### Pipeline gráfico

| Etapa | Implementación |
|---|---|
| Transformación de vértices | Modelo → Vista → Proyección, división de perspectiva, mapeo a viewport |
| Vertex shader | Transformación a espacio de mundo, muestreo de textura por UV, tinte por parte |
| Iluminación | Modelo de Phong completo (ambiental + difusa + especular), 2 luces direccionales independientes |
| Rasterización | Scanline con Bresenham, buffers de borde izquierdo/derecho, interpolación de Z y color |
| Z-buffer | Test de profundidad por píxel |
| Fragment shader | Clamp final de color a [0,1] |
| Backface culling | Descarte por área con signo en pantalla |
| Blending | Mezcla 30/70 con el fondo (transparencia simple, opcional) |

### Estructura del proyecto

```
├── rasterizer/
│   ├── src/
│   │   ├── main.c          # Punto de entrada, cámara, luces, loop principal, input
│   │   ├── gs.c            # Motor gráfico: framebuffer, z-buffer, id-buffer, rasterización
│   │   ├── trx.c           # Álgebra lineal: vectores, matrices 4×4, proyección
│   │   ├── model.c         # Carga de modelos en formato .tdm
│   │   └── texture.c       # Carga y muestreo de texturas BMP (24 bits)
│   ├── include/
│   │   ├── gs.h
│   │   ├── trx.h
│   │   ├── model.h
│   │   └── texture.h
│   ├── lib/                # Aquí va Osw.lib (ver Dependencias)
│   ├── tdm_exporter_v2.py  # Plugin de Blender para exportar al formato .tdm
│   └── build.bat           # Script de compilación (MSVC)
└── README.md
```

### Dependencias

Este proyecto requiere **libosw**, una librería externa para manejo de ventana e input. No está incluida en este repositorio.

1. Obtener libosw del proveedor original
2. Copiar `Osw.lib` a la carpeta `rasterizer/lib/`
3. Copiar los headers de libosw a `rasterizer/include/libosw/`

### Formato binario .tdm

Formato propio (little-endian) para almacenar mallas 3D:

- **Cabecera:** magic `"TDMD"`, versión, número de vértices, número de índices
- **Por vértice (40 bytes):** posición (3 floats), normal (3 floats), UV (2 floats), color (u32), part_id (u32)
- **Índices:** lista de triángulos

El exportador de Blender (`tdm_exporter_v2.py`) recorre la malla, deduplica vértices por posición+UV, obtiene el `part_id` desde los vertex groups y escribe el archivo binario.

### Uso con tu propio modelo

El repositorio no incluye modelos ni texturas por motivos de licencia. Para usar el rasterizador con tu propio modelo:

1. Abrir tu modelo en Blender
2. Asignar vertex groups para definir las partes seleccionables
3. Ejecutar `tdm_exporter_v2.py` para exportar al formato `.tdm`
4. Colocar el archivo `.tdm` y la textura `.bmp` (24 bits, sin compresión) en la carpeta `rasterizer/`
5. Ajustar los nombres de archivo en `main.c` y compilar

### Controles

| Tecla / Acción | Función |
|---|---|
| `A` / `D` | Rotar modelo (eje Y) |
| `W` / `S` | Zoom (acercar/alejar) |
| `R` | Alternar rotación automática |
| `T` | Resetear rotación |
| `L` | Encender/apagar luz secundaria |
| `ESC` | Cerrar |
| Click izquierdo + arrastre | Rotar modelo libremente |
| Click derecho | Picking: seleccionar y repintar partes del modelo |

### Compilación

Requiere MSVC (Visual Studio). Ajustar `VS_PATH` en `build.bat` y ejecutar:

```bat
cd rasterizer
build.bat
```

Enlaza: `User32.lib`, `Gdi32.lib`, `Opengl32.lib`, `Xinput9_1_0.lib`, `Osw.lib`

### Tecnologías

- **Lenguaje:** C
- **Álgebra lineal:** Implementación propia (vectores, matrices 4×4, rotación de Rodrigues)
- **Exportador:** Python (plugin de Blender)
- **Ventana/Input:** libosw (librería externa)

---

## English

**Software rendering** engine written in C that loads a 3D model exported from Blender and draws it without using the GPU rasterization pipeline. The entire graphics pipeline — transformations, rasterization, shading, and lighting — is implemented from scratch.

### Graphics pipeline

| Stage | Implementation |
|---|---|
| Vertex transformation | Model → View → Projection, perspective divide, viewport mapping |
| Vertex shader | World-space transformation, UV texture sampling, per-part tinting |
| Lighting | Full Phong model (ambient + diffuse + specular), 2 independent directional lights |
| Rasterization | Scanline with Bresenham, left/right edge buffers, Z and color interpolation |
| Z-buffer | Per-pixel depth test |
| Fragment shader | Final color clamp to [0,1] |
| Backface culling | Signed screen-area rejection |
| Blending | 30/70 mix with background (optional simple transparency) |

### Project structure

```
├── rasterizer/
│   ├── src/
│   │   ├── main.c          # Entry point, camera, lights, main loop, input
│   │   ├── gs.c            # Graphics engine: framebuffer, z-buffer, id-buffer, rasterization
│   │   ├── trx.c           # Linear algebra: vectors, 4×4 matrices, projection
│   │   ├── model.c         # .tdm model loading
│   │   └── texture.c       # BMP texture loading and sampling (24-bit)
│   ├── include/
│   │   ├── gs.h
│   │   ├── trx.h
│   │   ├── model.h
│   │   └── texture.h
│   ├── lib/                # Place Osw.lib here (see Dependencies)
│   ├── tdm_exporter_v2.py  # Blender plugin for .tdm export
│   └── build.bat           # Build script (MSVC)
└── README.md
```

### Dependencies

This project requires **libosw**, an external library for window management and input handling. It is not included in this repository.

1. Obtain libosw from the original provider
2. Copy `Osw.lib` to the `rasterizer/lib/` folder
3. Copy the libosw headers to `rasterizer/include/libosw/`

### Binary format .tdm

Custom format (little-endian) for storing 3D meshes:

- **Header:** magic `"TDMD"`, version, vertex count, index count
- **Per vertex (40 bytes):** position (3 floats), normal (3 floats), UV (2 floats), color (u32), part_id (u32)
- **Indices:** triangle list

The Blender exporter (`tdm_exporter_v2.py`) traverses the mesh, deduplicates vertices by position+UV, retrieves `part_id` from Blender's vertex groups, and writes the binary file.

### Using your own model

The repository does not include models or textures due to licensing. To use the rasterizer with your own model:

1. Open your model in Blender
2. Assign vertex groups to define selectable parts
3. Run `tdm_exporter_v2.py` to export to `.tdm` format
4. Place the `.tdm` file and `.bmp` texture (24-bit, uncompressed) in the `rasterizer/` folder
5. Update the file names in `main.c` and build

### Controls

| Key / Action | Function |
|---|---|
| `A` / `D` | Rotate model (Y axis) |
| `W` / `S` | Zoom (in/out) |
| `R` | Toggle auto-rotation |
| `T` | Reset rotation |
| `L` | Toggle secondary light |
| `ESC` | Close |
| Left click + drag | Freely rotate model |
| Right click | Picking: select and repaint model parts |

### Building

Requires MSVC (Visual Studio). Adjust `VS_PATH` in `build.bat` and run:

```bat
cd rasterizer
build.bat
```

Links: `User32.lib`, `Gdi32.lib`, `Opengl32.lib`, `Xinput9_1_0.lib`, `Osw.lib`

### Tech stack

- **Language:** C
- **Linear algebra:** Custom implementation (vectors, 4×4 matrices, Rodrigues rotation)
- **Exporter:** Python (Blender plugin)
- **Window/Input:** libosw (external library)