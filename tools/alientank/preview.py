r"""Eevee render of the specimen tank as shipped (Blender, background), for checking it and for the docs picture.

    blender -b --factory-startup --python tools/alientank/preview.py -- out.png [VIEW] [--lod N] [--cache DIR]
        [--size W H] [--dark]

VIEW: front3q (default), front, side, back3q, top, console, inside. --fx scatters stand-ins for the particles
(bubble streams from the diffuser, green motes) where effects.py's emitters put them. --anim N renders N frames
into the folder `out` instead (frame_0000.png...): bubbles rising, motes drifting, the light breathing and the
camera swaying, all looping seamlessly over N / --fps seconds (default 15 fps; --zoom 1.2 moves the camera 20%
closer); join them with ffmpeg. --transparent drops the
floor and background, for the website's item picture (then tools/build_wiki_items.py crops it to WebP). Reads data/tank/geb_alientank.p3d and the
textures it names (PAA converted to PNG with DayZ Tools' ImageToPAA into --cache, default %TEMP%\alientank_png).
Approximates the game: metal through Principled BSDF with the _smdi gloss, the fluid alpha-blended with a little
emission, glow/screen/grille materials emissive, a green point light at the "light" memory point. --dark renders
a dim room so the glow reads; otherwise a grey studio.
"""
import os
import subprocess
import sys

import bpy
import numpy as np

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(HERE, "..", "alienrig"))
import mlod  # noqa: E402

DAYZ_TOOLS = os.environ.get("DAYZ_TOOLS", r"G:\SteamLibrary\steamapps\common\DayZ Tools")
IMAGE_TO_PAA = os.path.join(DAYZ_TOOLS, "Bin", "ImageToPAA", "ImageToPAA.exe")

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
out = argv[0]
view = argv[1] if len(argv) > 1 and not argv[1].startswith("--") else "front3q"


def opt(name, n=1, default=None):
    if name in argv:
        i = argv.index(name)
        return argv[i + 1:i + 1 + n] if n > 1 else argv[i + 1]
    return default


lod_index = int(opt("--lod", default="0"))
cache = opt("--cache", default=os.path.join(os.environ.get("TEMP", "."), "alientank_png"))
size = [int(x) for x in opt("--size", 2, ["1100", "1400"])]
dark = "--dark" in argv
fx = "--fx" in argv
anim = int(opt("--anim", default="0"))
zoom = float(opt("--zoom", default="1"))
fps = int(opt("--fps", default="15"))
transparent = "--transparent" in argv
os.makedirs(cache, exist_ok=True)


def png_for(tex):
    """Repo PNG copy of a texture path from the model (alieninvasion\\...), or None."""
    if not tex or not tex.lower().startswith("alieninvasion\\"):
        return None
    paa = os.path.join(REPO, tex.split("\\", 1)[1])
    if not os.path.exists(paa):
        return None
    png = os.path.join(cache, os.path.basename(paa)[:-4] + ".png")
    if not os.path.exists(png) or os.path.getmtime(png) < os.path.getmtime(paa):
        subprocess.run([IMAGE_TO_PAA, paa, png], check=True, capture_output=True)
    return png


def smdi_for(tex):
    for suffix in ("_co.paa", "_ca.paa"):
        if tex.endswith(suffix):
            return png_for(tex[:-len(suffix)] + "_smdi.paa")
    return None


def nohq_for(tex):
    for suffix in ("_co.paa", "_ca.paa"):
        if tex.endswith(suffix):
            return png_for(tex[:-len(suffix)] + "_nohq.paa")
    return None


FLAT = {"metal": (0.18, 0.2, 0.2), "trim": (0.45, 0.46, 0.47), "hazard": (0.8, 0.6, 0.05), "hose": (0.05, 0.05, 0.05),
        "grille": (0.1, 0.3, 0.12), "glow": (0.3, 1.0, 0.4), "screen": (0.1, 0.6, 0.3), "label": (0.6, 0.6, 0.6),
        "fluid": (0.3, 0.9, 0.35), "alien": (0.5, 0.55, 0.4)}


def kind(tex, rv):
    name = os.path.basename(rv or tex).lower()
    for k in FLAT:
        if k in name:
            return k
    return "alien" if "alien" in name else "metal"


