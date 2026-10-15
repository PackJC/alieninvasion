// Bloody hands from an alien use a green copy of the survivor's bloody-hands material: vanilla's "*_bloodyhands"
// material from the survivor's config, renamed "*_alienblood", in data/blood/hands (tools/build_alien_hands.py).
modded class PluginLifespan
{
	override protected void SetHandsMaterial(PlayerBase player, int material_type)
	{
		if ( material_type == BloodyHands.MATERIAL_TYPE_BLOODY && player.geb_HasAlienBloodHands() )
		{
			string green = geb_AlienHandsMaterial(player.GetPlayerClass());
			EntityAI hands = player.GetInventory().FindPlaceholderForSlot(InventorySlots.GetSlotIdFromString("Gloves"));
			if ( green != "" && hands )
			{
				hands.SetObjectMaterial(0, green);
				return;
			}
		}

		super.SetHandsMaterial(player, material_type);
	}

	protected string geb_AlienHandsMaterial(string playerClass)
	{
		if ( !m_BloodyHands.Contains(playerClass) )
			return "";

		string bloody = m_BloodyHands.Get(playerClass).GetMaterial(BloodyHands.MATERIAL_TYPE_BLOODY);
		bloody.ToLower();
		int slash = bloody.LastIndexOf("\\");
		string file = bloody.Substring(slash + 1, bloody.Length() - slash - 1);
		if ( file.Replace("_bloodyhands", "_alienblood") == 0 )
			return "";

		return "alieninvasion\\data\\blood\\hands\\" + file;
	}
}
