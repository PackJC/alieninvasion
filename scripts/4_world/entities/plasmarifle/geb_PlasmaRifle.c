// The plasma shot's muzzle light, in place of vanilla's orange MuzzleFlashLight. It matches MuzzleFlashLight
// except for the colour: 0.04 s, radius 15, full brightness on the shot frame, then brightness and radius fade
// out over the second half. MuzzleFlashLight calls FadeIn() before SetBrightnessTo(1), which cancels the fade,
// so it doesn't fade in either. A real fade-in would dim a light that lasts two or three frames.
// The rifle sets the colour of the cell that was fired. This green is used when a cell has no colour set.
class geb_PlasmaFlashLight extends PointLightBase
{
	static float m_geb_Lifetime = 0.04;
	static vector m_geb_DefaultColour = "0.3 1 0.4";

	void geb_PlasmaFlashLight()
	{
		SetLifetime(m_geb_Lifetime);
		SetFadeOutTime(m_geb_Lifetime * 0.5);
		SetVisibleDuringDaylight(true);
		SetRadiusTo(15);
		SetBrightnessTo(1);
		SetCastShadow(false);
		SetFlareVisible(false);
		geb_SetColour(m_geb_DefaultColour);
	}

	// Diffuse and ambient alike, as vanilla's muzzle lights do
	void geb_SetColour(vector colour)
	{
		SetDiffuseColorV(colour);
		SetAmbientColorV(colour);
	}
};

// Must extend the same vanilla rifle as the config (geb_PlasmaRifle_Base: Ruger1022). The engine picks the
// hands animation graph from the config chain (Ruger1022_Base), so a FAL_Base script parent ran the FAL's
// bolt-lock state machine against Ruger animations: firing the last cell requested FIRE_LAST, which the
// Ruger graph can't play, and the weapon hung waiting for animation events that never came.
class geb_PlasmaRifle : Ruger1022_Base
{
	// Keep the recoil the rifle has always had.
	override RecoilBase SpawnRecoilObject()
	{
		return new FALRecoil(this);
	}

	// Debug and admin-tool spawns ("with ammo"): a loaded plasma cartridge of a random colour. Vanilla's Ruger
	// version tries to attach a Mag_Ruger1022_30Rnd, which this rifle refuses (a script error on every spawn).
	override void OnDebugSpawn()
	{
		SpawnAttachedMagazine(geb_RandomPlasmaCartridgeType());
	}

	// Tap R with the rifle in hand is vanilla's FirearmActionMechanicManipulate: it works the action and throws the
	// chambered round on the ground, one cell out of the gun. A live cell stays in: the action only runs when the
	// chamber is empty or fired out, where it chambers the next cell from the cartridge (or dry-cycles). Cartridges
	// themselves can't be split or emptied either (geb_PlasmaCartridge.c).
	override bool CanEjectBullet()
	{
		for (int i = 0; i < GetMuzzleCount(); i++)
		{
			if ( !IsChamberEmpty(i) && !IsChamberFiredOut(i) )
				return false;
		}
		return super.CanEjectBullet();
	}

	// The turning fluid is an item of its own in the hidden slot geb_PlasmaRifleFluid (tools/plasmarifle/fluid.py):
	// weapons don't run model.cfg's clock animations on their own model, so the vent tubes' and lime parts' fluid
	// turns on that attachment instead, the way vanilla's Magnum keeps its cylinder an item (SetGunParts). Every
	// rifle gets one on the server as it loads, rifles from before 2.0.0 and from storage too.
	override void AssembleGun()
	{
		super.AssembleGun();

		if ( !FindAttachmentBySlotName("geb_PlasmaRifleFluid") )
			GetInventory().CreateAttachment("geb_PlasmaRifleFluid");
	}