def image_node(nt, path, non_color=False):
    n = nt.nodes.new("ShaderNodeTexImage")
    n.image = bpy.data.images.load(path, check_existing=True)
    if non_color:
        n.image.colorspace_settings.name = "Non-Color"
    return n


def make_material(tex, rv):
    k = kind(tex, rv)
    mat = bpy.data.materials.new(os.path.basename(rv or tex))
    mat.use_nodes = True
    nt = mat.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    png = png_for(tex)
    col = None
    if png:
        col = image_node(nt, png)
        nt.links.new(col.outputs["Color"], bsdf.inputs["Base Color"])
    else:
        bsdf.inputs["Base Color"].default_value = (*FLAT[k], 1)
    smdi = smdi_for(tex)
    if smdi:
        s = image_node(nt, smdi, True)
        sep = nt.nodes.new("ShaderNodeSeparateColor")
        nt.links.new(s.outputs["Color"], sep.inputs["Color"])
        inv = nt.nodes.new("ShaderNodeMath")
        inv.operation = "SUBTRACT"
        inv.inputs[0].default_value = 1.0
        nt.links.new(sep.outputs["Blue"], inv.inputs[1])
        nt.links.new(inv.outputs[0], bsdf.inputs["Roughness"])
        nt.links.new(sep.outputs["Green"], bsdf.inputs["Specular IOR Level"])
    else:
        bsdf.inputs["Roughness"].default_value = 0.45
    nohq = nohq_for(tex)
    if nohq:
        nimg = image_node(nt, nohq, True)
        # DayZ normal maps are Y- (DirectX): flip green for Blender
        sep = nt.nodes.new("ShaderNodeSeparateColor")
        comb = nt.nodes.new("ShaderNodeCombineColor")
        inv = nt.nodes.new("ShaderNodeMath")
        inv.operation = "SUBTRACT"
        inv.inputs[0].default_value = 1.0
        nt.links.new(nimg.outputs["Color"], sep.inputs["Color"])
        nt.links.new(sep.outputs["Red"], comb.inputs["Red"])
        nt.links.new(sep.outputs["Green"], inv.inputs[1])
        nt.links.new(inv.outputs[0], comb.inputs["Green"])
        nt.links.new(sep.outputs["Blue"], comb.inputs["Blue"])
        nm = nt.nodes.new("ShaderNodeNormalMap")
        nt.links.new(comb.outputs["Color"], nm.inputs["Color"])
        nt.links.new(nm.outputs["Normal"], bsdf.inputs["Normal"])
    if k in ("metal", "trim", "hazard"):
        bsdf.inputs["Metallic"].default_value = 0.55 if k != "hazard" else 0.2
    if k in ("glow", "screen", "grille"):
        strength = {"glow": 6.0, "screen": 3.0, "grille": 2.5}[k]
        if col:
            nt.links.new(col.outputs["Color"], bsdf.inputs["Emission Color"])
        else:
            bsdf.inputs["Emission Color"].default_value = (*FLAT[k], 1)
        bsdf.inputs["Emission Strength"].default_value = strength
    if k == "fluid":
        bsdf.inputs["Roughness"].default_value = 0.05
        bsdf.inputs["Specular IOR Level"].default_value = 0.8
        if col:
            nt.links.new(col.outputs["Color"], bsdf.inputs["Emission Color"])
            nt.links.new(col.outputs["Alpha"], bsdf.inputs["Alpha"])
        else:
            bsdf.inputs["Alpha"].default_value = 0.4
            bsdf.inputs["Emission Color"].default_value = (*FLAT[k], 1)
        bsdf.inputs["Emission Strength"].default_value = 0.35
        mat.blend_method = "BLEND"
        mat.use_backface_culling = True
        mat.show_transparent_back = True
    return mat


