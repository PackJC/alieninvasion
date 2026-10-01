class CfgPatches
{
	// Was named DZ_Sounds_Effects, which is vanilla's own patch; reusing it merged into Bohemia's definition.
	class alieninvasion_sounds_aliens
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Sounds_Effects"
		};
	};
};

class CfgSoundSets
{
	class Zmb_VoiceFX_Base_SoundSet;

	// Infected voices are looked up as <script class>_<mind state>_SoundSet (InfectedSoundEventBase), so these
	// names must match the geb_GreenAlien / GreenAlienBase script classes.
	class geb_GreenAlien_CalmIdle_SoundSet: Zmb_VoiceFX_Base_SoundSet
	{
		soundShaders[]=
		{
			"Alien_Calm"
		};
		delay=5;
		delayRandomizer=3;
		startDelay=3;
		startDelayRandomizer=3;
	};
	class geb_GreenAlien_CalmMove_SoundSet: geb_GreenAlien_CalmIdle_SoundSet {};
	class geb_GreenAlien_DisturbedIdle_SoundSet: Zmb_VoiceFX_Base_SoundSet
	{
		soundShaders[]=
		{
			"Alien_Calm"
		};
		delay=3;
		delayRandomizer=2;
	};
	class geb_GreenAlien_AlertedIdle_SoundSet: Zmb_VoiceFX_Base_SoundSet
	{
		soundShaders[]=
		{
			"Alien_Agr"
		};
		delay=2;
		delayRandomizer=1;
	};
	class geb_GreenAlien_AlertedMove_SoundSet: geb_GreenAlien_AlertedIdle_SoundSet {};
	class geb_GreenAlien_ChaseMove_SoundSet: Zmb_VoiceFX_Base_SoundSet
	{
		soundShaders[]=
		{
			"Alien_Chase"
		};
		delay=2;
		delayRandomizer=1;
		startDelay=1;
		startDelayRandomizer=1;
	};

	// GreenAlienBase is spawnable on its own too
	class GreenAlienBase_CalmIdle_SoundSet: geb_GreenAlien_CalmIdle_SoundSet {};
	class GreenAlienBase_CalmMove_SoundSet: geb_GreenAlien_CalmMove_SoundSet {};
	class GreenAlienBase_DisturbedIdle_SoundSet: geb_GreenAlien_DisturbedIdle_SoundSet {};
	class GreenAlienBase_AlertedIdle_SoundSet: geb_GreenAlien_AlertedIdle_SoundSet {};
	class GreenAlienBase_AlertedMove_SoundSet: geb_GreenAlien_AlertedMove_SoundSet {};
	class GreenAlienBase_ChaseMove_SoundSet: geb_GreenAlien_ChaseMove_SoundSet {};

	// Played by script at the victim (psychic zap) and at a dying alien (geb_GreenAlien.c)
	class geb_AlienZap_SoundSet
	{
		soundShaders[]=
		{
			"geb_AlienZap_SoundShader"
		};
		sound3DProcessingType="infected3DProcessingType";
		volumeCurve="infectedAttenuationCurve";
		spatial=1;
		doppler=0;
		loop=0;
	};
	class geb_AlienDeath_SoundSet
	{
		soundShaders[]=
		{
			"geb_AlienDeath_SoundShader"
		};
		sound3DProcessingType="infected3DProcessingType";
		volumeCurve="infectedAttenuationCurve";
		spatial=1;
		doppler=0;
		loop=0;
	};

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
	class geb_AlienZap_SoundShader
	{
		samples[] =
		{

			{
				"alieninvasion\data\weapons\sounds\plasma2",
				1
			}
		};
		volume = 0.8;
		range = 60;
	};
	class geb_AlienDeath_SoundShader
	{
		samples[] =
		{

			{
				"alieninvasion\data\aliens\sounds\alienscream",
				1
			}
		};
		volume = 1;
		range = 80;
	};
	class Alien_Chase
	{
		samples[] =
		{

			{
				"alieninvasion\data\aliens\sounds\alienscream",
				1
			},

			{
				"alieninvasion\data\aliens\sounds\aliensound",
				1
			}
		};
		volume = 0.9;
		range = 90;
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
