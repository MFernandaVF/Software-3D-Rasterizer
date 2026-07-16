import bpy
import bmesh
import struct


def vec_to_rgb(vec):
    r = int(vec[0] * 255)
    g = int(vec[1] * 255)
    b = int(vec[2] * 255)
    return int((b << 16) | (g << 8) | r)


# Lista de partes en el orden de IDs. Los nombres deben coincidir EXACTAMENTE
# con los grupos de vértices que creaste en Blender.
PART_NAMES = [
    "cabeza",          # 0
    "ojo_blanco_izq",  # 1
    "ojo_color_izq",   # 2
    "ojo_blanco_der",  # 3
    "ojo_color_der",   # 4
    "boca",            # 5
    "lengua",          # 6
    "diente_izq",      # 7
    "diente_der",      # 8
    "brazo_izq",       # 9
    "brazo_der",       # 10
    "ropa",            # 11
    "pierna_izq",      # 12
    "pierna_der",      # 13
    "calcetin_izq",    # 14
    "calcetin_der",    # 15
    "zapato_izq",      # 16
    "zapato_der",      # 17
]


def get_part_id(obj, vert_index):
    """Devuelve el ID de la parte a la que pertenece un vértice, 0xFFFFFFFF si ninguna."""
    vert = obj.data.vertices[vert_index]
    for vg in vert.groups:
        group_name = obj.vertex_groups[vg.group].name
        if group_name in PART_NAMES:
            return PART_NAMES.index(group_name)
    return 0xFFFFFFFF


def write_tdm(context, filepath, use_some_setting):
    obj = bpy.context.selected_objects[0]
    mesh = obj.data
    bpy.ops.mesh.customdata_custom_splitnormals_add()
    uv_layer = mesh.uv_layers.active.data
    norm_loop = mesh.loops

    if not mesh.vertex_colors.active:
        mesh.vertex_colors.new()

    vertex_colors = mesh.vertex_colors.active.data

    face_list = []
    vert_list = []
    vert_dict = {}
    v_count = 0
    for poly in mesh.polygons:
        for loop_index in poly.loop_indices:
            vind = mesh.loops[loop_index].vertex_index
            key = str(mesh.vertices[vind]) + str(uv_layer[loop_index].uv)
            color = vec_to_rgb(vertex_colors[loop_index].color)
            if key not in vert_dict:
                vert_dict[key] = v_count
                part_id = get_part_id(obj, vind)
                vert_list.append((mesh.vertices[vind].co,
                                  norm_loop[loop_index].normal,
                                  uv_layer[loop_index].uv,
                                  color,
                                  part_id))
                v_count += 1
            face_list.append(vert_dict[key])

    out = open(filepath, 'wb')
    out.write(bytes('TDMD', 'utf-8'))
    out.write(struct.pack('<I', 2))    # version 2: ahora con part_id
    out.write(struct.pack('<I', len(vert_list)))
    out.write(struct.pack('<I', len(face_list)))
    for v in vert_list:
        out.write(struct.pack('<f', v[0].x))
        out.write(struct.pack('<f', v[0].z))
        out.write(struct.pack('<f', -v[0].y))
        out.write(struct.pack('<f', v[1].x))
        out.write(struct.pack('<f', v[1].z))
        out.write(struct.pack('<f', -v[1].y))
        out.write(struct.pack('<f', v[2].x))
        out.write(struct.pack('<f', v[2].y))
        out.write(struct.pack('<I', v[3]))
        out.write(struct.pack('<I', v[4]))  # nuevo: part_id

    for face in face_list:
        out.write(struct.pack('<I', face))

    # Resumen útil
    print("=== Resumen de partes exportadas ===")
    counts = {}
    for v in vert_list:
        pid = v[4]
        counts[pid] = counts.get(pid, 0) + 1
    for pid in sorted(counts.keys()):
        if pid == 0xFFFFFFFF:
            name = "(sin asignar)"
        else:
            name = PART_NAMES[pid] if pid < len(PART_NAMES) else f"(ID {pid})"
        print(f"  ID {pid}: {name} -> {counts[pid]} vertices")
    print(f"Total vertices: {len(vert_list)}, total indices: {len(face_list)}")

    out.close()
    return {'FINISHED'}


from bpy_extras.io_utils import ExportHelper
from bpy.props import StringProperty
from bpy.types import Operator


class ExportSomeData(Operator, ExportHelper):
    bl_idname = "export_tdm.some_data"
    bl_label = "Export TDM Mesh (v2 con partes)"
    filename_ext = ".tdm"
    filter_glob: StringProperty(default="*.tdm", options={'HIDDEN'}, maxlen=255)

    def execute(self, context):
        return write_tdm(context, self.filepath, 0)


def menu_func_export(self, context):
    self.layout.operator(ExportSomeData.bl_idname, text="TDM con partes (.tdm)")


def register():
    bpy.utils.register_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)


def unregister():
    bpy.utils.unregister_class(ExportSomeData)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)


if __name__ == "__main__":
    register()
    bpy.ops.export_tdm.some_data('INVOKE_DEFAULT')