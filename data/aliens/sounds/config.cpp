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

	// Alien voice: vanilla infected voice settings (distance filter, occlusion), pitched a little up
	class geb_Alien_VoiceFX_Base_SoundSet: Zmb_VoiceFX_Base_SoundSet
	{
		frequencyFactor = 1.08;
	};

	// Voice events from the animations (data/config.cpp geb_GreenAlienBase AnimEvents SoundVoice)
	class geb_Alien_LightHit_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_LightHit_SoundShader"
		};
	};
	class geb_Alien_HeavyHit_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_HeavyHit_SoundShader"
		};
	};
	class geb_Alien_Attack_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Attack_SoundShader"
		};
	};
	class geb_Alien_Jump_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Jump_SoundShader"
		};
	};
	class geb_Alien_Land_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Land_SoundShader"
		};
	};
	class geb_Alien_CallToArmsShort_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_CallToArmsShort_SoundShader"
		};
		volumeFactor = 1;
		volumeCurve = "infectedShoutingAttenuationCurve";
	};

	// Infected voices are looked up as <script class>_<mind state>_SoundSet (InfectedSoundEventBase), so these
	// names must match the geb_GreenAlien / geb_GreenAlienBase script classes. Delays as vanilla's skinny infected,
	// with more randomness so a group of aliens doesn't call in step.
	class geb_GreenAlien_CalmIdle_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Calm_SoundShader"
		};
		delay = 4;
		delayRandomizer = 3;
		startDelay = 2;
		startDelayRandomizer = 3;
	};
	class geb_GreenAlien_CalmMove_SoundSet: geb_GreenAlien_CalmIdle_SoundSet {};
	class geb_GreenAlien_DisturbedIdle_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Disturbed_SoundShader"
		};
	};
	class geb_GreenAlien_AlertedIdle_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Alerted_SoundShader"
		};
		delay = 2;
		delayRandomizer = 2;
	};
	class geb_GreenAlien_AlertedMove_SoundSet: geb_GreenAlien_AlertedIdle_SoundSet {};
	class geb_GreenAlien_ChaseMove_SoundSet: geb_Alien_VoiceFX_Base_SoundSet
	{
		soundShaders[] =
		{
			"geb_Alien_Chase_SoundShader"
		};
		delay = 2;
		delayRandomizer = 2;
		startDelay = 1;
		startDelayRandomizer = 2;
	};

	// geb_GreenAlienBase is spawnable on its own too (and runs the scope-1 alias GreenAlienBase's voice)
	class geb_GreenAlienBase_CalmIdle_SoundSet: geb_GreenAlien_CalmIdle_SoundSet {};
	class geb_GreenAlienBase_CalmMove_SoundSet: geb_GreenAlien_CalmMove_SoundSet {};
	class geb_GreenAlienBase_DisturbedIdle_SoundSet: geb_GreenAlien_DisturbedIdle_SoundSet {};
	class geb_GreenAlienBase_AlertedIdle_SoundSet: geb_GreenAlien_AlertedIdle_SoundSet {};
	class geb_GreenAlienBase_AlertedMove_SoundSet: geb_GreenAlien_AlertedMove_SoundSet {};
	class geb_GreenAlienBase_ChaseMove_SoundSet: geb_GreenAlien_ChaseMove_SoundSet {};

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
		distanceFilter="infectedDistanceFreqAttenuationFilter";
		spatial=1;
		doppler=0;
		loop=0;
	};

	// The distant crash sound (geb_Aliencrash.GetSoundSet, registered in the modded CrashBase); geb_ names since
	// 2.0.0 (AlienCrash_Distant* before)
	class geb_AlienCrash_Distant_Base_SoundSet
	{
		volumeFactor=1;
		frequencyRandomizer=2;
		spatial=1;
		doppler=0;
		loop=0;
		volumeCurve="heliCrashDistantAttenuationCurve";
		distanceFilter="explosionDistanceFreqAttenuationFilter";
	};

	class geb_AlienCrash_Distant_SoundSet: geb_AlienCrash_Distant_Base_SoundSet
	{
		soundShaders[]=
		{
			"geb_AlienCrash_Distant_SoundShader"
		};
		sound3DProcessingType="ThunderNear3DProcessingType";
	};
};

class CfgSoundShaders
{
	class geb_AlienCrash_Distant_SoundShader
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

	// Alien voice. The geb_alien_* takes are cut and varied from the mod's two original alien recordings by
	// tools/build_alien_sounds.py, levelled like vanilla's infected. Only alien takes: vanilla's skinny infected
	// voice used to be mixed in, and the aliens sounded like zombies (Cole, 2026-10-28).
	class geb_Alien_Calm_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_1", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_2", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_3", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_4", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_5", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_6", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_7", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calm_8", 0.125 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_Disturbed_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_1", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_2", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_3", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_4", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_5", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_6", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_disturbed_7", 0.1429 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_Alerted_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_1", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_2", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_3", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_4", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_5", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_6", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_7", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_alerted_8", 0.125 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_Chase_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_1", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_2", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_3", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_4", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_5", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_6", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_7", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_8", 0.1111 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_chase_9", 0.1111 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_Attack_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_1", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_2", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_3", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_4", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_5", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_6", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_7", 0.125 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_attack_8", 0.125 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_LightHit_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_1", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_2", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_3", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_4", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_5", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_6", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_lighthit_7", 0.1429 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_HeavyHit_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_1", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_2", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_3", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_4", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_5", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_6", 0.1429 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_heavyhit_7", 0.1429 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_Jump_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_jump_1", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_jump_2", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_jump_3", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_jump_4", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_jump_5", 0.2 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_Land_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_land_1", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_land_2", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_land_3", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_land_4", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_land_5", 0.2 }
		};
		volume = 1;
		range = 60;
	};
	class geb_Alien_CallToArmsShort_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calltoarms_1", 0.1667 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calltoarms_2", 0.1667 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calltoarms_3", 0.1667 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calltoarms_4", 0.1667 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calltoarms_5", 0.1667 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_calltoarms_6", 0.1667 }
		};
		volume = 1;
		range = 60;
	};

	class geb_AlienDeath_SoundShader
	{
		samples[] =
		{
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_death_1", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_death_2", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_death_3", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_death_4", 0.2 },
			{ "alieninvasion\data\aliens\sounds\voice\geb_alien_death_5", 0.2 }
		};
		volume = 1;
		range = 80;
	};
};
