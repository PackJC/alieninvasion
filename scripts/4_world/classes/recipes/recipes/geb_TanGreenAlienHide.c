// Roswell Hide + Garden Lime tans into Roswell Leather. A copy of vanilla's "Tan leather" recipe that only
// takes the hide; vanilla's recipe skips the hide, so the action menu offers just the green result.
// (Tanning in a barrel is handled in geb_Barrel_ColorBase.c.)
modded class CraftTannedLeather
{
	protected bool m_geb_GreenAlien; // set on the Roswell copy below

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		if ( ingredients[0].IsKindOf("geb_GreenAlienSkin") != m_geb_GreenAlien )
			return false;

		return super.CanDo(ingredients, player);
	}
};

class geb_TanGreenAlienHide extends CraftTannedLeather
{
	override void Init()
	{
		super.Init();
		RemoveIngredient(0, "Pelt_Base");
		InsertIngredient(0, "geb_GreenAlienSkin");
		m_geb_GreenAlien = true;
	}

	// Vanilla's Do, with Roswell Leather in place of Tanned Leather
	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		ItemBase hide = ingredients[0];
		int yieldQuantity = hide.ConfigGetFloat("leatherYield");
		yieldQuantity = Math.Clamp(yieldQuantity * hide.GetHealth01("", "Health"), 1, float.MAX);

		ItemBase gardenLime = ingredients[1];
		float usedLime = gardenLime.GetQuantityMax() * m_PercentageUsed * yieldQuantity;
		gardenLime.SetQuantity(gardenLime.GetQuantity() - usedLime);

		vector posHead;
		MiscGameplayFunctions.GetHeadBonePos(player, posHead);
		vector posTarget = player.GetPosition() + (player.GetDirection() * DEFAULT_SPAWN_DISTANCE);
		MiscGameplayFunctions.CreateItemBasePilesDispersed("geb_GreenAlienLeather", posHead, posTarget, UAItemsSpreadRadius.NARROW, yieldQuantity, float.MAX, player);
	}
};
