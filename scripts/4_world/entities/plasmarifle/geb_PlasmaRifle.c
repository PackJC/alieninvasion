class geb_PlasmaRifle : FAL_Base
{
	override void EEFired(int muzzleType, int mode, string ammoType)
	{
		super.EEFired(muzzleType, mode, ammoType);

		if (GetGame().IsClient() || !GetGame().IsMultiplayer())
		{
			Particle.PlayOnObject(ParticleList.geb_plasma_shot, this, Vector(-0.75, 0.05, 0), Vector(0, 0, 0), false);
		}
	}
};
