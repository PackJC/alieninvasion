class CfgPatches
{
	class WeaponSounds
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
class CfgSoundShaders
{
	class Plasma_SoundShaderClose
	{
		samples[]=
		{
			
			{
				"alieninvasion\data\weapons\sounds\plasma1",
				1
			}
		};
		volume=0.5;
		range=50;
		rangeCurve="closeShotCurve";
	};
	class Plasma_SoundShaderMid
	{
		samples[]=
		{
			
			{
				"alieninvasion\data\weapons\sounds\plasma2",
				1
			}
		};
		volume=0.30000001;
		range=500;
		rangeCurve[]=
		{
			{0,0.2},
			{50,1},
			{200,0},
			{500,0}
		};
	};
	class Plasma_SoundShaderLong
	{
		samples[]=
		{
			
			{
				"alieninvasion\data\weapons\sounds\plasma3",
				1
			}
		};
		volume=0.1;
		range=1000;
		rangeCurve[]=
		{
			{0,0},
			{50,0},
			{400,1},
			{1000,1}
		};
	};
	
};
class CfgSoundSets
{

	class Rifle_Shot_Base_SoundSet;

	class Plasma_SoundSet: Rifle_Shot_Base_SoundSet
	{
		soundShaders[]=
		{
			"Plasma_SoundShaderClose",
			"Plasma_SoundShaderMid",
			"Plasma_SoundShaderLong"
		};
	};

};

