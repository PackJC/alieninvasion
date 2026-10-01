modded class CrashBase
{
	// Registered next to vanilla's CrashBase.Init() rather than overriding it: overriding replaced vanilla's list,
	// so helicopter and sleigh crashes lost their distant crash sound on servers running this mod.
	static bool m_AlienCrashSoundsInit = RegisterAlienCrashSounds();

	static bool RegisterAlienCrashSounds()
	{
		CrashSoundSets.RegisterSoundSet("AlienCrash_Distant_SoundSet");
		return true;
	}
};
