class CfgPatches
{
	class alieninvasion_tank
	{
		units[]=
		{
			"geb_AlienTank"
		};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Sounds_Effects"
		};
	};
};

class CfgVehicles
{
	class HouseNoDestruct;

	// Specimen tank: a sleeping alien in glowing green fluid. A prop for admins and map makers; nothing spawns it.
	// Model, textures and particles come from tools/alientank/build.py; scripts/4_world/entities/building/
	// geb_AlienTank.c adds the bubbles, motes, vent wisps, green light and hum on clients, animates the screen
	// and makes the alien drift.
	class geb_AlienTank: HouseNoDestruct
	{
		scope=2;
		displayName="$STR_geb_AlienTank";
		descriptionShort="$STR_geb_AlienTankDesc";
		model="\alieninvasion\data\tank\geb_alientank.p3d";
		// The console screen (model.cfg section "display"). geb_AlienTank.c steps it through geb_tank_screen_00_co
		// to _19_co near players. A hidden selection's faces only draw once it has a texture, so keep frame 00 here;
		// the glowing screen material keeps it lit.
		hiddenSelections[]=
		{
			"display"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\tank\geb_tank_screen_00_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\tank\geb_tank_screen.rvmat"
		};
		// The floating alien (model.cfg animations from tools/alientank/build.py FLOAT_*): geb_AlienTank.c moves
		// these near players. 0.5 is rest; animPeriod is set so each source keeps up with its 250 ms targets.
		class AnimationSources
		{
			class geb_bob
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.94;
			};
			class geb_roll
			{
				source="user";
				initPhase=0.5;
				animPeriod=3.04;
			};
			class geb_pitch
			{
				source="user";
				initPhase=0.5;
				animPeriod=2.49;
			};
			class geb_head
			{
				source="user";
				initPhase=0.5;
				animPeriod=2.21;
			};
			class geb_arm_l
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.74;
			};
			class geb_arm_r
			{
				source="user";
				initPhase=0.5;
				animPeriod=2.05;
			};
			class geb_armout_l
			{
				source="user";
				initPhase=0.5;
				animPeriod=2.38;
			};
			class geb_armout_r
			{
				source="user";
				initPhase=0.5;
				animPeriod=2.68;
			};
			class geb_forearm_l
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.55;
			};
			class geb_forearm_r
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.69;
			};
			class geb_leg_l
			{
				source="user";
				initPhase=0.5;
				animPeriod=2.19;
			};
			class geb_leg_r
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.91;
			};
			class geb_shin_l
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.63;
			};
			class geb_shin_r
			{
				source="user";
				initPhase=0.5;
				animPeriod=1.83;
			};
		};
	};
};

// Loops on the tank: a synthesised machine hum (tools/alientank/effects.py, about -12 LUFS) and vanilla's geyser
// bubbling (about -34 LUFS). Levelled against vanilla: the hum lands near the power generator up close (-24 vs
// -23 LUFS), the bubbling near vanilla's own geyser; both are at full strength within ~40% of their range.
class CfgSoundShaders
{
	class geb_AlienTankHum_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\tank\geb_tank_hum",
				1
			}
		};
		volume=0.25;
		range=15;
	};
	class geb_AlienTankBubbling_SoundShader
	{
		samples[]=
		{
			{
				"DZ\sounds\environment\water\Geyser_bubbling_loop",
				1
			}
		};
		volume=1;
		range=10;
	};
};

class CfgSoundSets
{
	class geb_AlienTankHum_SoundSet
	{
		soundShaders[]=
		{
			"geb_AlienTankHum_SoundShader"
		};
		volumeFactor=1;
		frequencyRandomizer=0;
		spatial=1;
		doppler=0;
		loop=1;
		volumeCurve="objectInner7VolumeCurve";
		distanceFilter="BaseCharacter_AttenuationFilter";
	};
	class geb_AlienTankBubbling_SoundSet: geb_AlienTankHum_SoundSet
	{
		soundShaders[]=
		{
			"geb_AlienTankBubbling_SoundShader"
		};
	};
};
