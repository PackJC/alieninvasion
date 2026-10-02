const string VERSION_ALIENINVASION = "1.2.0"; // Current version of the mod; bumping it regenerates the Gebs/mpmissions files

// Server -> client RPC ids. Arbitrary; they only have to stay clear of vanilla ERPCs and other mods.
const int GEB_RPC_ALIEN_VISION = 7194263;	// on the eating player: start night vision
const int GEB_RPC_ALIEN_ZAP = 7194264;		// on an alien: zap effect at the victim
const int GEB_RPC_ALIEN_DEATH = 7194265;	// on an alien: death effect