	// ---- The rear screen (model section "screen", hiddenSelections index 0; tools/plasmarifle/screen.py) ----
	// A client-side timer steps it through its frames while the rifle is in someone's hands near the camera
	// (first person included: the camera is then the holder's) or lies loose in the world near it. In an
	// inventory, on a back or far away it keeps its frame and only looks again every GEB_SCREEN_IDLE_MS. The
	// timer starts when the entity is created on the client and is removed when it is deleted (leaving the
	// network bubble too), so picking it up, dropping it or stowing it needs nothing else. Dedicated servers run
	// none of it. The frame and camera helpers are shared with the specimen tank's console (geb_ScreenFrames.c).
	static const string GEB_SCREEN_TEXTURE = "alieninvasion\\data\\weapons\\geb_plasmarifle_screen_%1_co.paa";
	static const int GEB_SCREEN_FRAMES = 16;		// screen.py FRAMES
	static const int GEB_SCREEN_STEP_MS = 125;		// screen.py FPS 8: a 2 s loop
	static const int GEB_SCREEN_IDLE_MS = 1000;
	static const float GEB_SCREEN_RANGE_SQ = 625.0;	// 25 m

	protected int m_geb_ScreenFrame;

	override void EEInit()
	{
		super.EEInit();

		if ( !GetGame().IsDedicatedServer() )
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(geb_ScreenStep, GEB_SCREEN_IDLE_MS, false);
	}

	override void EEDelete(EntityAI parent)
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(geb_ScreenStep);

