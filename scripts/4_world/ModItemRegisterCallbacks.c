modded class ModItemRegisterCallbacks
{
	override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
	{
    	super.RegisterOneHanded(pType, pBehavior);
		pType.AddItemInHandsProfileIK("geb_PlasmaCartridge", "dz/anims/workspaces/player/player_main/props/player_main_stanag_mag.asi", pBehavior, "dz/anims/anm/player/ik/attachments/magazines/magazine_fal_20rnd.anm");
	}
};