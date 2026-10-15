// The specimen tank's glow: a green light in the fluid, in front of the alien, that slowly breathes.
// Lit in daylight too, since the tank is meant for labs and bunkers.
class geb_AlienTankLight extends PointLightBase
{
	void geb_AlienTankLight()
	{
		SetVisibleDuringDaylight(true);
		SetRadiusTo(6);
		SetBrightnessTo(2.2);
		SetCastShadow(false);
		SetFlareVisible(false);
		SetDiffuseColor(0.35, 1.0, 0.45);
		SetAmbientColor(0.35, 1.0, 0.45);
		SetFlickerSpeed(0.5);
		SetFlickerAmplitude(0.1);
		FadeIn(1.5);
		SetFadeOutTime(0.5);
	}
};

// geb_AlienTank (data/tank/config.cpp): a containment tank with a sleeping alien in glowing green fluid, a prop
// that admins and map makers place (nothing spawns it). The model and its glowing materials carry most of the
// look; on clients this adds the rest at the model's memory points:
//  - bubbles rising from the floor grille (GEB_TANK_BUBBLES at "bubbles") and green motes drifting in the fluid
//    (GEB_TANK_MOTES at "motes");
//  - wisps from the vent valve on the lid (GEB_TANK_STEAM at "steam");
//  - the green light (geb_AlienTankLight at "light");
//  - a low hum and bubbling, audible within a few metres (data/tank/config.cpp sound sets).
class geb_AlienTank : House
{
	protected ParticleSource m_geb_Bubbles;
	protected ParticleSource m_geb_Motes;
	protected ParticleSource m_geb_Steam;
	protected geb_AlienTankLight m_geb_Light;
	protected EffectSound m_geb_Hum;
	protected EffectSound m_geb_Bubbling;

	override void EEInit()
	{
		super.EEInit();

		// Admin tools and editors move a new object into place just after creating it: start a moment later
		if ( !GetGame().IsDedicatedServer() )
			GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(geb_StartEffects, 250, false);
	}

	override void EEDelete(EntityAI parent)
	{
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(geb_StartEffects);
		geb_StopEffects();

		super.EEDelete(parent);
	}

	protected void geb_StartEffects()
	{
		if ( IsSetForDeletion() )
			return;

		if ( !m_geb_Bubbles )
			m_geb_Bubbles = geb_PlayParticle(ParticleList.GEB_TANK_BUBBLES, "bubbles");
		if ( !m_geb_Motes )
			m_geb_Motes = geb_PlayParticle(ParticleList.GEB_TANK_MOTES, "motes");
		if ( !m_geb_Steam )
			m_geb_Steam = geb_PlayParticle(ParticleList.GEB_TANK_STEAM, "steam");

		if ( !m_geb_Light )
		{
			m_geb_Light = geb_AlienTankLight.Cast(ScriptedLightBase.CreateLight(geb_AlienTankLight, "0 0 0"));
			if ( m_geb_Light )
				m_geb_Light.AttachOnMemoryPoint(this, "light");
		}

		if ( !m_geb_Hum )
			m_geb_Hum = SEffectManager.PlaySoundOnObject("geb_AlienTankHum_SoundSet", this, 1.0, 0.5, true);
		if ( !m_geb_Bubbling )
			m_geb_Bubbling = SEffectManager.PlaySoundOnObject("geb_AlienTankBubbling_SoundSet", this, 1.0, 0.5, true);
	}

	// In the tank's own frame (not the world's), so the effects turn with a rotated tank
	protected ParticleSource geb_PlayParticle(int particleId, string memoryPoint)
	{
		ParticleSource p = ParticleManager.GetInstance().PlayOnObject(particleId, this, GetMemoryPointPos(memoryPoint), "0 0 0", false);
		if ( p )
			p.SetParticleAutoDestroyFlags(ParticleAutoDestroyFlags.ON_STOP);
		return p;
	}

	protected void geb_StopEffects()
	{
		if ( m_geb_Bubbles )
		{
			m_geb_Bubbles.Stop();
			m_geb_Bubbles = null;
		}
		if ( m_geb_Motes )
		{
			m_geb_Motes.Stop();
			m_geb_Motes = null;
		}
		if ( m_geb_Steam )
		{
			m_geb_Steam.Stop();
			m_geb_Steam = null;
		}
		if ( m_geb_Light )
		{
			m_geb_Light.FadeOut();
			m_geb_Light = null;
		}
		if ( m_geb_Hum )
		{
			m_geb_Hum.SoundStop();
			SEffectManager.DestroyEffect(m_geb_Hum);
			m_geb_Hum = null;
		}
		if ( m_geb_Bubbling )
		{
			m_geb_Bubbling.SoundStop();
			SEffectManager.DestroyEffect(m_geb_Bubbling);
			m_geb_Bubbling = null;
		}
	}
};
