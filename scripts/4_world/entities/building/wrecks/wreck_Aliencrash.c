class geb_Aliencrash extends CrashBase
{
	// Counts, chances and the gas zone toggle come from alieninvasion.json (Crash* settings)
	static const float ALIEN_RANGE_MIN = 5;
	static const float ALIEN_RANGE_MAX = 25;
	static const float LOOT_RANGE_MIN = 3;
	static const float LOOT_RANGE_MAX = 8;

	protected Particle m_FireEfx;
	protected Particle m_MistEfx;
	protected XmasSleighLight m_Light;
	protected geb_AlienRadiationArea m_RadiationArea;
	// Script-spawned aliens aren't managed by the Central Economy, so the wreck cleans up the ones still alive
	// when it despawns (CE only removes the wreck with no players inside the event's cleanup radius).
	protected ref array<EntityAI> m_Aliens = new array<EntityAI>;
	protected bool m_SiteInitialized;

	void geb_Aliencrash()
	{
		if ( !GetGame().IsDedicatedServer() )
		{
			//particles - Aurora trail (m_ParticleEfx is stopped by CrashBase)
			m_ParticleEfx = ParticleManager.GetInstance().PlayOnObject(ParticleList.UFO_WRECK, this, Vector(0, -1, 0));
			//was 2.35
			m_FireEfx = ParticleManager.GetInstance().PlayOnObject(ParticleList.UFO_FIRE, this, Vector(-0.45, 1.85, -0.5));
			m_MistEfx = ParticleManager.GetInstance().PlayOnObject(ParticleList.SPOOKY_MIST, this, Vector(0, -5, 0));
			m_Light = XmasSleighLight.Cast( ScriptedLightBase.CreateLight( XmasSleighLight, "0 0 0") );
			m_Light.SetAmbientColor(0.3, 1.0, 0.2);
			m_Light.SetDiffuseColor(0.3, 1.0, 0.2);
			m_Light.SetFlickerSpeed(2);
			m_Light.SetBrightnessTo(0.5);
			m_Light.AttachOnMemoryPoint(this, "light");
		}
	}

	// needs to have the soundset registered (see modded CrashBase)
	override string GetSoundSet()
	{
		return "AlienCrash_Distant_SoundSet";
	}

	// Wrecks from the StaticAlienCrash event get EEOnCECreate (which also sends the distant crash sound);
	// admin-spawned ones (e.g. COT) only get EEInit. Both set the site up once. EEInit waits a frame so the
	// admin tool has positioned the wreck, and only runs on a dedicated server: client-side spawn previews
	// never get there.
	override void EEInit()
	{
		super.EEInit();

		if ( GetGame().IsDedicatedServer() )
			GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( InitCrashSite, 0);
	}

	override void EEOnCECreate()
	{
		super.EEOnCECreate();
		InitCrashSite();
	}

	protected void InitCrashSite()
	{
		if ( m_SiteInitialized || !GetGame().IsServer() )
			return;

		m_SiteInitialized = true;
		SpawnLoot();
		SpawnRadiationArea();

		// Aliens only exist while someone is close enough to meet them, instead of idling at every crash site
		if ( GetAlienInvasionConfig().CrashAliensSpawnDistance <= 0 )
			GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( SpawnAliens, 0);
		else
			GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( WatchForPlayers, 5000, true);
	}

	protected void WatchForPlayers()
	{
		float range = GetAlienInvasionConfig().CrashAliensSpawnDistance;
		vector crashPos = GetPosition();
		array<Man> players = new array<Man>;
		GetGame().GetPlayers(players);
		foreach (Man player : players)
		{
			if ( player && vector.DistanceSq(player.GetPosition(), crashPos) <= range * range )
			{
				GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( WatchForPlayers );
				SpawnAliens();
				return;
			}
		}
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);

		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( InitCrashSite );
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( WatchForPlayers );
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).Remove( SpawnAliens );

		if ( !GetGame().IsDedicatedServer() )
		{
			if ( m_FireEfx )
				m_FireEfx.Stop();
			if ( m_MistEfx )
				m_MistEfx.Stop();
			if ( m_Light )
				m_Light.Destroy();
		}

		if ( GetGame().IsServer() )
		{
			if ( m_RadiationArea )
				m_RadiationArea.Delete();

			foreach (EntityAI alien : m_Aliens)
			{
				if ( alien && alien.IsAlive() )
					alien.Delete();
			}
		}
	}

	void SpawnAliens()
	{
		vector crashPos = GetPosition();
		Print("UFO Wreck: " + crashPos.ToString());

		geb_AlienInvasionConfig cfg = GetAlienInvasionConfig();
		int count = Math.RandomIntInclusive(cfg.CrashAliensMin, cfg.CrashAliensMax);
		for (int i = 0; i < count; i++)
		{
			vector pos = RandomGroundPosition(crashPos, ALIEN_RANGE_MIN, ALIEN_RANGE_MAX);
			EntityAI alien = EntityAI.Cast(GetGame().CreateObjectEx("geb_GreenAlien", pos, ECE_PLACE_ON_SURFACE|ECE_INITAI|ECE_EQUIP_ATTACHMENTS));
			if ( !alien )
				continue;

			m_Aliens.Insert(alien);
			vector orientation = alien.GetOrientation();
			alien.SetOrientation(Vector(Math.RandomFloat(0, 360), orientation[1], orientation[2]));
		}
	}

	void SpawnLoot()
	{
		vector crashPos = GetPosition();
		geb_AlienInvasionConfig cfg = GetAlienInvasionConfig();

		if ( Math.RandomFloat01() < cfg.CrashRifleChance )
		{
			Weapon_Base rifle = Weapon_Base.Cast(GetGame().CreateObjectEx("geb_PlasmaRifle", RandomGroundPosition(crashPos, LOOT_RANGE_MIN, LOOT_RANGE_MAX), ECE_PLACE_ON_SURFACE));
			if ( rifle )
				rifle.SpawnAttachedMagazine("geb_PlasmaCartridge");
		}

		int spares = Math.RandomIntInclusive(0, cfg.CrashSpareCartridgesMax);
		for (int i = 0; i < spares; i++)
		{
			Magazine cartridge = Magazine.Cast(GetGame().CreateObjectEx("geb_PlasmaCartridge", RandomGroundPosition(crashPos, LOOT_RANGE_MIN, LOOT_RANGE_MAX), ECE_PLACE_ON_SURFACE));
			if ( cartridge )
				cartridge.ServerSetAmmoCount(Math.RandomIntInclusive(1, cartridge.GetAmmoMax()));
		}
	}

	void SpawnRadiationArea()
	{
		if ( !GetAlienInvasionConfig().CrashGasZone )
			return;

		m_RadiationArea = geb_AlienRadiationArea.Cast(GetGame().CreateObject("geb_AlienRadiationArea", GetPosition()));
	}

	//Return a point on the ground between minRange and maxRange metres from origin, in a random direction.
	vector RandomGroundPosition(vector origin, float minRange, float maxRange)
	{
		float angle = Math.RandomFloat(0, Math.PI2);
		float distance = Math.RandomFloatInclusive(minRange, maxRange);

		vector pos = origin;
		pos[0] = pos[0] + Math.Cos(angle) * distance;
		pos[2] = pos[2] + Math.Sin(angle) * distance;
		pos[1] = GetGame().SurfaceY(pos[0], pos[2]);
		return pos;
	}
}
