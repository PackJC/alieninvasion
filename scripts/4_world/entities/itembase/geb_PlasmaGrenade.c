// A plasma grenade's blue flash when it bursts (vanilla grenades flash yellow), lit by day too and lasting as
// long as the blast's halo.
class geb_PlasmaGrenadeLight extends ExplosiveLight
{
	void geb_PlasmaGrenadeLight()
	{
		SetVisibleDuringDaylight(true);
		SetRadiusTo(30);
		SetBrightnessTo(14);
		SetAmbientColor(0.3, 0.55, 1.0);
		SetDiffuseColor(0.3, 0.55, 1.0);
		SetLifetime(0.6);
		SetFadeOutTime(0.4);
	}
};

// The glow round an armed plasma grenade: a small, flickering blue light that travels with it.
class geb_PlasmaGrenadeGlow extends PointLightBase
{
	void geb_PlasmaGrenadeGlow()
	{
		SetVisibleDuringDaylight(true);
		SetRadiusTo(3);
		SetBrightnessTo(1.5);
		SetCastShadow(false);
		SetFlareVisible(false);
		SetAmbientColor(0.3, 0.55, 1.0);
		SetDiffuseColor(0.3, 0.55, 1.0);
		SetFlickerSpeed(8);
		SetFlickerAmplitude(0.4);
		FadeIn(0.1);
		SetFadeOutTime(0.1);
	}
};

// geb_PlasmaGrenade (data/config.cpp; model tools/plasmagrenade/build.py): an alien sticky grenade.
//  - Prime and disarm it like any grenade's unpin and pin (geb_ActionPlasmaGrenade.c, vanilla Grenade_Base rules):
//    the fuse starts when it leaves the hand primed and runs GEB_FUSE seconds.
//  - Thrown (or dropped) unpinned, the first thing it touches holds it (server, EOnContact): the ball presses
//    against the surface where it hit. Players, infected and animals carry it on their skin, on the bone of the
//    body part it hit (geb_FindBodySurface: their contact shape is a movement capsule, not the body); cars and boats
//    carry it on their body; anything else (ground, walls, trees, items) holds it where it is. It ignores its
//    thrower for the first half second, so it can't stick to the throwing hand.
//  - Whoever it is stuck to also takes the blast point-blank (ProcessDirectDamage), as in Halo.
//  - Armed (unpinned, in hand or out), its lights turn blue-white and blaze, plasma crackles round it and trails
//    behind it in flight (geb_plasmagrenade_armed), and a blue light flickers with it. It bursts in a blue blast
//    (CfgAmmo geb_PlasmaGrenade_Ammo particle geb_plasmagrenade_blast) with a blue flash.
// The stick is synced (holder's network id, bone and transform) so every client holds it in the same place.
class geb_PlasmaGrenade : Grenade_Base
{
	static const float GEB_FUSE = 3;
	static const float GEB_RADIUS = 0.032;			// the ball's radius: tools/plasmagrenade/build.py R
	static const float GEB_SINK = 0.004;			// how far the ball presses into what it sticks to (m)
	static const float GEB_SINK_BODY = 0.008;		// and into skin, fur and clothes
	static const float GEB_NORMAL_RAY_OFFSET = 0.03;	// spacing of the rays that find the slope of the skin (m)
	static const int GEB_THROWER_GRACE_MS = 500;	// it can't stick to its thrower this soon after the throw
	static const int GEB_RETRY_MS = 200;			// a client waiting for the holder to stream in looks again this often
	static const string GEB_LIGHTS_TEXTURE = "alieninvasion\\data\\plasmagrenade\\geb_plasmagrenade_lights_co.paa";
	static const string GEB_LIGHTS_MATERIAL = "alieninvasion\\data\\plasmagrenade\\geb_plasmagrenade_light.rvmat";
	static const string GEB_ARMED_TEXTURE = "alieninvasion\\data\\plasmagrenade\\geb_plasmagrenade_lights_armed_co.paa";
	static const string GEB_ARMED_MATERIAL = "alieninvasion\\data\\plasmagrenade\\geb_plasmagrenade_light_armed.rvmat";
	// Bones it can ride on players and infected (the nearest one to the hit); animals use whichever they have
	static const int GEB_BONE_COUNT = 21;
	static const string GEB_BONES[21] = {"Head", "Neck", "Spine3", "Spine2", "Spine1", "Spine", "Pelvis", "LeftShoulder", "LeftArm", "LeftForeArm", "LeftHand", "RightShoulder", "RightArm", "RightForeArm", "RightHand", "LeftUpLeg", "LeftLeg", "LeftFoot", "RightUpLeg", "RightLeg", "RightFoot"};

