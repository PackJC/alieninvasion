class geb_Aliencrash extends CrashBase
{
	XmasSleighLight 		m_SleighLight;
	XmasSleighLight 		m_Light;
	
	int m_MaxDeersAmount = 15;
	int m_MinDeersAmount = 10;
	int m_MaxDeersSpawnRange = 25;
	int m_MinDeersSpawnRange = 5;

	override static bool Init()
    {
        CrashSoundSets.RegisterSoundSet("AlienCrash_Distant_SoundSet");
        return true;
    }

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
		SpawnRandomDeerLater();
	}
	
	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);
		
		if ( !GetGame().IsDedicatedServer() )
		{
			if ( m_SleighLight )
				m_SleighLight.Destroy();
		}	
	}
	
	void SpawnRandomDeerLater()
	{
		//SpawnRandomDeers();
		GetGame().GetCallQueue( CALL_CATEGORY_GAMEPLAY ).CallLater( SpawnRandomDeers, 0);
	}
	
	//Spawn a random amount of (dead) deers around the sleigh
	void SpawnRandomDeers()
	{
		EntityAI deer;
		vector crash_pos = GetPosition();
		Print("UFO Wreck: " + crash_pos)
		int deersAmount = Math.RandomIntInclusive(m_MinDeersAmount,m_MaxDeersAmount);
		
		for (int i = 0; i < m_MaxDeersAmount; i++)
		{
			vector deer_pos = RandomizePosition(crash_pos);
			//Print("Position: " + deer_pos);
			deer = EntityAI.Cast(GetGame().CreateObject("geb_GreenAlien", deer_pos,false, true));
			deer.SetHealth01("","Health", 1.0);
			vector orientation = deer.GetOrientation();
			deer.SetOrientation(Vector(Math.RandomIntInclusive(0,360),orientation[1],orientation[2]));
		}		
	}
	
	//Return a new vector scattered around origin.
	vector RandomizePosition(vector origin)
	{		
		int randX;
		int randZ;
		
		randX = Math.RandomIntInclusive(m_MinDeersSpawnRange, m_MaxDeersSpawnRange);
		if (Math.RandomIntInclusive(0,1) < 1)
			randX = -randX;
			
		randZ = Math.RandomIntInclusive(m_MinDeersSpawnRange, m_MaxDeersSpawnRange);
		if (Math.RandomIntInclusive(0,1) < 1)
			randZ = -randZ; 
			
		origin[0] = origin[0] + randX;
		origin[2] = origin[2] + randZ;
		
		return origin;
	
	}
}