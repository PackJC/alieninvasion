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

	// The plasma shot particle comes from config (Particles > OnFire > MuzzleFlash and the ammo's
	// muzzleFlashParticle), so it isn't played again here.
};