		super.EEDelete(parent);
	}

	// In someone's hands, or not in any inventory at all; and near the camera
	protected bool geb_ScreenShown()
	{
		if ( GetHierarchyParent() )
		{
			PlayerBase holder = PlayerBase.Cast(GetHierarchyRootPlayer());
			if ( !holder || holder.GetItemInHands() != this )
				return false;
		}

		return geb_CameraWithin(this, GEB_SCREEN_RANGE_SQ);
	}

	protected void geb_ScreenStep()
	{
		if ( IsSetForDeletion() )
			return;

		int delayMs = GEB_SCREEN_IDLE_MS;
		if ( geb_ScreenShown() )
		{
			m_geb_ScreenFrame = geb_NextScreenFrame(this, GEB_SCREEN_TEXTURE, GEB_SCREEN_FRAMES, m_geb_ScreenFrame);
			delayMs = GEB_SCREEN_STEP_MS;
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(geb_ScreenStep, delayMs, false);
	}

	// Overheating effects get no ammo type. Vanilla's WeaponParticlesBase.CheckParticleOverride gives every
	// Particles entry the fired ammo's muzzleFlashParticle when the ammo has one, OnOverheating entries included.
	// With the ammo passed on, each cell's muzzle burst stood in for the overheating effect and played again on
	// every shot once the barrel was hot: 2.0.0's cloud of white dots. With "" each OnOverheating entry plays its
	// own overrideParticle (the blue-white arcs in data/config.cpp), as vanilla's decay ticks already do
	// (ItemBase.OnOverheatingDecay passes ""). The overheating count itself is unchanged.
	override void IncreaseOverheating(ItemBase weapon, string ammoType, ItemBase muzzle_owner, ItemBase suppressor, string config_to_search)
	{
		super.IncreaseOverheating(weapon, "", muzzle_owner, suppressor, config_to_search);
	}

	// The cell's own muzzle effect comes from config (Particles > OnFire > MuzzleFlash and the ammo's
	// muzzleFlashParticle). The plasma ball you see fly (CfgAmmo <cell> geb_laserParticle, in the cell's colour;
	// the original laser streak geb_plasma_shot until 2.0.0) is played here on top of it, every shot: config
	// can't play both, because vanilla gives every OnFire entry the ammo's muzzleFlashParticle.
	//
	// The ball is created loose at the muzzle, aimed by script, not on the rifle. Played on the rifle (as vanilla
	// plays muzzle effects) it took its line from the barrel a moment after the shot, once FALRecoil's first kick
	// (its first curve point is always to the right and up) had turned the gun, and flew off right of the round
	// and its tracer (Cole, 2026-10-24). The shooter's own ball aims at what the crosshair is on, as the round
	// does; other players' balls, and shots in freelook, follow the barrel line (Konec hlavne -> Usti hlavne).
	static const float GEB_BALL_AIM_RANGE = 300.0;		// m: the sights' longest zeroing (OpticsInfo)
	static const float GEB_BALL_MAX_OFF = 0.9;			// cos of the largest turn from the barrel (about 25 deg)

	protected vector geb_BallDirection(vector muzzle, vector barrel)
	{
		PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
		if ( !player || player != GetGame().GetPlayer() )
			return barrel;

		HumanInputController hic = player.GetInputController();
		if ( hic && hic.CameraIsFreeLook() )
			return barrel;

		vector from = GetGame().GetCurrentCameraPosition();
		vector target = from + GetGame().GetCurrentCameraDirection() * GEB_BALL_AIM_RANGE;
		vector hitPos;
		vector hitNormal;
		int hitComponent;
		set<Object> hits = new set<Object>;
		if ( DayZPhysics.RaycastRV(from, target, hitPos, hitNormal, hitComponent, hits, null, player, false, false, ObjIntersectFire) )
			target = hitPos;

		// A hit right in front of the muzzle (a wall, the rifle itself) or far off the barrel: use the barrel
		vector aim = vector.Direction(muzzle, target);
		if ( aim.Length() < 2 )
			return barrel;

		aim.Normalize();
		if ( vector.Dot(aim, barrel) < GEB_BALL_MAX_OFF )
			return barrel;

		return aim;
	}

	// The ball's source frame: the rifle's own, turned so its -x (the barrel axis, along which the ball's
	// emitters fire with vanilla's Angles 0 0 -90) points down dir. Handedness is kept from the rifle's frame.
	protected void geb_BallFrame(vector dir, vector pos, out vector frame[4])
	{
		vector rifle[4];
		GetTransform(rifle);

		vector x = dir * -1;
		vector y = rifle[1] - x * vector.Dot(rifle[1], x);
		y.Normalize();
		vector z = x * y;									// cross product
		if ( vector.Dot(rifle[0] * rifle[1], rifle[2]) < 0 )
			z = z * -1;

		frame[0] = x;
		frame[1] = y;
		frame[2] = z;
		frame[3] = pos;
	}

	protected void geb_PlayBall(string ammoType)
	{
		int laser = ParticleList.geb_plasma_shot;
		string laserName;
		GetGame().ConfigGetText("CfgAmmo " + ammoType + " geb_laserParticle", laserName);
		if ( laserName != "" && ParticleList.IsValidId(ParticleList.GetParticleIDByName(laserName)) )
			laser = ParticleList.GetParticleIDByName(laserName);

		vector muzzle = ModelToWorld(GetSelectionPositionLS("Usti hlavne"));
		vector barrel = vector.Direction(ModelToWorld(GetSelectionPositionLS("Konec hlavne")), muzzle);
		barrel.Normalize();

		// Created without playing, placed and turned, then started: its first emission already has the line
		ParticleSource ball = ParticleManager.GetInstance().CreateInWorld(laser, muzzle);
		if ( !ball )
			return;

		vector frame[4];
		vector dir = geb_BallDirection(muzzle, barrel);
		geb_BallFrame(dir, muzzle, frame);
		ball.SetTransform(frame);
		ball.PlayParticle();

		// A cell that shoots lightning names its bolt (CfgAmmo <cell> geb_boltParticle); its laser is only the puff
		string boltName;
		GetGame().ConfigGetText("CfgAmmo " + ammoType + " geb_boltParticle", boltName);
		if ( boltName == "" )
			return;

		int bolt = ParticleList.GetParticleIDByName(boltName);
		if ( !ParticleList.IsValidId(bolt) )
			return;

		int landing = -1;
		string landingName;
		GetGame().ConfigGetText("CfgAmmo " + ammoType + " geb_boltHitParticle", landingName);
		if ( landingName != "" && ParticleList.IsValidId(ParticleList.GetParticleIDByName(landingName)) )
			landing = ParticleList.GetParticleIDByName(landingName);

		geb_PlayLightning(ammoType, muzzle, dir, bolt, landing);
	}

	// The violet cell's lightning (tools/plasmashots/shots.py LIGHTNING): a jagged bolt from the muzzle along the
	// shot to what it hits, or GEB_BOLT_RANGE on. It strikes GEB_BOLT_STRIKES times, GEB_BOLT_GAP_MS apart, on a new
	// path each time, the first with a few forks, and flashes where it lands. Each segment is one bolt effect at the
	// segment's middle, turned onto it as the ball is and stretched to its length (the effect is made for 1 m).
	static const float GEB_BOLT_RANGE = 100.0;			// m
	static const float GEB_BOLT_STEP = 2.0;				// m: about this long per segment (longer past 48 m)
	static const int GEB_BOLT_MAX_SEGMENTS = 24;
	static const float GEB_BOLT_JAG = 0.7;				// m: how far a joint can stray off the straight line
	static const int GEB_BOLT_FORKS = 3;
	static const int GEB_BOLT_STRIKES = 3;
	static const int GEB_BOLT_GAP_MS = 70;

	protected void geb_PlayLightning(string ammoType, vector muzzle, vector dir, int bolt, int landing)
	{
		vector to = muzzle + dir * GEB_BOLT_RANGE;
		vector hitPos;
		vector hitNormal;
		int hitComponent;
		set<Object> hits = new set<Object>;
		bool landed = DayZPhysics.RaycastRV(muzzle + dir * 0.05, to, hitPos, hitNormal, hitComponent, hits, null, GetHierarchyRootPlayer(), false, false, ObjIntersectFire);
		if ( landed )
			to = hitPos;

		geb_Strike(muzzle, to, bolt, GEB_BOLT_FORKS);
		for ( int s = 1; s < GEB_BOLT_STRIKES; s++ )
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(geb_Strike, s * GEB_BOLT_GAP_MS, false, muzzle, to, bolt, 0);

		if ( !landed )
			return;

		if ( landing != -1 )
			ParticleManager.GetInstance().PlayInWorld(landing, to);

		// The landing lights up in the cell's colour too
		geb_PlasmaFlashLight light = geb_PlasmaFlashLight.Cast(ScriptedLightBase.CreateLight(geb_PlasmaFlashLight, to + hitNormal * 0.3));
		TFloatArray colour = new TFloatArray;
		GetGame().ConfigGetFloatArray("CfgAmmo " + ammoType + " geb_muzzleLightColor", colour);
		if ( light && colour.Count() == 3 )
			light.geb_SetColour(Vector(colour[0], colour[1], colour[2]));
	}

	// One strike: the straight line from `from` to `to` cut into segments whose joints stray to either side (none at
	// the ends, little near the muzzle, so it leaves the barrel cleanly), and `forks` short branches off random joints
	protected void geb_Strike(vector from, vector to, int bolt, int forks)
	{
		vector line = vector.Direction(from, to);
		float dist = line.Length();
		if ( dist < 1 )
			return;

		vector dir = line.Normalized();
		vector up = "0 1 0";
		vector across = "1 0 0";
		vector u = dir * up;								// cross products: two directions across the bolt
		if ( u.Length() < 0.1 )
			u = dir * across;
		u.Normalize();
		vector v = u * dir;

		int count = Math.Ceil(dist / GEB_BOLT_STEP);
		count = Math.Clamp(count, 2, GEB_BOLT_MAX_SEGMENTS);
		float jag = Math.Min(GEB_BOLT_JAG, dist * 0.05);
		array<vector> joints = new array<vector>;
		vector last = from;
		for ( int i = 1; i <= count; i++ )
		{
			float t = i;
			t = t / count;
			vector joint = from + line * t;
			if ( i < count )
			{
				float taper = Math.Min(1.0, i * 0.5);
				joint = joint + (u * Math.RandomFloat(-1, 1) + v * Math.RandomFloat(-1, 1)) * (jag * taper);
				joints.Insert(joint);
			}
			geb_BoltSegment(last, joint, bolt);
			last = joint;
		}

		for ( int k = 0; k < forks && joints.Count() > 2; k++ )
		{
			vector start = joints[Math.RandomInt(1, joints.Count())];
			vector side = u * Math.RandomFloat(-1, 1) + v * Math.RandomFloat(-1, 1);
			side.Normalize();
			vector forkDir = dir + side * Math.RandomFloat(0.6, 1.2);
			forkDir.Normalize();
			float len = Math.RandomFloat(2.0, 3.2);
			vector a = start;
			for ( int j = 0; j < 3; j++ )
			{
				vector b = a + forkDir * len;
				geb_BoltSegment(a, b, bolt);
				a = b;
				len = len * 0.6;
				forkDir = forkDir + side * Math.RandomFloat(-0.5, 0.5);
				forkDir.Normalize();
			}
		}
	}

	protected void geb_BoltSegment(vector a, vector b, int bolt)
	{
		vector seg = vector.Direction(a, b);
		float len = seg.Length();
		if ( len < 0.05 )
			return;

		vector mid = (a + b) * 0.5;
		ParticleSource p = ParticleManager.GetInstance().CreateInWorld(bolt, mid);
		if ( !p )
			return;

		vector frame[4];
		geb_BallFrame(seg.Normalized(), mid, frame);
		p.SetTransform(frame);
		p.ScaleParticleParamFromOriginal(EmitorParam.SIZE, len);
		p.PlayParticle();
	}

	// The muzzle light, in the fired cell's colour. Config turns vanilla's light off (illuminateWorld=0 under
	// Particles > OnFire > MuzzleFlash) because MuzzleFlashLight is orange whatever the shot looks like.
	// EEFired is the engine's per-shot event. It runs on the server and on every client that sees the shot,
	// remote players' shots included, and vanilla plays its muzzle particles from it (Weapon_Base.EEFired).
	// OnFire would be the wrong hook: the weapon state machine calls it only on the server and on the shooter's
	// own client, so other players would never see the light. Lights only exist on clients, so a dedicated
	// server returns straight away.
	override void EEFired(int muzzleType, int mode, string ammoType)
	{
		super.EEFired(muzzleType, mode, ammoType);

		if ( GetGame().IsDedicatedServer() )
			return;

		geb_PlayBall(ammoType);

		// Same place as vanilla's muzzle light: 0.2 m along -x from the muzzle point, i.e. just in front of the
		// muzzle, since the barrel points along -x (WeaponParticlesBase.OnActivate)
		vector pos = ModelToWorld(GetSelectionPositionLS("Usti hlavne") + Vector(-0.2, 0, 0));
		geb_PlasmaFlashLight light = geb_PlasmaFlashLight.Cast(ScriptedLightBase.CreateLight(geb_PlasmaFlashLight, pos));
		if ( !light )
			return;

		// Each cell's colour is CfgAmmo <cell> geb_muzzleLightColor[] = {r, g, b} in 0..1 (data/config.cpp)
		TFloatArray colour = new TFloatArray;
		GetGame().ConfigGetFloatArray("CfgAmmo " + ammoType + " geb_muzzleLightColor", colour);
		if ( colour.Count() == 3 )
			light.geb_SetColour(Vector(colour[0], colour[1], colour[2]));
	}
};

// The rifle's turning fluid (see geb_PlasmaRifle.AssembleGun): a plain item, so its model's clock animations run.
class geb_PlasmaRifleFluid : DummyItem
{
};
