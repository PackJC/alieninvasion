
class geb_PlasmaCartridge: MagazineStorage
{
	override void SetActions()
	{
		super.SetActions();

		//Deny unloading plasma (a loose cell cleared from the rifle's chamber can still be loaded back in)
		RemoveAction(ActionEmptyMagazine);
	}

	// No splitting either (R in the inventory, or right-click): vanilla's Magazine.SplitItem moves half the rounds
	// into a new copy of the magazine, so a cartridge of two cells became two cartridges of one. A cartridge is one
	// sealed cell pack.
	override bool CanBeSplit()
	{
		return false;
	}
};

// A loose cell (data/config.cpp cfgMagazines): what the rifle ejects when a chambered cell is cleared. It keeps the
// colour it was loaded as and loads back into any Montauk Cartridge.
class Ammo_Plasma_Cell: Ammunition_Base {};