/*

  CREATED BY PACKJC
  https://github.com/PackJC/gebsfish
  https://steamcommunity.com/sharedfiles/filedetails/?id=2757509117
  https://discord.com/invite/G8uSGZ8yyf
  Contributions welcome via github

*/

modded class PluginRecipesManager
{
	override void RegisterRecipies()
	{
		super.RegisterRecipies();

		// Aliens are gutted with the normal skinning action (Skinning class in config), not a recipe:
		// recipes only take inventory items, never a corpse.
		RegisterRecipe(new geb_RechargePlasmaCartridge);

		// Roswell Hide crafting chain (green copies of vanilla's leather and fur bag recipes)
		RegisterRecipe(new geb_TanGreenAlienHide);
		RegisterRecipe(new geb_CraftGreenAlienLeatherSack);
		RegisterRecipe(new geb_DeCraftGreenAlienLeatherSack);
		RegisterRecipe(new geb_CraftGreenAlienCourierBag);
		RegisterRecipe(new geb_DeCraftGreenAlienCourierBag);
		RegisterRecipe(new geb_CraftGreenAlienImprovisedBag);
		RegisterRecipe(new geb_DeCraftGreenAlienImprovisedBag);
	}

};