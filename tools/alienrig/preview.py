"""Orthographic Workbench render of MLOD visual LODs side by side (Blender, background).

    blender -b --factory-startup --python tools/alienrig/preview.py -- out.png VIEW model.p3d[~][:lod][@offset] ...

VIEW is front, back, side or top, optionally zoomed: front:x,y,z,size (centre in model coordinates, ortho size
in metres). Faces are coloured by their strongest bone; set PREVIEW_TEXTURE to a PNG (ImageToPAA the .paa) to
show that texture through the face UVs instead. A trailing ~ draws the model as wireframe, for overlays.
Models sit 1.5 m apart (along X, or along Z in side view) unless @offset is given; :lod picks a visual LOD
(0 = first).
"""
import colorsys
import hashlib
import os
import sys

import bpy
import numpy as np
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import mlod  # noqa: E402
import rig  # noqa: E402

argv = sys.argv[sys.argv.index("--") + 1:]
out, view, specs = argv[0], argv[1], argv[2:]
ZOOM = None
if ":" in view:
    view, z = view.split(":")
    ZOOM = [float(v) for v in z.split(",")]
TEXTURE = os.environ.get("PREVIEW_TEXTURE")

bpy.ops.wm.read_factory_settings(use_empty=True)
scene = bpy.context.scene
mats = {}


def mat(name):
    if name not in mats:
        h = int(hashlib.md5(name.encode()).hexdigest()[:6], 16) / 0xFFFFFF
        m = bpy.data.materials.new(name)
        m.diffuse_color = colorsys.hsv_to_rgb(h, 0.65, 0.95) + (1,)
        mats[name] = m
    return mats[name]


def add(path, lodi, offset):
    L = [l for l in mlod.read(path)["lods"] if l["res"] < 1000][lodi]
    P = L["pts"]
    W = rig.bone_weights(L)
    names = list(W)
    dom = np.vstack([W[n] for n in names]).argmax(0) if names else np.zeros(len(P), int)
    if view == "side":
        verts = [(p[0], -(p[2] - offset), p[1]) for p in P]
    else:
        verts = [(p[0] + offset, -p[2], p[1]) for p in P]
    keep = rig.face_mask_without_proxies(L)
    kept = [fi for fi in range(len(L["faces"])) if keep[fi]]
    faces, fm = [], []
    for fi in kept:
        f = L["faces"][fi]
        idx = [f["v"][k][0] for k in range(f["n"])]
        faces.append(idx[::-1])                      # MLOD winding -> Blender outward
        fm.append(names[np.bincount(dom[idx]).argmax()] if names else "none")
    used = sorted({i for f in faces for i in f})
    remap = {o: n for n, o in enumerate(used)}
    me = bpy.data.meshes.new(os.path.basename(path))
    me.from_pydata([verts[i] for i in used], [], [[remap[i] for i in f] for f in faces])
    ob = bpy.data.objects.new(me.name, me)
    scene.collection.objects.link(ob)
    if TEXTURE:
        uvl = me.uv_layers.new(name="uv")
        for poly, fi in zip(me.polygons, kept):
            f = L["faces"][fi]
            uvs = [(f["v"][k][2], 1 - f["v"][k][3]) for k in range(f["n"])][::-1]
            for j, li in enumerate(poly.loop_indices):
                uvl.data[li].uv = uvs[j]
        m = bpy.data.materials.new("tex")
        m.use_nodes = True
        node = m.node_tree.nodes.new("ShaderNodeTexImage")
        node.image = bpy.data.images.load(TEXTURE)
        m.node_tree.links.new(node.outputs["Color"], m.node_tree.nodes["Principled BSDF"].inputs["Base Color"])
        me.materials.append(m)
    else:
        order = sorted(set(fm))
        for n in order:
            me.materials.append(mat(n))
        for poly, n in zip(me.polygons, fm):
            poly.material_index = order.index(n)
    return ob


obs = []
x = 0.0
for s in specs:
    offset = x
    if "@" in s:
        s, d = s.split("@")
        offset = float(d)
    lodi = 0
    if ":" in s[2:]:
        s, l = s.rsplit(":", 1)
        lodi = int(l)
    wire = s.endswith("~")
    ob = add(s.rstrip("~"), lodi, offset)
    if wire:
        ob.modifiers.new("w", "WIREFRAME").thickness = 0.0025
        ob.data.materials.clear()
        mk = bpy.data.materials.new("wire")
        mk.diffuse_color = (0.05, 0.05, 0.05, 1)
        ob.data.materials.append(mk)
    obs.append(ob)
    x += 1.5

co_all = [v.co for o in obs for v in o.data.vertices]
lo = Vector([min(c[i] for c in co_all) for i in range(3)])
hi = Vector([max(c[i] for c in co_all) for i in range(3)])
c = (lo + hi) / 2
cam = bpy.data.cameras.new("cam")
cam.type = "ORTHO"
co = bpy.data.objects.new("cam", cam)
scene.collection.objects.link(co)
scene.camera = co
size = max(hi.x - lo.x, hi.z - lo.z) * 1.08
if view == "front":      # models face -Z (Blender +Y): camera on +Y looking back at them
    co.location, co.rotation_euler = (c.x, c.y + 10, c.z), (np.pi / 2, 0, np.pi)
elif view == "back":
    co.location, co.rotation_euler = (c.x, c.y - 10, c.z), (np.pi / 2, 0, 0)
elif view == "side":     # from the model's left (+X)
    co.location, co.rotation_euler = (c.x + 10, c.y, c.z), (np.pi / 2, 0, np.pi / 2)
    size = max(hi.y - lo.y, hi.z - lo.z) * 1.08
elif view == "top":
    co.location, co.rotation_euler = (c.x, c.y, c.z + 10), (0, 0, 0)
    size = max(hi.x - lo.x, hi.y - lo.y) * 1.08
cam.ortho_scale = size
if ZOOM:
    zx, zy, zz, zs = ZOOM
    if view in ("front", "back"):
        co.location = (zx, co.location.y, zy)
    elif view == "side":
        co.location = (co.location.x, -zz, zy)
    elif view == "top":
        co.location = (zx, -zz, co.location.z)
    cam.ortho_scale = zs
scene.render.engine = "BLENDER_WORKBENCH"
scene.display.shading.light = "STUDIO"
scene.display.shading.color_type = "TEXTURE" if TEXTURE else "MATERIAL"
scene.display.shading.show_backface_culling = False
scene.render.resolution_x = 1400
scene.render.resolution_y = 1000
scene.render.filepath = out
bpy.ops.render.render(write_still=True)
print("wrote", out)
