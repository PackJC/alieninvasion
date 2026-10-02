// A Roswell Hide + rope make a Roswell Hide Courier Bag; breaking it down gives the hide and rope back.
// Copies of vanilla's fur courier bag recipes (which use a wild boar pelt) with the hide and bag swapped.
// Vanilla's craft never accepts the hide, so only its break-down needs to skip the green bag.
modded class DeCraftLeatherCourierBag
{
	protected bool m_geb_GreenAlien; // set on the Roswell copy below

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[0].IsKindOf("geb_GreenAlienCourierBag") != m_geb_GreenAlien )
			return false;

		return super.CanDo(ingredients, player);
	}
};

class geb_CraftGreenAlienCourierBag extends CraftLeatherCourierBag
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(0, "WildboarPelt");
		InsertIngredient(0, "geb_GreenAlienSkin");
		m_ItemsToCreate[0] = "geb_GreenAlienCourierBag";
	}
};

class geb_DeCraftGreenAlienCourierBag extends DeCraftLeatherCourierBag
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(0, "FurCourierBag");
		InsertIngredient(0, "geb_GreenAlienCourierBag");
		m_ItemsToCreate[0] = "geb_GreenAlienSkin"; // result 1 stays vanilla's rope
		m_geb_GreenAlien = true;
	}
};
