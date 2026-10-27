// Aliens bleed green. Their Fire Geometry uses vanilla's meatbones surface (the infected "body" hit sound), so
// bullet and melee hits on them come through Hit_MeatBones and its melee variants; when the thing hit is an
// alien, the same effect plays the green copies of the blood particles (tools/build_alien_blood.py).
// Fist hits keep vanilla's "no particle".
modded class Hit_MeatBones
{
	protected bool m_geb_AlienBlood;

	override void EvaluateEffect(Object directHit, int componentIndex, vector pos, int impact_type, vector surfNormal, vector exitPos, vector inSpeed, vector outSpeed, string ammoType)
	{
		m_geb_AlienBlood = directHit && directHit.IsKindOf("geb_GreenAlienBase");
		if ( m_geb_AlienBlood )
		{
			if ( m_ParticleEnter == ParticleList.IMPACT_MEATBONES_ENTER )
				SetEnterParticle(ParticleList.GEB_ALIEN_BLOOD_ENTER);
			if ( m_ParticleExit == ParticleList.IMPACT_MEATBONES_EXIT )
				SetExitParticle(ParticleList.GEB_ALIEN_BLOOD_EXIT);
			if ( m_ParticleRicochet == ParticleList.IMPACT_MEATBONES_RICOCHET )
				SetRicochetParticle(ParticleList.GEB_ALIEN_BLOOD_ENTER);
		}

		super.EvaluateEffect(directHit, componentIndex, pos, impact_type, surfNormal, exitPos, inSpeed, outSpeed, ammoType);
	}

	// Vanilla's ground splatter with the green drops and chunks. Keep in step with Hit_MeatBones.BloodSplatGround.
	override void BloodSplatGround( vector start_pos, vector speed_vector, float decay_coef )
	{
		if ( !m_geb_AlienBlood )
		{
			super.BloodSplatGround(start_pos, speed_vector, decay_coef);
			return;
		}

		vector pos = start_pos;
		float groundHeight = g_Game.SurfaceY(pos[0], pos[2]);
		if (groundHeight > start_pos[1])
			return;

		float power = m_StoppingForce;
		float upscale = 1;
		float rnd_offset = 0.5;
		float rnd_offset_2 = rnd_offset * 0.5;

		while (power > 200)
		{
			pos = pos + ( speed_vector * 0.001 );
			pos = pos + Vector( rnd_offset_2 - Math.RandomFloat( 0, rnd_offset ), 0, rnd_offset_2 - Math.RandomFloat( 0, rnd_offset ) );
			pos[1] = g_Game.SurfaceY(pos[0], pos[2]);

			vector ori = g_Game.GetSurfaceOrientation(pos[0], pos[2]);

			Particle drops = ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_ALIEN_BLOOD_DROPS, pos);
			if ( drops )
			{
				drops.SetOrientation(ori);
				drops.ScaleParticleParam(EmitorParam.SIZE, upscale);
			}

			Particle chunks = ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_ALIEN_BLOOD_CHUNKS, pos);
			if ( chunks )
				chunks.SetOrientation(ori);

			power = power * decay_coef;
			upscale = upscale + Math.RandomFloat(0.1, 1);

			BloodSplatWall();
		}
	}
}
