// Toxic cloud around a UFO crash: vanilla gas-zone mechanics (NBC gear protects), sized to the wreck itself
// (8 m, down from 18 m with an inner and an outer ring of clouds, which cost a lot of frame rate at the site).
// The wreck creates it on spawn and deletes it on despawn; MAX_LIFETIME is only a safety net.
// Mirrors vanilla ContaminatedArea_Local (a gas grenade's 10 m zone: one ring of clouds, no inner or outer
// rings), which can't be subclassed for this because its SetupZoneData hard-codes the 10 m radius.
class geb_AlienRadiationArea : ContaminatedArea_DynamicBase
{
	const float TICK_RATE = 1;
	const float MAX_LIFETIME = 10800;

	ref Timer m_LifetimeTimer = new Timer;
	float m_Lifetime = MAX_LIFETIME;

	void geb_AlienRadiationArea()
	{
		m_EffectsPriority = -10;
	}

	override void SetupZoneData(EffectAreaParams params)
	{
		params.m_ParamPartId 		= ParticleList.CONTAMINATED_AREA_GAS_AROUND;
		params.m_ParamInnerRings 	= 0;
		params.m_ParamPosHeight 	= 4;
		params.m_ParamNegHeight 	= 3;
		params.m_ParamRadius 		= 8;
		params.m_ParamOuterToggle 	= false;
		params.m_ParamTriggerType 	= "ContaminatedTrigger_Local";

		params.m_ParamAroundPartId 	= 0;
		params.m_ParamTinyPartId 	= 0;

		super.SetupZoneData(params);

		InitZone();
	}

	override void EEInit()
	{
		if (GetGame().IsServer() || !GetGame().IsMultiplayer())
			m_LifetimeTimer.Run(TICK_RATE, this, "Tick", NULL, true);
	}

	override void DeferredInit()
	{
		if (!m_ToxicClouds)
			m_ToxicClouds = new array<Particle>();

		SetupZoneData(new EffectAreaParams);

		super.DeferredInit();
	}

	override void SpawnParticles(ParticlePropertiesArray props, vector centerPos, vector partPos, inout int count)
	{
		partPos[1] = GetGame().SurfaceRoadY(partPos[0], partPos[2]);	// Snap particles to ground

		// Keep the particle inside the trigger
		if (!Math.IsInRange(partPos[1], centerPos[1] - m_NegativeHeight, centerPos[1] + m_PositiveHeight))
			partPos[1] = centerPos[1];

		props.Insert(ParticleProperties(partPos, ParticlePropertiesFlags.PLAY_ON_CREATION, null, GetGame().GetSurfaceOrientation( partPos[0], partPos[2] ), this));
		++count;
	}

	override float GetStartDecayLifetime()
	{
		return 20;
	}

	override float GetFinishDecayLifetime()
	{
		return 10;
	}

	override float GetRemainingTime()
	{
		return m_Lifetime;
	}

	override void Tick()
	{
		m_Lifetime -= TICK_RATE;
		if (m_Lifetime <= 0)
			Delete();
	}
}
