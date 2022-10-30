
class geb_PlasmaCartridge: MagazineStorage
{
	override void SetActions()
	{
		super.SetActions();

		//Deny reloading or unloading plasma
		RemoveAction(ActionEmptyMagazine);
	}
};