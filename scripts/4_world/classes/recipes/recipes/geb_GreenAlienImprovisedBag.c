// A Roswell Hide Courier Bag + three sticks make a Roswell Hide Backpack; breaking it down gives a stick
// and the hide back. Copies of vanilla's fur backpack recipes with the bags and hide swapped; vanilla's own
// recipes skip the green bags so the action menu never offers a brown result for them.
modded class CraftImprovisedLeatherBag
{
	protected bool m_geb_GreenAlien; // set on the Roswell copy below

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[1].IsKindOf("geb_GreenAlienCourierBag") != m_geb_GreenAlien )
			return false;

		return super.CanDo(ingredients, player);
	}
};

modded class DeCraftImprovisedLeatherBag
{
	protected bool m_geb_GreenAlien;

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[0].IsKindOf("geb_GreenAlienImprovisedBag") != m_geb_GreenAlien )
			return false;

		return super.CanDo(ingredients, player);
	}
};

class geb_CraftGreenAlienImprovisedBag extends CraftImprovisedLeatherBag
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(1, "FurCourierBag");
		InsertIngredient(1, "geb_GreenAlienCourierBag");
		m_ItemsToCreate[0] = "geb_GreenAlienImprovisedBag";
		m_geb_GreenAlien = true;
	}
};

class geb_DeCraftGreenAlienImprovisedBag extends DeCraftImprovisedLeatherBag
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(0, "FurImprovisedBag");
		InsertIngredient(0, "geb_GreenAlienImprovisedBag");
		m_ItemsToCreate[1] = "geb_GreenAlienSkin"; // result 0 stays vanilla's stick
		m_geb_GreenAlien = true;
	}
};
