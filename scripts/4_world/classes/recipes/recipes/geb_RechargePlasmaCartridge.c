// Plasma cells can't be unloaded or found loose, so cartridges are refilled from 9V batteries instead.
// A full battery refills a full cartridge; a partial one adds as many cells as its charge covers.
class geb_RechargePlasmaCartridge extends RecipeBase
{
	override void Init()
	{
		m_Name = "#STR_geb_RechargePlasma";
		m_IsInstaRecipe = false;
		m_AnimationLength = 1;
		m_Specialty = 0;

		m_MinDamageIngredient[0] = -1;
		m_MaxDamageIngredient[0] = 3;
		m_MinQuantityIngredient[0] = -1;
		m_MaxQuantityIngredient[0] = -1;

		m_MinDamageIngredient[1] = -1;
		m_MaxDamageIngredient[1] = 3;
		m_MinQuantityIngredient[1] = -1;
		m_MaxQuantityIngredient[1] = -1;

		InsertIngredient(0, "geb_PlasmaCartridge");
		m_IngredientAddHealth[0] = 0;
		m_IngredientSetHealth[0] = -1;
		m_IngredientAddQuantity[0] = 0;
		m_IngredientDestroy[0] = false;
		m_IngredientUseSoftSkills[0] = false;

		InsertIngredient(1, "Battery9V");
		m_IngredientAddHealth[1] = 0;
		m_IngredientSetHealth[1] = -1;
		m_IngredientAddQuantity[1] = 0;
		m_IngredientDestroy[1] = false;
		m_IngredientUseSoftSkills[1] = false;
	}

	override bool CanDo(ItemBase ingredients[], PlayerBase player)
	{
		Magazine cartridge = Magazine.Cast(ingredients[0]);
		ItemBase battery = ingredients[1];
		if ( !cartridge || !battery || !battery.GetCompEM() )
			return false;

		return cartridge.GetAmmoCount() < cartridge.GetAmmoMax() && BatteryEnergy(battery) >= EnergyPerCell(cartridge, battery);
	}

	override void Do(ItemBase ingredients[], PlayerBase player, array<ItemBase> results, float specialty_weight)
	{
		Magazine cartridge = Magazine.Cast(ingredients[0]);
		ItemBase battery = ingredients[1];
		if ( !cartridge || !battery || !battery.GetCompEM() )
			return;

		float perCell = EnergyPerCell(cartridge, battery);
		int missing = cartridge.GetAmmoMax() - cartridge.GetAmmoCount();
		int affordable = Math.Floor(BatteryEnergy(battery) / perCell + 0.001);
		int cells = Math.Min(missing, affordable);
		if ( cells <= 0 )
			return;

		cartridge.ServerSetAmmoCount(cartridge.GetAmmoCount() + cells);
		battery.GetCompEM().AddEnergy(-cells * perCell);
	}

	protected float EnergyPerCell(Magazine cartridge, ItemBase battery)
	{
		return battery.GetCompEM().GetEnergyMax() / cartridge.GetAmmoMax();
	}

	// Energy only exists on the server; clients see it through the battery's synced quantity.
	protected float BatteryEnergy(ItemBase battery)
	{
		if ( GetGame().IsServer() || !GetGame().IsMultiplayer() )
			return battery.GetCompEM().GetEnergy();

		return battery.GetQuantityNormalized() * battery.GetCompEM().GetEnergyMax();
	}
};
