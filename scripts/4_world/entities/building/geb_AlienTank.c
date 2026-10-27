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
//  - a low hum and bubbling, audible within a few metres (data/tank/config.cpp sound sets);
//  - the console screen's read-out: the "display" hidden selection steps through a 5 s loop of 20 frames
//    (geb_tank_screen_00_co .. _19_co, tools/alientank/paint.py) while a camera is within 40 m;
//  - the alien floating: while a camera is within 40 m it bobs, rocks and drifts its head, arms and legs a few
//    degrees through the user animation sources in data/tank/config.cpp (model.cfg from tools/alientank/build.py).
// Nothing moves and nothing is set when no camera is near; dedicated servers run none of it.
class geb_AlienTank : House
{
	// Screen loop: paint.py's SCREEN_FRAMES frames at SCREEN_FPS (one heartbeat per loop at 12 BPM)
	static const string GEB_SCREEN_TEXTURE = "alieninvasion\\data\\tank\\geb_tank_screen_%1_co.paa";
	static const int GEB_SCREEN_FRAMES = 20;
	static const int GEB_ANIM_STEP_MS = 250;
	static const int GEB_ANIM_IDLE_MS = 2000;		// a far tank checks this often whether a camera came near
	static const float GEB_ANIM_RANGE_SQ = 1600.0;	// 40 m
	// Floating alien: build.py's FLOAT_SOURCES (keep in step). Each source follows
	// 0.5 + 0.5 sin(2 pi (t / period + phase)); 0.5 is rest, the periods differ so the drift never repeats.
	static const int GEB_FLOAT_COUNT = 14;
	static const string GEB_FLOAT_SOURCES[14] = {"geb_bob", "geb_roll", "geb_pitch", "geb_head", "geb_arm_l", "geb_arm_r", "geb_armout_l", "geb_armout_r", "geb_forearm_l", "geb_forearm_r", "geb_leg_l", "geb_leg_r", "geb_shin_l", "geb_shin_r"};
	static const float GEB_FLOAT_PERIODS[14] = {7.0, 11.0, 9.0, 8.0, 6.3, 7.4, 8.6, 9.7, 5.6, 6.1, 7.9, 6.9, 5.9, 6.6};
	static const float GEB_FLOAT_PHASES[14] = {0.00, 0.30, 0.55, 0.15, 0.70, 0.20, 0.45, 0.90, 0.35, 0.80, 0.60, 0.05, 0.25, 0.50};

	protected ParticleSource m_geb_Bubbles;
	protected ParticleSource m_geb_Motes;
	protected ParticleSource m_geb_Steam;
	protected geb_AlienTankLight m_geb_Light;
	protected EffectSound m_geb_Hum;
	protected EffectSound m_geb_Bubbling;
	protected int m_geb_ScreenFrame;
	protected float m_geb_FloatTime;

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
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(geb_AnimStep);
		geb_StopEffects();

		super.EEDelete(parent);
	}

	// While a camera is within 40 m: shows the screen's next frame and aims each floating source at where its sine
	// will be one step from now (the engine glides there at the config's animPeriod), then comes back in 250 ms.
	// Farther away everything holds still and it only looks again every 2 s. Clients only (started by
	// geb_StartEffects), until deletion.
	protected void geb_AnimStep()
	{
		if ( IsSetForDeletion() )
			return;

		int delayMs = GEB_ANIM_IDLE_MS;
		if ( geb_CameraWithin(this, GEB_ANIM_RANGE_SQ) )
		{
			m_geb_ScreenFrame = geb_NextScreenFrame(this, GEB_SCREEN_TEXTURE, GEB_SCREEN_FRAMES, m_geb_ScreenFrame);

			m_geb_FloatTime += GEB_ANIM_STEP_MS * 0.001;
			float ahead = m_geb_FloatTime + GEB_ANIM_STEP_MS * 0.001;
			for ( int i = 0; i < GEB_FLOAT_COUNT; i++ )
			{
				SetAnimationPhase(GEB_FLOAT_SOURCES[i], 0.5 + 0.5 * Math.Sin(Math.PI2 * (ahead / GEB_FLOAT_PERIODS[i] + GEB_FLOAT_PHASES[i])));
			}
			delayMs = GEB_ANIM_STEP_MS;
		}
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(geb_AnimStep, delayMs, false);
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

		// each tank starts somewhere else in the drift, so two tanks never move together
		if ( m_geb_FloatTime == 0 )
			m_geb_FloatTime = Math.RandomFloat(0, 600);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(geb_AnimStep);
		geb_AnimStep();
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