def build():
    m = mlod.read(os.path.join(REPO, "data", "tank", "geb_alientank.p3d"))
    vis = [L for L in m["lods"] if L["res"] < 100]
    L = vis[lod_index]
    uvs = mlod.uvset(L)
    me = bpy.data.meshes.new("tank")
    # DayZ's model space is left-handed: reading it as Blender's (x, z, y) is a mirror, which is also what turns
    # MLOD's inward winding into Blender's outward one. (Rotating it instead showed the mirror image of the game.)
    faces = [[c[0] for c in f["v"][:f["n"]]] for f in L["faces"]]
    me.from_pydata([to_blender(p) for p in L["pts"]], [], faces)
    uvl = me.uv_layers.new()
    loop = 0
    mats = {}
    for fi, (f, uv) in enumerate(zip(L["faces"], uvs)):
        key = (f["tex"], f["mat"])
        if key not in mats:
            mats[key] = len(mats)
            me.materials.append(make_material(*key))
        me.polygons[fi].material_index = mats[key]
        for (u, v) in uv:
            uvl.data[loop].uv = (u, 1 - v)
            loop += 1
    normals = []
    for f in L["faces"]:
        for c in f["v"][:f["n"]]:
            normals.append(to_blender(-L["norms"][c[1]]))
    # no validate(): it drops the fluid's inside faces, which share points with the outside ones
    me.update()
    me.normals_split_custom_set(normals)
    ob = bpy.data.objects.new("tank", me)
    bpy.context.scene.collection.objects.link(ob)
    mem = [x for x in m["lods"] if abs(x["res"] - 1e15) < 1e9][0]
    names = [n for a, n, d in mem["taggs"] if not n.startswith("#")]
    pts = {n: mem["pts"][i] for i, n in enumerate(names)}
    return pts


def to_blender(p):
    """DayZ model (x right, y up, z forward; left-handed) -> Blender (x, y forward, z up)."""
    return (float(p[0]), float(p[2]), float(p[1]))


def scene_setup(mem):
    sc = bpy.context.scene
    sc.render.engine = "BLENDER_EEVEE_NEXT"
    sc.render.resolution_x, sc.render.resolution_y = size
    sc.render.film_transparent = transparent
    sc.eevee.taa_render_samples = 64
    sc.view_settings.view_transform = "AgX"
    world = bpy.data.worlds.new("w")
    sc.world = world
    world.use_nodes = True
    bg = world.node_tree.nodes["Background"]
    bg.inputs["Color"].default_value = (0.012, 0.014, 0.016, 1) if dark else (0.32, 0.33, 0.35, 1)
    bg.inputs["Strength"].default_value = 1.0
    # floor
    bpy.ops.mesh.primitive_plane_add(size=12, location=(0, 0, 0))
    floor = bpy.context.active_object
    floor.hide_render = transparent
    fm = bpy.data.materials.new("floor")
    fm.use_nodes = True
    fm.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = \
        (0.03, 0.03, 0.035, 1) if dark else (0.22, 0.22, 0.23, 1)
    fm.node_tree.nodes["Principled BSDF"].inputs["Roughness"].default_value = 0.35
    floor.data.materials.append(fm)

    def light(kind_, loc, energy, color=(1, 1, 1), size_=1.0, rot=None):
        ld = bpy.data.lights.new(kind_, kind_)
        ld.energy = energy
        ld.color = color
        if kind_ == "AREA":
            ld.size = size_
        lo = bpy.data.objects.new(kind_, ld)
        lo.location = loc
        if rot:
            lo.rotation_euler = rot
        elif kind_ == "AREA":                                   # face the tank
            from mathutils import Vector
            lo.rotation_euler = (Vector((0, 0, 1.5)) - Vector(loc)).to_track_quat("-Z", "Y").to_euler()
        sc.collection.objects.link(lo)
        return lo
    key_e, fill_e = (60, 25) if dark else (900, 350)
    if transparent:
        key_e, fill_e = 420, 150
    light("AREA", (3.5, -4.0, 4.5), key_e, (1.0, 0.97, 0.92), 3.0)      # key: front right, high
    light("AREA", (-4.0, -2.5, 2.5), fill_e, (0.8, 0.88, 1.0), 4.0)     # fill: front left
    light("AREA", (-0.5, 4.0, 4.0), fill_e, (1, 1, 1), 3.0)             # rim: behind
    glow = light("POINT", to_blender(mem["light"]), 160 if dark else 90, (0.35, 1.0, 0.45))

    # the model's front (-Z) is Blender's -Y
    cams = {
        "front3q": ((3.4, -5.1, 2.3), (0, 0, 1.75), 40),
        "front": ((0, -5.6, 1.7), (0, 0, 1.65), 40),
        "side": ((5.6, 0, 1.7), (0, 0, 1.65), 40),
        "back3q": ((-3.4, 4.4, 2.6), (0, 0, 1.65), 40),
        "top": ((0.01, -0.01, 7.5), (0, 0, 1.5), 40),
        "console": ((0.8, -2.6, 1.6), (0, -1.2, 0.85), 40),
        "inside": ((1.2, -2.0, 2.0), (0, 0, 1.8), 40),
    }
    loc, target, fov = cams[view]
    cd = bpy.data.cameras.new("cam")
    cd.lens_unit = "FOV"
    cd.angle = np.radians(fov)
    cam = bpy.data.objects.new("cam", cd)
    cam.location = loc
    sc.collection.objects.link(cam)
    tgt = bpy.data.objects.new("tgt", None)
    tgt.location = target
    sc.collection.objects.link(tgt)
    c = cam.constraints.new("TRACK_TO")
    c.target = tgt
    c.track_axis = "TRACK_NEGATIVE_Z"
    c.up_axis = "UP_Y"
    sc.camera = cam
    return {"glow": glow, "glow_energy": glow.data.energy, "cam": cam, "cam_loc": np.array(loc, float),
            "target": np.array(target, float)}