	protected vector m_geb_Centre;			// the ball's centre in model space (memory point "centre")
	protected bool m_geb_Thrown;			// server: left a hand unpinned, so the next touch sticks it
	protected Object m_geb_Thrower;
	protected int m_geb_ThrowTime;
	protected bool m_geb_StickQueued;
	protected bool m_geb_HasHolder;
	protected Object m_geb_Holder;			// what holds it; null when the world does

	// The stick, synced: the holder's network id (both 0 when the world holds it), the bone (-1: the holder's
	// origin) and the transform, local to that bone or holder (else in the world), as position and quaternion
	protected bool m_geb_Stuck;
	protected int m_geb_HolderLow;
	protected int m_geb_HolderHigh;
	protected int m_geb_Pivot = -1;
	protected float m_geb_PosX;
	protected float m_geb_PosY;
	protected float m_geb_PosZ;
	protected float m_geb_Q0;
	protected float m_geb_Q1;
	protected float m_geb_Q2;
	protected float m_geb_Q3;
	protected bool m_geb_StuckApplied;

	// Clients: the armed look
	protected bool m_geb_FxOn;
	protected ParticleSource m_geb_Fx;
	protected geb_PlasmaGrenadeGlow m_geb_Glow;

	void geb_PlasmaGrenade()
	{
		SetAmmoType("geb_PlasmaGrenade_Ammo");
		SetFuseDelay(GEB_FUSE);
		SetEventMask(EntityEvent.CONTACT);

		RegisterNetSyncVariableBool("m_geb_Stuck");
		RegisterNetSyncVariableInt("m_geb_HolderLow");
		RegisterNetSyncVariableInt("m_geb_HolderHigh");
		RegisterNetSyncVariableInt("m_geb_Pivot");
		RegisterNetSyncVariableFloat("m_geb_PosX");
		RegisterNetSyncVariableFloat("m_geb_PosY");
		RegisterNetSyncVariableFloat("m_geb_PosZ");
		RegisterNetSyncVariableFloat("m_geb_Q0");
		RegisterNetSyncVariableFloat("m_geb_Q1");
		RegisterNetSyncVariableFloat("m_geb_Q2");
		RegisterNetSyncVariableFloat("m_geb_Q3");
	}

	override void SetActions()
	{
		super.SetActions();

		RemoveAction(ActionUnpin);
		RemoveAction(ActionPin);
		AddAction(geb_ActionPrimePlasmaGrenade);
		AddAction(geb_ActionDisarmPlasmaGrenade);
	}

	override void EEInit()
	{
		super.EEInit();

		m_geb_Centre = GetMemoryPointPos("centre");
		if ( !GetGame().IsDedicatedServer() )
			geb_UpdateFx();
	}

	override void EEDelete(EntityAI parent)
	{
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(geb_StickServer);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(geb_ApplyStick);
		geb_StopFx();

		super.EEDelete(parent);
	}

	override protected void OnPin()
	{
		super.OnPin();

		m_geb_Thrown = false;
		if ( !GetGame().IsDedicatedServer() )
			geb_UpdateFx();
	}

