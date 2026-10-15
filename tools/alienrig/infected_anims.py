r"""Blender scene that plays DayZ's own infected animations on p3d models side by side (Blender, background).

    blender -b --factory-startup --python tools/alienrig/infected_anims.py -- --out <scene.blend>
        [--game <DayZ Addons folder>] [--xob <z_hermit_m.xob>] [--anims <folder of .anm>]
        [--model <p3d>[=<texture png>] ...] [--clips "*walkf*,*attack*"] [--render <dir> --render-clips a,b]

Defaults: the infected skeleton (characters_zombies.pbo: z_hermit_m.xob) and every infected animation
(anims_anm_infected.pbo) read straight from the installed game, so the scene matches the current DayZ version:
DayZ, else DayZServer, else DayZ Exp under Steam's common folder (--game picks another Addons folder; --xob and
--anims read extracted files instead, e.g. from P:\DZ). Two models: the
rebuilt alien (data/aliens/geb_GreenAlien.p3d) and the debinarized vanilla zombie in tools/debinned, textured
from tools/debinned/textures/newgreenalien_ca.png and patient_skinny_m_co.png when those exist (make them with
DayZ Tools' ImageToPAA <file>.paa <file>.png). Keep the .blend in tools/debinned: it holds Bohemia's
animations and zombie mesh, so it must never be committed.

In the scene: one armature per model, 1.2 m apart. The first armature ("Driver") carries the action; the
others copy it bone by bone, so pick a clip in the Dope Sheet's Action Editor on the Driver and every model
plays it. Every infected clip is loaded as an action (fake user), named after its .anm. Clips the game layers
over another one (light hits and look-at blends: no pelvis track) play over the first frame of an idle and
are marked "+layer", or "+add" when they store offsets. Every action keys every bone, so switching clips never
leaves a bone where the previous clip put it. Clips play in place: root motion (EntityPosition) isn't
applied. The 5 ANIMSET5 files (crawl turns, one aggro turn) aren't read.

DayZ animations are authored in the engine's model space, which is the p3d's turned 180 degrees about up,
so the root gets a 180-degree yaw to line them up with the p3d meshes (both face -Z in the scene's own space).
"""
import argparse
import fnmatch
import glob
import math
import os
import sys

import bpy
import numpy as np
from mathutils import Matrix, Quaternion

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, HERE)
import enfusion as en  # noqa: E402
import mlod  # noqa: E402
import pbo  # noqa: E402
import rig  # noqa: E402

STEAM_COMMON = r"C:\Program Files (x86)\Steam\steamapps\common"
GAME_ADDONS = [os.path.join(STEAM_COMMON, "DayZ", "Addons"), os.path.join(STEAM_COMMON, "DayZServer", "addons"),
               os.path.join(STEAM_COMMON, "DayZ Exp", "Addons")]

YAW180 = np.array([0.0, 1.0, 0.0, 0.0])        # quaternion x, y, z, w: 180 degrees about up (y)


def qinv(q):
    return np.array([-q[0], -q[1], -q[2], q[3]])


def mat(t, q):
    m = Quaternion((q[3], q[0], q[1], q[2])).to_matrix().to_4x4()
    m.translation = t
    return m


# ---------------------------------------------------------------------------------------------------------
# scene pieces
# ---------------------------------------------------------------------------------------------------------

def make_armature(name, bones, offset):
    arm = bpy.data.armatures.new(name)
    arm.display_type = "STICK"
    ob = bpy.data.objects.new(name, arm)
    bpy.context.scene.collection.objects.link(ob)
    ob.rotation_euler = (math.pi / 2, 0, 0)     # model space (y up) -> Blender (z up)
    ob.location = (offset, 0, 0)
    ob.show_in_front = True
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.mode_set(mode="EDIT")
    rest = en.world(bones)
    ebs = []
    for b in bones:
        eb = arm.edit_bones.new(b.name.lower())
        eb.head = (0, 0, 0)
        eb.tail = (0, 0.04, 0)
        eb.matrix = mat(*rest[b.index])
        eb.length = 0.04
        ebs.append(eb)
    for b in bones:
        if b.parent != -1:
            ebs[b.index].parent = ebs[b.parent]
    bpy.ops.object.mode_set(mode="OBJECT")
    for pb in ob.pose.bones:
        pb.rotation_mode = "QUATERNION"
    # the edit-bone setter must reproduce the xob rest frames exactly, or the poses below are off
    worst = max(max(abs(x) for row in (ob.data.bones[b.name.lower()].matrix_local - mat(*rest[b.index])) for x in row)
                for b in bones)
    assert worst < 1e-4, "bone rest frames drifted by %g" % worst
    return ob