class Particles:
    """Stand-ins for the tank's particles at loop time t of T seconds, seamless over the loop.

    Bubbles rise in effects.py's six streams plus a fizz across the floor at about effects.py's steady 0.5 m/s,
    each a whole number of times per loop, wobbling and growing as they rise and shrinking away at the top; motes
    drift on small closed paths and twinkle."""

    def __init__(self, mem, T):
        rng = np.random.default_rng(3)
        self.T = T
        self.floor = np.array(mem["bubbles"], float)
        mid = np.array(mem["motes"], float)
        self.rise = 2.05
        laps = max(1, int(round(T * 0.5 / self.rise)))
        self.bub = []
        for deg in (20, 92, 164, 236, 308, None):
            if deg is None:
                c = [0.0, 0.0, 0.05]
            else:
                c = [0.30 * np.cos(np.radians(deg)), 0.0, 0.30 * np.sin(np.radians(deg))]
            for _ in range(26):
                self.bub.append(dict(c=np.array(c, float), ph=rng.uniform(), laps=laps, r=rng.uniform(0.004, 0.009),
                                     wob=rng.uniform(0.006, 0.016), wk=int(rng.integers(2, 5)),
                                     wp=rng.uniform(0, 6.28)))
        for _ in range(70):
            c = [rng.uniform(-0.38, 0.38), 0.0, rng.uniform(-0.38, 0.38)]
            self.bub.append(dict(c=np.array(c, float), ph=rng.uniform(), laps=max(1, laps - 1),
                                 r=rng.uniform(0.002, 0.004), wob=0.01, wk=int(rng.integers(1, 4)),
                                 wp=rng.uniform(0, 6.28)))
        self.mot = []
        for _ in range(45):
            c = mid + [rng.uniform(-0.34, 0.34), rng.uniform(-0.88, 0.88), rng.uniform(-0.34, 0.34)]
            self.mot.append(dict(c=c, a=rng.uniform(0.02, 0.06, 3), ph=rng.uniform(0, 6.28, 3),
                                 r=rng.uniform(0.003, 0.006), tw=rng.uniform(0, 6.28)))

    def at(self, t):
        u = t / self.T
        bub, mot = [], []
        for b in self.bub:
            f = (b["ph"] + u * b["laps"]) % 1.0
            y = 0.02 + f * self.rise
            w = b["wob"] * (0.4 + f)
            ang = 2 * np.pi * b["wk"] * u + b["wp"]
            off = np.array([w * np.sin(ang), 0.0, w * np.cos(ang)])
            fade = max(0.0, min(1.0, f / 0.04, (1 - f) / 0.06))
            bub.append((self.floor + b["c"] + off + [0, y, 0], b["r"] * (1 + 0.3 * f) * fade))
        for m in self.mot:
            off = m["a"] * np.sin(2 * np.pi * u + m["ph"])
            tw = 0.6 + 0.4 * np.sin(2 * np.pi * 2 * u + m["tw"])
            mot.append((m["c"] + off, m["r"] * tw))
        return bub, mot


