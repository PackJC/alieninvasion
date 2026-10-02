# Blender wrapper around gebsfish's tools/render_p3d.py for this mod's item pictures. Same arguments:
#
#   blender --background --python tools/render_items.py -- --manifest tools/render_manifest.json \
#       --src data --texroot data --out <renders_dir>
#
# Debinarized vanilla backpacks carry geometry that isn't drawn in-game: attachment proxies (backpack slot,
# chemlight, radio) and, on the worn models, a helper triangle with no texture or material. render_p3d
# imports without named selections, so the add-on can't recognise the proxies, and either kind of face takes
# the first material slot, where the manifest's texture override lands instead of on the bag. This imports
# with selections (proxy_action='CLEAR' then drops the proxies), deletes faces whose material has no texture,
# and removes the material slots left unused.
import importlib.util
import os

import bmesh
import bpy

RENDER_P3D = os.environ.get("GEBSFISH_RENDER_P3D") or os.path.normpath(
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "gebsfish", "tools", "render_p3d.py"))

spec = importlib.util.spec_from_file_location("render_p3d", RENDER_P3D)
render_p3d = importlib.util.module_from_spec(spec)
spec.loader.exec_module(render_p3d)


def texture_path(material):
    props = getattr(material, "a3ob_properties_material", None) if material else None
    return getattr(props, "texture_path", "") if props else ""


def import_model(path):
    bpy.ops.a3ob.import_p3d(
        filepath=path,
        first_lod_only=True,
        enclose=False,
        groupby='NONE',
        proxy_action='CLEAR',
        additional_data_allowed=True,
        additional_data={'UV', 'MATERIALS', 'SELECTIONS'},
        validate_meshes=True,
        absolute_paths=False,
    )
    meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    for obj in meshes:
        untextured = {i for i, slot in enumerate(obj.material_slots) if not texture_path(slot.material)}
        if untextured and len(untextured) < len(obj.material_slots):
            bm = bmesh.new()
            bm.from_mesh(obj.data)
            helpers = [f for f in bm.faces if f.material_index in untextured]
            if helpers:
                bmesh.ops.delete(bm, geom=helpers, context='FACES')
                bm.to_mesh(obj.data)
                print("  removed %d untextured helper face(s)" % len(helpers))
            bm.free()

        used = {poly.material_index for poly in obj.data.polygons}
        for index in reversed(range(len(obj.material_slots))):
            if index not in used:
                obj.active_material_index = index
                with bpy.context.temp_override(object=obj, active_object=obj):
                    bpy.ops.object.material_slot_remove()
    return meshes


render_p3d.import_model = import_model
render_p3d.main()