def load_texture(path):
    if not path or not os.path.exists(path):
        return None
    img = bpy.data.images.load(path, check_existing=True)
    m = bpy.data.materials.new(os.path.basename(path))
    m.use_nodes = True
    node = m.node_tree.nodes.new("ShaderNodeTexImage")
    node.image = img
    m.node_tree.links.new(node.outputs["Color"], m.node_tree.nodes["Principled BSDF"].inputs["Base Color"])
    return m


def make_mesh(path, texture, arm_ob):
    L = [l for l in mlod.read(path)["lods"] if l["res"] < 1000][0]
    keep = rig.face_mask_without_proxies(L)
    faces = [L["faces"][i] for i in range(len(L["faces"])) if keep[i]]
    used = sorted({f["v"][k][0] for f in faces for k in range(f["n"])})
    remap = {o: n for n, o in enumerate(used)}
    me = bpy.data.meshes.new(os.path.splitext(os.path.basename(path))[0])
    me.from_pydata([tuple(L["pts"][i]) for i in used], [],
                   [[remap[f["v"][k][0]] for k in range(f["n"])][::-1] for f in faces])   # MLOD winding -> Blender
    uvl = me.uv_layers.new(name="uv")
    for poly, f in zip(me.polygons, faces):
        uvs = [(f["v"][k][2], 1 - f["v"][k][3]) for k in range(f["n"])][::-1]
        for j, li in enumerate(poly.loop_indices):
            uvl.data[li].uv = uvs[j]
    for p in me.polygons:
        p.use_smooth = True
    ob = bpy.data.objects.new(me.name, me)
    bpy.context.scene.collection.objects.link(ob)
    ob.parent = arm_ob
    mat_ = load_texture(texture)
    if mat_:
        me.materials.append(mat_)
    idx = np.array(used)
    for name, w in rig.bone_weights(L).items():
        if name not in arm_ob.data.bones or not w[idx].any():
            continue
        vg = ob.vertex_groups.new(name=name)
        for nv, ov in enumerate(idx):
            if w[ov] > 0:
                vg.add([nv], float(w[ov]), "REPLACE")
    mod = ob.modifiers.new("Armature", "ARMATURE")
    mod.object = arm_ob
    return ob


def follow(ob, driver):
    for pb in ob.pose.bones:
        c = pb.constraints.new("COPY_TRANSFORMS")
        c.target = driver
        c.subtarget = pb.name
        c.owner_space = "LOCAL"
        c.target_space = "LOCAL"


# ---------------------------------------------------------------------------------------------------------
# animations -> actions
# ---------------------------------------------------------------------------------------------------------

def bone_basis(b, t, q):
    """Pose-bone basis (location, quaternion x y z w) for an animated local (t, q) of bone b."""
    qi = qinv(b.q)
    loc = en.qrot(qi, t - b.t)
    rot = en.qmul(qi, q)
    return loc, rot


def add_curves(action, group, path, frames, values):
    for i in range(values.shape[1]):
        fc = action.fcurves.new(path, index=i, action_group=group)
        fc.keyframe_points.add(len(frames))
        co = np.empty(2 * len(frames))
        co[0::2] = frames
        co[1::2] = values[:, i]
        fc.keyframe_points.foreach_set("co", co)
        fc.keyframe_points.foreach_set("interpolation", [1] * len(frames))    # LINEAR
        fc.update()


