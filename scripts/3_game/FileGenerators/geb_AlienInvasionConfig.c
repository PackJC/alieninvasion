// Server tunables in $profile:Gebs/alieninvasion.json, alongside gebsfish's configs. Every value is used on the
// server only, so nothing is synced to clients. Like gebsfish: a missing file is created with defaults, a file that
// fails to parse is left untouched (defaults are used for that session), and the file is only rewritten on a
// version bump, which also adds any new settings without touching existing values.
class geb_AlienInvasionConfig
{
	string ConfigVersion = "";

	string CrashInfo = "UFO crash sites: aliens spawned around each wreck once a player comes within CrashAliensSpawnDistance metres (0 = as soon as it crashes), chance of a plasma rifle in the wreckage, up to N spare cartridges, and whether the wreck is surrounded by a toxic gas zone (NBC gear protects).";
	int CrashAliensMin = 10;
	int CrashAliensMax = 15;
	float CrashAliensSpawnDistance = 300;
	float CrashRifleChance = 0.35;
	int CrashSpareCartridgesMax = 2;
	bool CrashGasZone = true;

	string PsychicZapInfo = "Aliens chasing a player in line of sight zap them from PsychicZapMinRange..PsychicZapRange metres, at most once per PsychicZapCooldown seconds each. A foil hat blocks it. Closer than the min range they use normal melee.";
	bool PsychicZapEnabled = true;
	float PsychicZapMinRange = 4;
	float PsychicZapRange = 80;
	float PsychicZapCooldown = 60;

	string DamageInfo = "Damage multipliers against aliens: plasma bolts vs every other firearm.";
	float PlasmaDamageVsAliens = 2.0;
	float OtherFirearmDamageVsAliens = 0.75;

	string ItemsInfo = "Foil hat: aliens can't target the wearer beyond this many metres. Cooked alien meat: seconds of night vision. Raw/burnt/rotten alien meat: food-poisoning agents per bite.";
	float FoilHatDetectRange = 3;
	float AlienVisionSeconds = 90;
	float BadMeatPoison = 100;

	protected const static string PATH = "$profile:Gebs/alieninvasion.json";

	void Load()
	{
		bool changed = false;
		if ( FileExist(PATH) )
		{
			string error;
			if ( !JsonFileLoader<geb_AlienInvasionConfig>.LoadFile(PATH, this, error) )
			{
				Print("[AlienInvasion] alieninvasion.json failed to load; file preserved, using defaults for this session: " + error);
				return;
			}
			if ( ConfigVersion != VERSION_ALIENINVASION )
			{
				ConfigVersion = VERSION_ALIENINVASION;
				changed = true;
			}
		}
		else
		{
			ConfigVersion = VERSION_ALIENINVASION;
			changed = true;
		}

		Clamp();

		if ( changed )
		{
			MakeDirectory("$profile:Gebs");
			string saveError;
			if ( !JsonFileLoader<geb_AlienInvasionConfig>.SaveFile(PATH, this, saveError) )
				Print("[AlienInvasion] Could not write alieninvasion.json: " + saveError);
		}
	}

	// Keeps hand-edited values usable (min <= max, chances 0..1, nothing negative).
	protected void Clamp()
	{
		CrashAliensMin = Math.Max(CrashAliensMin, 0);
		CrashAliensMax = Math.Max(CrashAliensMax, CrashAliensMin);
		CrashAliensSpawnDistance = Math.Max(CrashAliensSpawnDistance, 0);
		CrashRifleChance = Math.Clamp(CrashRifleChance, 0, 1);
		CrashSpareCartridgesMax = Math.Max(CrashSpareCartridgesMax, 0);
		PsychicZapMinRange = Math.Max(PsychicZapMinRange, 0);
		PsychicZapRange = Math.Max(PsychicZapRange, PsychicZapMinRange);
		PsychicZapCooldown = Math.Max(PsychicZapCooldown, 1);
		PlasmaDamageVsAliens = Math.Max(PlasmaDamageVsAliens, 0);
		OtherFirearmDamageVsAliens = Math.Max(OtherFirearmDamageVsAliens, 0);
		FoilHatDetectRange = Math.Max(FoilHatDetectRange, 0);
		AlienVisionSeconds = Math.Max(AlienVisionSeconds, 0);
		BadMeatPoison = Math.Max(BadMeatPoison, 0);
	}
}

ref geb_AlienInvasionConfig g_geb_AlienInvasionConfig;

//! Loaded from disk on first use on the server; clients only ever see the defaults (and never need the values).
geb_AlienInvasionConfig GetAlienInvasionConfig()
{
	if ( !g_geb_AlienInvasionConfig )
	{
		g_geb_AlienInvasionConfig = new geb_AlienInvasionConfig();
		if ( GetGame() && GetGame().IsServer() )
			g_geb_AlienInvasionConfig.Load();
	}
	return g_geb_AlienInvasionConfig;
}
