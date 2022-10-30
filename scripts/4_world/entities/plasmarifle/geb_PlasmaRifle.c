class geb_PlasmaRifle : FAL_Base
{
	override void EEFired (int muzzleType, int mode, string ammoType)
 	{
    	if (GetGame().IsClient() || !GetGame().IsMultiplayer())
    	{
      		super.EEFired(muzzleType, mode, ammoType);
     		{

                //in future it could be IfAmmoType then
                //could make different ammo shoot different stuff
                Particle.PlayOnObject(ParticleList.geb_plasma_shot, this, Vector(-0.75, 0.05, 0), Vector(0, 0, 0), false);
      		}
   	 	}
  	}
};