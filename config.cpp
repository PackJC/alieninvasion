class CfgPatches
{
	class alieninvasion
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Characters",
			"DZ_Structures",
			"DZ_Weapons_Firearms",
			"DZ_Weapons_Firearms_Ruger1022",
			"DZ_Weapons_Ammunition",
			"DZ_Weapons_Magazines",
			"DZ_Weapons_Optics"


		};
	};
};
class CfgMods
{
	class alieninvasion
	{
		dir="alieninvasion";
		picture = "alieninvasion\data\logo.paa";					// picture in expanded description
		logoSmall = "alieninvasion\data\logo.paa";					// icon next to mod name when description is not expanded
		logo = "alieninvasion\data\logo.paa";						// logo below game menu
		logoOver = "alieninvasion\data\logo_hover.paa";				// logo hovered (CfgMods reads logoOver, not logoHover)
		action="";
		name="alieninvasion";
		// Without this the engine looks for alieninvasion/Anims/cfg/skeletons.anim.xml
		// and logs "ANIMATION (E): Can't load ..." on every start; point it at vanilla's.
		skeletonDefinitions="DZ/anims/cfg/skeletons.anim.xml";
		credits="Geb";
		author="Geb";
		authorID="0";
		version="2.0.0";
		extra=0;
		type="mod";
		dependencies[]=
		{
			"Game",
			"World",
			"Mission"
		};
		class defs
		{
			class gameScriptModule
			{
				value="";
				files[]=
				{
					"alieninvasion/scripts/3_game"
				};
			};
			class worldScriptModule
			{
				value="";
				files[]=
				{
					"alieninvasion/scripts/4_world"
				};
			};
			class missionScriptModule
			{
				value="";
				files[]=
				{
					"alieninvasion/scripts/5_mission"
				};
			};
		};
	};

};
