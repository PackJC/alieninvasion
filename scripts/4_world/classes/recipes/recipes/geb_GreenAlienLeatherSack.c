// Two Roswell Leather + a leather sewing kit make a Roswell Leather Backpack, which breaks back down into
// Roswell Leather. Copies of vanilla's leather backpack recipes with the leather and backpack swapped;
// vanilla's own recipes skip the green items so the action menu never offers a brown result for them.
modded class CraftLeatherSack
{
	protected bool m_geb_GreenAlien; // set on the Roswell copy below

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[0].IsKindOf("geb_GreenAlienLeather") != m_geb_GreenAlien )
			return false;

		return super.CanDo(ingredients, player);
	}
};

modded class DeCraftLeatherSack
{
	protected bool m_geb_GreenAlien;

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[0].IsKindOf("geb_GreenAlienLeatherSack") != m_geb_GreenAlien )
			return false;

		return super.CanDo(ingredients, player);
	}
};

class geb_CraftGreenAlienLeatherSack extends CraftLeatherSack
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(0, "TannedLeather");
		InsertIngredient(0, "geb_GreenAlienLeather");
		m_ItemsToCreate[0] = "geb_GreenAlienLeatherSack";
		m_geb_GreenAlien = true;
	}
};

class geb_DeCraftGreenAlienLeatherSack extends DeCraftLeatherSack
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(0, "LeatherSack_ColorBase");
		InsertIngredient(0, "geb_GreenAlienLeatherSack");
		m_ItemsToCreate[0] = "geb_GreenAlienLeather";
		m_geb_GreenAlien = true;
	}
};