def build_action(anim, bones, name, base):
    """Every bone gets keys, so switching clips never leaves a bone posed by the previous one. Bones (or
    channels) the clip doesn't key take `base` ({bone index: (t, q)}), or the rest pose if it has none."""
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    action.use_frame_range = True
    action.frame_start = 0
    action.frame_end = max(1, anim.frames - 1)
    keys = 0
    for b in bones:
        bn = b.name.lower()
        if b.parent == -1:                                  # Scene_Root: the 180-degree turn into p3d space
            add_curves(action, bn, 'pose.bones["%s"].rotation_quaternion' % bn, np.array([0.0]),
                       np.array([[YAW180[3], YAW180[0], YAW180[1], YAW180[2]]]))
            continue
        tr = anim.tracks.get(b.name)
        bpose = base.get(b.index)
        frames = np.union1d(tr.t_frames, tr.r_frames).astype(float) if tr is not None else np.array([])
        locs, rots = [], []
        if not len(frames):                                 # not in this clip: one constant key
            frames = np.array([0.0])
            locs, rots = zip(bone_basis(b, *(bpose or (b.t, b.q))))
            locs, rots = list(locs), list(rots)
        else:
            for f in frames:
                loc, rot = bone_basis(b, *en.keyed_local(anim, b, tr, f, bpose))
                if rots and rot @ rots[-1] < 0:             # keep neighbouring quaternions on one side
                    rot = -rot
                locs.append(loc)
                rots.append(rot)
        locs = np.array(locs)
        rots = np.array(rots)[:, [3, 0, 1, 2]]              # Blender: w, x, y, z
        moving = np.ptp(locs, axis=0).max() > 1e-5
        add_curves(action, bn, 'pose.bones["%s"].location' % bn, frames if moving else frames[:1],
                   locs if moving else locs[:1])
        moving = np.ptp(rots, axis=0).max() > 1e-5
        add_curves(action, bn, 'pose.bones["%s"].rotation_quaternion' % bn, frames if moving else frames[:1],
                   rots if moving else rots[:1])
        keys += len(frames)
    return action, keys


# ---------------------------------------------------------------------------------------------------------
# where the skeleton and clips come from
# ---------------------------------------------------------------------------------------------------------

def game_addons(a):
    if a.game:
        return a.game
    for d in GAME_ADDONS:
        if os.path.exists(os.path.join(d, "anims_anm_infected.pbo")):
            return d
    raise SystemExit("no DayZ install with anims_anm_infected.pbo; pass --game, or --xob and --anims")


def load_skeleton(a):
    if a.xob:
        return en.read_xob_skeleton(a.xob), a.xob
    src = os.path.join(game_addons(a), "characters_zombies.pbo")
    files = pbo.entries(src)
    key = [k for k in files if k.rsplit("/", 1)[-1] == "z_hermit_m.xob"][0]
    return en.read_xob_skeleton(key, pbo.read(src, files[key])), src + ":" + key


def list_clips(a):
    """[(name, folder, loader)] sorted by name; loader() -> Animation or None."""
    out = []
    if a.anims:
        for path in glob.glob(os.path.join(a.anims, "**", "*.anm"), recursive=True):
            out.append((os.path.splitext(os.path.basename(path))[0].lower(),
                        os.path.basename(os.path.dirname(path)).lower(), lambda p=path: en.read_anm(p)))
        return sorted(out, key=lambda c: (c[0], c[1])), a.anims
    src = os.path.join(game_addons(a), "anims_anm_infected.pbo")
    files = pbo.entries(src)
    for key, entry in files.items():
        if key.endswith(".anm"):
            out.append((os.path.splitext(os.path.basename(key))[0], os.path.basename(os.path.dirname(key)),
                        lambda k=key, e=entry: en.read_anm(k, pbo.read(src, e))))
    return sorted(out, key=lambda c: (c[0], c[1])), src


