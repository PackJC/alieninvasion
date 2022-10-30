modded class CrashBase
{
	override static bool Init()
    {
        CrashSoundSets.RegisterSoundSet("AlienCrash_Distant_SoundSet");
        return true;
    }

};