modded class ModItemRegisterCallbacks
{
	override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
	{
    	super.RegisterOneHanded(pType, pBehavior);
		pType.AddItemInHandsProfileIK("geb_PlasmaCartridge", "dz/anims/workspaces/player/player_main/props/player_main_stanag_mag.asi", pBehavior, "dz/anims/anm/player/ik/attachments/magazines/magazine_fal_20rnd.anm");
		// held like vanilla's M67 (tools/plasmagrenade/build.py fits the ball to this pose)
		pType.AddItemInHandsProfileIK("geb_PlasmaGrenade", "dz/anims/workspaces/player/player_main/player_main_1h.asi", pBehavior, "dz/anims/anm/player/ik/explosives/grenade.anm");
	}
};