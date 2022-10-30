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
		logoHover = "alieninvasion\data\logo_hover.paa";			// logo hovered
		action="";
		name="alieninvasion";
		credits="DayZ Modding Community";
		author="Geb";
		authorID="0";
		version="1.0";
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

		};
	};

};
