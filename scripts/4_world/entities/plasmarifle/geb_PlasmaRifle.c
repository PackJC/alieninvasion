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

	// The plasma shot particle comes from config (Particles > OnFire > MuzzleFlash and the ammo's
	// muzzleFlashParticle), so it isn't played again here.

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

		// Same place as vanilla's muzzle light: 0.2 m behind the muzzle point (WeaponParticlesBase.OnActivate)
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