	override protected void OnUnpin()
	{
		super.OnUnpin();

		if ( !GetGame().IsDedicatedServer() )
			geb_UpdateFx();
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if ( m_geb_Stuck && !m_geb_StuckApplied )
			geb_ApplyStick();
		geb_UpdateFx();
	}

	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);

		if ( oldLoc.GetType() == InventoryLocationType.HANDS && newLoc.GetType() == InventoryLocationType.GROUND && !IsPinned() )
		{
			m_geb_Thrown = true;
			m_geb_Thrower = oldLoc.GetParent();
			m_geb_ThrowTime = GetGame().GetTime();
		}

		// the armed look shows in hands and out in the world, not inside a bag
		if ( !GetGame().IsDedicatedServer() )
			geb_UpdateFx();
	}

	// ------------------------------------------------------------------------------------------------------------
	// Sticking
	// ------------------------------------------------------------------------------------------------------------

	override void EOnContact(IEntity other, Contact extra)
	{
		super.EOnContact(other, extra);

		if ( !GetGame().IsServer() || !m_geb_Thrown || m_geb_Stuck || m_geb_StickQueued || IsPinned() )
			return;
		if ( other && other == m_geb_Thrower && GetGame().GetTime() - m_geb_ThrowTime < GEB_THROWER_GRACE_MS )
			return;

		// Which way it was flying when it hit (the contact's other body is what it hit)
		vector travel = extra.VelocityBefore2;
		if ( extra.Physics1 == GetPhysics() )
			travel = extra.VelocityBefore1;

		Object holder = Object.Cast(other);
		if ( !geb_CanHold(holder) )
			holder = null;

		// On a body: the skin and the bone of the part it hit. Anywhere else: where it touched, the contact normal
		// turned to face back the way it came (a fast throw can be a little past the surface by the time the
		// contact comes, so the ball's own position can't tell the sides apart), or toward the ball if it was
		// only dropped.
		vector spot = extra.Position;
		vector normal = extra.Normal;
		float sink = GEB_SINK;
		bool onBody = false;
		int pivot = -1;
		if ( holder && geb_IsBody(holder) )
		{
			int component = -1;
			onBody = geb_FindBodySurface(holder, extra.Position, travel, spot, normal, component);
			if ( onBody )
			{
				sink = GEB_SINK_BODY;
				pivot = geb_ComponentBone(holder, component);
			}
			if ( pivot == -1 )
				pivot = geb_NearestBone(holder, spot);
		}
		if ( !onBody )
		{
			if ( travel.LengthSq() > 0.25 )
			{
				if ( vector.Dot(normal, travel) > 0 )
					normal = -normal;
			}
			else if ( vector.Dot(normal, ModelToWorld(m_geb_Centre) - spot) < 0 )
			{
				normal = -normal;
			}
		}
		normal.Normalize();

		// The ball pressed against the surface, turned about the normal the way it flew
		vector dir = geb_Flatten(GetDirection(), normal);
		if ( dir.LengthSq() < 0.01 )
			dir = geb_Flatten(vector.Forward, normal);
		if ( dir.LengthSq() < 0.01 )
			dir = geb_Flatten(vector.Aside, normal);
		dir.Normalize();
		vector mat[4];
		Math3D.DirectionAndUpMatrix(dir, normal, mat);
		vector centre = spot + normal * (GEB_RADIUS - sink);
		mat[3] = centre - mat[0] * m_geb_Centre[0] - mat[1] * m_geb_Centre[1] - mat[2] * m_geb_Centre[2];

		// Something that moves carries it: keep the transform relative to the bone or body it hit
		if ( holder )
		{
			vector parentMat[4];
			if ( pivot == -1 )
				holder.GetTransform(parentMat);
			else
				holder.GetBoneTransformWS(pivot, parentMat);
			Math3D.MatrixInvMultiply4(parentMat, mat, mat);
			Math3D.MatrixOrthogonalize4(mat);
		}

		m_geb_Holder = holder;
		m_geb_HasHolder = holder != null;
		m_geb_Pivot = pivot;
		geb_StoreTransform(mat);
		m_geb_StickQueued = true;
		// Physics can't be changed inside its own contact callback: stick on the next frame
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Call(geb_StickServer);
	}

	protected vector geb_Flatten(vector v, vector normal)
	{
		return v - normal * vector.Dot(v, normal);
	}

	// Players, infected, animals and vehicles carry the grenade with them; anything else holds it where it hit.
	protected bool geb_CanHold(Object holder)
	{
		if ( !holder || holder == this )
			return false;
		return geb_IsBody(holder) || holder.IsInherited(Transport);
	}

	protected bool geb_IsBody(Object holder)
	{
		return holder.IsMan() || holder.IsInherited(DayZCreature);
	}

	// Where on a creature's body the grenade lands. What it touched is the creature's movement capsule, which
	// doesn't follow the body (a cow's is far thinner than the cow, so the ball stuck inside it), so this finds the
	// body the way bullets and crossbow bolts do: a ray at its fire geometry, back along the throw through the hit,
	// else from just outside straight at the nearest bone. spot is on the skin and normal points out of it (from
	// two more rays a few centimetres to the sides; the ray's own result gives no surface normal); component is
	// the fire-geometry piece hit, or -1.
	protected bool geb_FindBodySurface(Object holder, vector hit, vector travel, out vector spot, out vector normal, out int component)
	{
		vector bonePos = holder.GetPosition() + Vector(0, 1, 0);
		int bone = geb_NearestBone(holder, hit);
		if ( bone != -1 )
			bonePos = holder.GetBonePositionWS(bone);

		array<vector> froms = new array<vector>;
		array<vector> tos = new array<vector>;
		if ( travel.LengthSq() > 0.25 )
		{
			vector along = travel;
			along.Normalize();
			froms.Insert(hit - along * 0.6);
			tos.Insert(hit + along * 1.2);
		}
		vector outward = hit - bonePos;
		if ( outward.LengthSq() > 0.0001 )
		{
			outward.Normalize();
			froms.Insert(hit + outward * 0.6);
			tos.Insert(bonePos);
		}

		for (int i = 0; i < froms.Count(); i++)
		{
			if ( !geb_RayToBody(holder, froms[i], tos[i], spot, component) )
				continue;

			vector ray = tos[i] - froms[i];
			ray.Normalize();
			normal = -ray;
			vector side = geb_Flatten(vector.Up, ray);
			if ( side.LengthSq() < 0.01 )
				side = geb_Flatten(vector.Aside, ray);
			side.Normalize();
			vector side2 = ray * side;
			vector spot1;
			vector spot2;
			int unused;
			float off = GEB_NORMAL_RAY_OFFSET;
			if ( geb_RayToBody(holder, froms[i] + side * off, tos[i] + side * off, spot1, unused) && geb_RayToBody(holder, froms[i] + side2 * off, tos[i] + side2 * off, spot2, unused) )
			{
				vector slope = (spot1 - spot) * (spot2 - spot);
				if ( slope.LengthSq() > 0.0000001 )
				{
					slope.Normalize();
					if ( vector.Dot(slope, ray) > 0 )
						slope = -slope;
					// a steep miss on one side ray (an arm's edge) can tip it a long way: keep it within ~70 degrees of facing the ray
					if ( vector.Dot(slope, -ray) > 0.35 )
						normal = slope;
				}
			}
			return true;
		}
		return false;
	}

	// The first point of holder's fire geometry (else its indirect fire geometry) on the segment. component is the
	// fire-geometry piece, or -1 when only the indirect fire geometry was hit (its pieces are numbered differently).
	protected bool geb_RayToBody(Object holder, vector from, vector to, out vector spot, out int component)
	{
		for (int attempt = 0; attempt < 2; attempt++)
		{
			RaycastRVParams rayParams = new RaycastRVParams(from, to, this);
			rayParams.flags = CollisionFlags.ONLYDYNAMIC;
			rayParams.sorted = true;
			rayParams.type = ObjIntersectFire;
			if ( attempt == 1 )
				rayParams.type = ObjIntersectIFire;
			array<ref RaycastRVResult> results = new array<ref RaycastRVResult>;
			if ( !DayZPhysics.RaycastRVProxy(rayParams, results) )
				continue;

			foreach (RaycastRVResult res : results)
			{
				if ( res.obj != holder && res.parent != holder )
					continue;
				spot = res.pos;
				component = -1;
				if ( attempt == 0 )
					component = res.component;
				return true;
			}
		}
		return false;
	}

	// A body's bone by name; -1 if it has none or isn't a body. Human and DayZCreature each declare
	// GetBoneIndexByName, Object doesn't.
	protected int geb_BoneIndex(Object holder, string boneName)
	{
		Human human = Human.Cast(holder);
		if ( human )
			return human.GetBoneIndexByName(boneName);

		DayZCreature creature = DayZCreature.Cast(holder);
		if ( creature )
			return creature.GetBoneIndexByName(boneName);

		return -1;
	}

	// The bone a fire-geometry piece belongs to, as vanilla finds it for a crossbow bolt (AddArrow); -1 if none.
	protected int geb_ComponentBone(Object holder, int component)
	{
		if ( component < 0 )
			return -1;
		TStringArray names = new TStringArray;
		holder.GetActionComponentNameList(component, names, LOD.NAME_FIRE);
		foreach (string compName : names)
		{
			int bone = geb_BoneIndex(holder, compName);
			if ( bone != -1 )
				return bone;
		}
		return -1;
	}

	protected int geb_NearestBone(Object holder, vector spot)
	{
		int best = -1;
		float bestDistSq = 1000000;
		for (int i = 0; i < GEB_BONE_COUNT; i++)
		{
			int bone = geb_BoneIndex(holder, GEB_BONES[i]);
			if ( bone < 0 )
				continue;

			float distSq = vector.DistanceSq(holder.GetBonePositionWS(bone), spot);
			if ( distSq < bestDistSq )
			{
				bestDistSq = distSq;
				best = bone;
			}
		}
		return best;
	}

	protected void geb_StickServer()
	{
		m_geb_StickQueued = false;
		if ( m_geb_Stuck || IsSetForDeletion() || IsDamageDestroyed() )
			return;
		// What it hit went away in the meantime: the next touch sticks it instead
		if ( m_geb_HasHolder && !m_geb_Holder )
			return;

		int low = 0;
		int high = 0;
		if ( m_geb_Holder )
			m_geb_Holder.GetNetworkID(low, high);
		m_geb_HolderLow = low;
		m_geb_HolderHigh = high;
		m_geb_Stuck = true;
		SetTakeable(false);
		geb_ApplyStick();
		SetSynchDirty();
	}

	// Server, and clients from the synced values: no more physics, then hold it where it stuck.
	protected void geb_ApplyStick()
	{
		if ( m_geb_StuckApplied || IsSetForDeletion() )
			return;

		Object holder = m_geb_Holder;
		if ( !holder && (m_geb_HolderLow != 0 || m_geb_HolderHigh != 0) )
		{
			holder = GetGame().GetObjectByNetworkId(m_geb_HolderLow, m_geb_HolderHigh);
			if ( !holder )
			{
				// not streamed in on this client yet
				GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(geb_ApplyStick, GEB_RETRY_MS, false);
				return;
			}
		}
		m_geb_StuckApplied = true;
		m_geb_Holder = holder;

		if ( dBodyIsSet(this) )
			dBodyDestroy(this);

		vector mat[4];
		geb_LoadTransform(mat);
		if ( !holder )
		{
			SetTransform(mat);
			return;
		}
		if ( GetParent() != holder )
		{
			SetTransform(mat);
			holder.AddChild(this, m_geb_Pivot);
		}
	}

	protected void geb_StoreTransform(vector mat[4])
	{
		vector rot[3];
		rot[0] = mat[0];
		rot[1] = mat[1];
		rot[2] = mat[2];
		float q[4];
		Math3D.MatrixToQuat(rot, q);
		m_geb_Q0 = q[0];
		m_geb_Q1 = q[1];
		m_geb_Q2 = q[2];
		m_geb_Q3 = q[3];
		vector pos = mat[3];
		m_geb_PosX = pos[0];
		m_geb_PosY = pos[1];
		m_geb_PosZ = pos[2];
	}

	protected void geb_LoadTransform(inout vector mat[4])
	{
		float q[4];
		q[0] = m_geb_Q0;
		q[1] = m_geb_Q1;
		q[2] = m_geb_Q2;
		q[3] = m_geb_Q3;
		vector rot[3];
		Math3D.QuatToMatrix(q, rot);
		mat[0] = rot[0];
		mat[1] = rot[1];
		mat[2] = rot[2];
		mat[3] = Vector(m_geb_PosX, m_geb_PosY, m_geb_PosZ);
	}

	// ------------------------------------------------------------------------------------------------------------
	// The burst
	// ------------------------------------------------------------------------------------------------------------

	override protected void InitiateExplosion()
	{
		if ( GetGame().IsServer() )
		{
			// Whoever wears it takes the blast point-blank
			EntityAI victim = EntityAI.Cast(m_geb_Holder);
			if ( victim && victim.IsAlive() )
				victim.ProcessDirectDamage(DamageType.EXPLOSION, this, "", "geb_PlasmaGrenade_Ammo", victim.WorldToModel(GetPosition()), 1.0);
			// Burst and get deleted as a loose item, not as part of whoever it was stuck to
			IEntity parent = GetParent();
			if ( parent )
				parent.RemoveChild(this, true);
		}

		super.InitiateExplosion();
	}

	override protected void CreateLight()
	{
		m_Light = ExplosiveLight.Cast(ScriptedLightBase.CreateLight(geb_PlasmaGrenadeLight, GetPosition()));
	}

	// ------------------------------------------------------------------------------------------------------------
	// The armed look (clients)
	// ------------------------------------------------------------------------------------------------------------

	protected void geb_UpdateFx()
	{
		bool armed = !IsPinned() && !IsDamageDestroyed() && !IsSetForDeletion();
		if ( armed && !m_geb_Stuck )
		{
			InventoryLocation il = new InventoryLocation();
			GetInventory().GetCurrentInventoryLocation(il);
			armed = il.GetType() == InventoryLocationType.GROUND || il.GetType() == InventoryLocationType.HANDS;
		}
		if ( armed == m_geb_FxOn )
			return;

		m_geb_FxOn = armed;
		if ( armed )
		{
			SetObjectTexture(0, GEB_ARMED_TEXTURE);
			SetObjectMaterial(0, GEB_ARMED_MATERIAL);
			m_geb_Fx = ParticleManager.GetInstance().PlayOnObject(ParticleList.GEB_PLASMAGRENADE_ARMED, this, m_geb_Centre);
			m_geb_Glow = geb_PlasmaGrenadeGlow.Cast(ScriptedLightBase.CreateLight(geb_PlasmaGrenadeGlow, GetPosition()));
			if ( m_geb_Glow )
				m_geb_Glow.AttachOnObject(this, m_geb_Centre);
		}
		else
		{
			SetObjectTexture(0, GEB_LIGHTS_TEXTURE);
			SetObjectMaterial(0, GEB_LIGHTS_MATERIAL);
			geb_StopFx();
		}
	}

	protected void geb_StopFx()
	{
		if ( m_geb_Fx )
		{
			m_geb_Fx.Stop();
			m_geb_Fx = null;
		}
		if ( m_geb_Glow )
		{
			m_geb_Glow.FadeOut();
			m_geb_Glow = null;
		}
	}
};
