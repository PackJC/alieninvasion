class CfgPatches
{
	class DZ_Sounds_Effects
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data"
		};
	};
};

class CfgSoundSets
{

	class AlienCrash_Distant_Base_SoundSet
	{
		volumeFactor=1;
		frequencyRandomizer=2;
		spatial=1;
		doppler=0;
		loop=0;
		volumeCurve="heliCrashDistantAttenuationCurve";
		distanceFilter="explosionDistanceFreqAttenuationFilter";
	};

	class AlienCrash_Distant_SoundSet: AlienCrash_Distant_Base_SoundSet
	{
		soundShaders[]=
		{
			"AlienCrash_Distant"
		};
		sound3DProcessingType="ThunderNear3DProcessingType";
	};

	class Alien_Calm_soundset
	{
		soundShaders[]=
		{
			"Alien_OnHit"
		};
		sound3DProcessingType="infected3DProcessingType";
		volumeCurve="infectedAttenuationCurve";
		spatial=1;
		doppler=0;
		loop=0;
	};

	class Alien_Attack_soundset
	{
		soundShaders[] =
		{
			"Alien_Attack"
		};
		sound3DProcessingType = "infected3DProcessingType";
		volumeCurve = "infectedAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class Alien_Agr_soundset
	{
		soundShaders[] =
		{
			"Alien_Agr"
		};
		sound3DProcessingType = "infected3DProcessingType";
		volumeCurve = "infectedAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class Alien_Soundset
	{
		soundShaders[] =
		{
			"Alien_Run"
		};
		sound3DProcessingType = "infected3DProcessingType";
		volumeCurve = "infectedAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};
	class Alien_Onhit_soundset
	{
		soundShaders[] =
		{
			"Alien_OnHit"
		};
		sound3DProcessingType = "infected3DProcessingType";
		volumeCurve = "infectedAttenuationCurve";
		spatial = 1;
		doppler = 0;
		loop = 0;
	};


};

class CfgSoundShaders
{

	class AlienCrash_Distant
	{
		samples[]=
		{
			
			{
				"alieninvasion\data\aliens\sounds\aliencrash_distant_1",
				1
			},
			
			{
				"alieninvasion\data\aliens\sounds\aliencrash_distant_2",
				1
			},
			
			{
				"alieninvasion\data\aliens\sounds\aliencrash_distant_3",
				1
			}
		};
		volume=1;
		range=3000;
	};

	class Alien_Attack
	{
		samples[]=
		{
			
			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			}
		};
		volume=0.30000001;
		range=44;
	};
	class Alien_Onhit
	{
		samples[]=
		{
			
			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},
			
			{
				"alieninvasion\data\aliens\sounds\alienscream",
				0.80000001
			},
			
			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},
			
			{
				"alieninvasion\data\aliens\sounds\alienscream",
				0.80000001
			}
		};
		volume=0.2;
		range=44;
	};


	class Alien_Agr
	{
		samples[] =
		{

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			}
		};
		volume = 1.4400001;
		range = 120;
	};
	class Alien_Calm
	{
		samples[] =
		{

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			}
		};
		volume = 0.40000001;
		range = 65;
	};
	class Alien_Run
	{
		samples[] =
		{

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\alienscream",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\alienscream",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\alienscream",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				0.80000001
			}
		};
		volume = 0.1;
		range = 33;
	};
};