def particle_objects():
    objs = {}
    for name, color, emit, alpha in (("bubbles", (0.85, 1.0, 0.9), 0.4, 0.55), ("motes", (0.4, 1.0, 0.5), 12.0, 1.0)):
        me = bpy.data.meshes.new(name)
        ob = bpy.data.objects.new(name, me)
        mat = bpy.data.materials.new(name)
        mat.use_nodes = True
        b = mat.node_tree.nodes["Principled BSDF"]
        b.inputs["Base Color"].default_value = (*color, 1)
        b.inputs["Emission Color"].default_value = (*color, 1)
        b.inputs["Emission Strength"].default_value = emit
        b.inputs["Alpha"].default_value = alpha
        b.inputs["Roughness"].default_value = 0.1
        mat.blend_method = "BLEND"
        me.materials.append(mat)
        bpy.context.scene.collection.objects.link(ob)
        objs[name] = ob
    return objs


def place_particles(objs, items_by_name):
    import bmesh
    for name, items in items_by_name.items():
        bm = bmesh.new()
        for pos, r in items:
            if r <= 1e-4 or np.hypot(pos[0], pos[2]) > 0.58:
                continue
            m = bmesh.ops.create_icosphere(bm, subdivisions=1, radius=r)
            bmesh.ops.translate(bm, verts=m["verts"], vec=to_blender(pos))
        bm.to_mesh(objs[name].data)
        bm.free()
        objs[name].data.update()


def glow_compositor():
    """Soft bloom round the glowing parts, roughly how the game's HDR glow reads."""
    sc = bpy.context.scene
    sc.use_nodes = True
    nt = sc.node_tree
    for n in list(nt.nodes):
        nt.nodes.remove(n)
    rl = nt.nodes.new("CompositorNodeRLayers")
    comp = nt.nodes.new("CompositorNodeComposite")
    try:
        g = nt.nodes.new("CompositorNodeGlare")
        g.glare_type = "FOG_GLOW"
        g.quality = "HIGH"
        g.threshold = 0.6
        g.size = 8
        g.mix = -0.55
        nt.links.new(rl.outputs["Image"], g.inputs["Image"])
        nt.links.new(g.outputs["Image"], comp.inputs["Image"])
    except Exception as e:                                   # API differences between Blender versions
        print("no glare:", e)
        nt.links.new(rl.outputs["Image"], comp.inputs["Image"])


for o in list(bpy.data.objects):
    bpy.data.objects.remove(o)
mem = build()
rig = scene_setup(mem)
sc = bpy.context.scene
if anim:
    T = anim / float(fps)
    os.makedirs(out, exist_ok=True)
    glow_compositor()
    sc.eevee.taa_render_samples = 32
    parts = Particles(mem, T)
    objs = particle_objects()
    base = (rig["cam_loc"] - rig["target"]) / zoom              # --zoom moves the camera in
    for i in range(anim):
        t = i / float(fps)
        u = t / T
        bub, mot = parts.at(t)
        place_particles(objs, {"bubbles": bub, "motes": mot})
        # the light breathes twice a loop; the camera sways +-10 degrees round the tank once a loop
        rig["glow"].data.energy = rig["glow_energy"] * (1 + 0.12 * np.sin(2 * np.pi * 2 * u))
        a = np.radians(10) * np.sin(2 * np.pi * u)
        ca, sa = np.cos(a), np.sin(a)
        rig["cam"].location = rig["target"] + [ca * base[0] - sa * base[1], sa * base[0] + ca * base[1], base[2]]
        sc.render.filepath = os.path.join(out, "frame_%04d.png" % i)
        bpy.ops.render.render(write_still=True)
        print("frame", i + 1, "/", anim)
    print("wrote", anim, "frames in", out)
else:
    if fx:
        bub, mot = Particles(mem, 6.0).at(0.0)
        place_particles(particle_objects(), {"bubbles": bub, "motes": mot})
    sc.render.filepath = out
    bpy.ops.render.render(write_still=True)
    print("wrote", out)
