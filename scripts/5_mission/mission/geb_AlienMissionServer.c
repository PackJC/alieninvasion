modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();

		// Creates/updates $profile:Gebs/alieninvasion.json now rather than on the first alien encounter
		GetAlienInvasionConfig();

		// Economy files for server owners, written to $profile:Gebs/mpmissions/ (skipped when already current)
		geb_AlienInvasionFiles files = new geb_AlienInvasionFiles();
		files.GenerateAll();
	}
};
