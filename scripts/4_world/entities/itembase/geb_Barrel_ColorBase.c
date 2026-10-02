// Vanilla's barrel tanning hard-codes Tanned Leather as the result; this is the same method with one change:
// a Roswell Hide tans into Roswell Leather. Keep it in step with vanilla Barrel_ColorBase.TanPelts.
modded class Barrel_ColorBase
{
	override void TanPelts( ItemBase lime, PlayerBase player )
	{
		EntityAI item;
		int item_count = GetInventory().GetCargo().GetItemCount();
		int pelt_count = 0;
		int lime_amount = Math.Floor(lime.GetQuantity()/GameConstants.BAREL_LIME_PER_PELT);

		for (int i = 0; i < item_count; i++)
		{
			item = GetInventory().GetCargo().GetItem(i);
			if ( item.IsPeltBase() )
			{
				pelt_count = g_Game.ConfigGetInt("cfgVehicles " + item.GetType() + " peltGain");
				if ( pelt_count <= lime_amount )
				{
					string leather = "TannedLeather";
					if ( item.IsKindOf("geb_GreenAlienSkin") )
						leather = "geb_GreenAlienLeather";

					TanLeatherLambda lambda = new TanLeatherLambda(item, leather, player, pelt_count);
					lambda.SetTransferParams(true, true, true);
					player.ServerReplaceItemWithNew(lambda);

					lime_amount -= pelt_count;
					if ( lime_amount <= 0 )
					{
						lime.Delete();
						break;
					}
				}
			}
		}
		if ( lime )
		{
			lime.SetQuantity(lime_amount*GameConstants.BAREL_LIME_PER_PELT);
		}
		if ( pelt_count > 0 )
		{
			Lock(30);
		}
	}
};
