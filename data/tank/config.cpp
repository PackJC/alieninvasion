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
	// geb_AlienTank.c adds the bubbles, motes, vent wisps, green light and hum on clients.
	class geb_AlienTank: HouseNoDestruct
	{
		scope=2;
		displayName="$STR_geb_AlienTank";
		descriptionShort="$STR_geb_AlienTankDesc";
		model="\alieninvasion\data\tank\geb_alientank.p3d";
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
