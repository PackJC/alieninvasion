class geb_Aliencrash extends CrashBase
{
	XmasSleighLight 		m_Light;
	
	int m_MaxAliensAmount = 15;
	int m_MinAliensAmount = 10;
	int m_MaxAliensSpawnRange = 25;
	int m_MinAliensSpawnRange = 5;

	void geb_Aliencrash()
	{
		if ( !GetGame().IsDedicatedServer() )
		{
			//particles - Aurora trail
			m_ParticleEfx = ParticleManager.GetInstance().PlayOnObject(ParticleList.UFO_WRECK, this, Vector(0, -1, 0));
			//was 2.35
			Particle m_ParticleEfx2 = ParticleManager.GetInstance().PlayOnObject(ParticleList.UFO_FIRE, this, Vector(-0.45, 1.85, -0.5));
			Particle m_ParticleEfx4 = ParticleManager.GetInstance().PlayOnObject(ParticleList.SPOOKY_MIST, this, Vector(0, -5, 0));
			m_Light = XmasSleighLight.Cast( ScriptedLightBase.CreateLight( XmasSleighLight, "0 0 0") );
			m_Light.SetAmbientColor(0.3, 1.0, 0.2);
			m_Light.SetDiffuseColor(0.3, 1.0, 0.2);
			m_Light.SetFlickerSpeed(2);
			m_Light.SetBrightnessTo(0.5);
			m_Light.AttachOnMemoryPoint(this, "light");
		}
	}
	
	// needs to have the soundset registered in CrashBase.Init()
	override string GetSoundSet()
	{
		return "AlienCrash_Distant_SoundSet";
	}
	
	override void EEOnCECreate()
	{
		super.EEOnCECreate();
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(SpawnAliens, 0);
	}
	
	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		if ( !GetGame().IsDedicatedServer() )
		{
			if (m_Light)
				m_Light.Destroy();
		}	
	}

	// Spawn a random number of aliens around the wreck.
	void SpawnAliens()
	{
		vector crash_pos = GetPosition();
		Print("UFO Wreck: " + crash_pos);
		int aliensAmount = Math.RandomIntInclusive(m_MinAliensAmount, m_MaxAliensAmount);
		
		for (int i = 0; i < aliensAmount; i++)
		{
			vector alien_pos = RandomizePosition(crash_pos);
			EntityAI alien = EntityAI.Cast(GetGame().CreateObject("geb_GreenAlien", alien_pos, false, true));
			if (!alien)
				continue;

			alien.SetHealth01("", "Health", 1.0);
			vector orientation = alien.GetOrientation();
			alien.SetOrientation(Vector(Math.RandomIntInclusive(0, 360), orientation[1], orientation[2]));
		}		
	}
	
	//Return a new vector scattered around origin.
	vector RandomizePosition(vector origin)
	{		
		int randX;
		int randZ;
		
		randX = Math.RandomIntInclusive(m_MinAliensSpawnRange, m_MaxAliensSpawnRange);
		if (Math.RandomIntInclusive(0,1) < 1)
			randX = -randX;
			
		randZ = Math.RandomIntInclusive(m_MinAliensSpawnRange, m_MaxAliensSpawnRange);
		if (Math.RandomIntInclusive(0,1) < 1)
			randZ = -randZ; 
			
		origin[0] = origin[0] + randX;
		origin[2] = origin[2] + randZ;
		
		return origin;
	
	}
}
