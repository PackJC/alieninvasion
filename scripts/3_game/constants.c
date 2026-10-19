const string VERSION_ALIENINVASION = "1.2.0"; // Current version of the mod; bumping it regenerates the Gebs/mpmissions files

// Server -> client RPC ids. Arbitrary; they only have to stay clear of vanilla ERPCs and other mods.
const int GEB_RPC_ALIEN_VISION = 7194263;	// on the eating player: start night vision
const int GEB_RPC_ALIEN_ZAP = 7194264;		// on an alien: zap effect at the victim
const int GEB_RPC_ALIEN_DEATH = 7194265;	// on an alien: death effect

// Plasma cartridge colours (data/config.cpp: green, blue, lime, violet). Crash sites, alien loot and admin
// spawns pick one at random; each one's cell sets its own muzzle effect, light and bolt.
TStringArray geb_PlasmaCartridgeTypes()
{
	TStringArray types = {"geb_PlasmaCartridge", "geb_PlasmaCartridge_Arc", "geb_PlasmaCartridge_Lance", "geb_PlasmaCartridge_Void"};
	return types;
}

string geb_RandomPlasmaCartridgeType()
{
	return geb_PlasmaCartridgeTypes().GetRandomElement();
}