def main():
    argv = sys.argv[sys.argv.index("--") + 1:]
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--game")
    ap.add_argument("--xob")
    ap.add_argument("--anims")
    ap.add_argument("--model", action="append")
    ap.add_argument("--clips", default="*")
    ap.add_argument("--default", default="za_erc_walkf")
    ap.add_argument("--render")
    ap.add_argument("--render-clips", default="")
    ap.add_argument("--render-view", default="side")
    a = ap.parse_args(argv)
    textures = os.path.join(REPO, "tools", "debinned", "textures")
    models = a.model or [
        os.path.join(REPO, "data", "aliens", "geb_GreenAlien.p3d") + "=" + os.path.join(textures, "newgreenalien_ca.png"),
        os.path.join(REPO, "tools", "debinned", "_patient_skinny_m.p3d") + "=" + os.path.join(textures, "patient_skinny_m_co.png"),
    ]

    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.render.fps = 30
    bones, skel_src = load_skeleton(a)
    print("skeleton", skel_src, "-", len(bones), "bones")

    arms = []
    for i, spec in enumerate(models):
        path, _, tex = spec.partition("=")
        name = "Driver" if i == 0 else "Follow%d" % i
        arm = make_armature("%s_%s" % (name, os.path.splitext(os.path.basename(path))[0]), bones, 1.2 * i)
        make_mesh(path, tex, arm)
        if arms:
            follow(arm, arms[0])
        arms.append(arm)
        print("model", path)

    patterns = [p.strip() for p in a.clips.split(",") if p.strip()]
    clips, clip_src = list_clips(a)
    print("clips", clip_src, "-", len(clips), "files")

    def base_pose(clip):
        """Frame 0 of an idle, under the layer clips (light hits, look-at), like the game blends them."""
        found = [c for c in clips if c[0] == clip]
        anim = found[0][2]() if found else None
        return en.local_pose(anim, bones, 0) if anim else {}

    bases = {"erected": base_pose("zb_erc_idle01"), "crawl": base_pose("z_crwl_idle01")}
    actions = {}
    skipped = []
    total = 0
    for name, folder, load in clips:
        if not any(fnmatch.fnmatch(name, p) for p in patterns):
            continue
        anim = load()
        if anim is None:
            skipped.append(name)
            continue
        if name in actions:                                 # same file name in two folders (z_erc_idle_fight01)
            name = "%s (%s)" % (name, folder)
        en.mark_additive(anim, bones)
        base = {}
        if en.is_partial(anim):
            base = bases["crawl" if ("crwl" in name or "crawl" in name) else "erected"]
        label = name + (" +add" if anim.additive else (" +layer" if base else ""))
        action, keys = build_action(anim, bones, label, base)
        actions[name] = (action, anim)
        total += keys
    print("actions: %d (%d bone-frames); skipped (ANIMSET5): %s" % (len(actions), total, ", ".join(skipped)))

    first = actions.get(a.default) or next(iter(actions.values()))
    arms[0].animation_data_create()
    arms[0].animation_data.action = first[0]
    scene.frame_start, scene.frame_end = 0, max(1, first[1].frames - 1)

    note = bpy.data.texts.new("README")
    note.write(__doc__)

    if a.render:
        render(a, arms, actions, scene)
    out = os.path.abspath(a.out)
    os.makedirs(os.path.dirname(out), exist_ok=True)
    for img in bpy.data.images:                         # textures next to the .blend stay found if it moves with them
        if img.filepath and not img.filepath.startswith("//"):
            try:
                img.filepath = "//" + os.path.relpath(bpy.path.abspath(img.filepath), os.path.dirname(out)).replace("\\", "/")
            except ValueError:                          # another drive: keep the absolute path
                pass
    bpy.ops.wm.save_as_mainfile(filepath=out)
    print("saved", a.out)


def render(a, arms, actions, scene):
    """PNG frames of the listed clips (start, quarter, half, three quarters) for checking without Blender's UI."""
    os.makedirs(a.render, exist_ok=True)
    cam = bpy.data.objects.new("cam", bpy.data.cameras.new("cam"))
    scene.collection.objects.link(cam)
    scene.camera = cam
    cam.data.type = "ORTHO"
    span = 1.2 * (len(arms) - 1)
    cam.data.ortho_scale = max(2.6, span + 1.6)
    # models face Blender +Y; a camera turned by yaw about Z looks along (-sin yaw, cos yaw)
    yaw = math.pi if a.render_view == "front" else math.radians(125)     # side: front-left three-quarter
    cam.rotation_euler = (math.pi / 2, 0, yaw)
    cam.location = (span / 2 + 10 * math.sin(yaw), -10 * math.cos(yaw), 0.95)
    scene.render.engine = "BLENDER_WORKBENCH"
    scene.display.shading.light = "STUDIO"
    scene.display.shading.color_type = "TEXTURE"
    scene.render.resolution_x, scene.render.resolution_y = 900, 900
    for name in [c.strip() for c in a.render_clips.split(",") if c.strip()]:
        if name not in actions:
            print("no clip", name)
            continue
        action, anim = actions[name]
        arms[0].animation_data.action = action
        n = anim.frames
        for k, f in enumerate(sorted({0, n // 4, n // 2, 3 * n // 4})):
            scene.frame_set(f)
            scene.render.filepath = os.path.join(a.render, "%s_%d.png" % (name, k))
            bpy.ops.render.render(write_still=True)


main()
