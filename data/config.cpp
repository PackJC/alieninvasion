class CfgPatches
{
	// Separate from the root config's "alieninvasion" patch (same name merged the two); lists every vanilla
	// patch whose classes are inherited below so they always load first.
	class alieninvasion_data
	{
		units[]={};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Characters",
			"DZ_Characters_Zombies",
			"DZ_Gear_Food",
			"DZ_Gear_Consumables",
			"DZ_Characters_Backpacks",
			"DZ_Structures",
			"DZ_Weapons_Firearms",
			"DZ_Weapons_Firearms_Ruger1022",
			"DZ_Weapons_Magazines",
			"DZ_Weapons_Ammunition",
			"DZ_Weapons_Projectiles",
			"DZ_Weapons_Explosives",
			"DZ_Sounds_Effects"
		};
	};
};

class cfgVehicles
{
	//Inherited from classes
	class ZombieBase;
	class BearPelt;
	class TannedLeather;
	class LeatherSack_ColorBase;
	class FurCourierBag;
	class FurImprovisedBag;
	class Edible_Base;
	class FoodAnimationSources;
	class MeatStageTransitions;
	class CowSteakMeat;
	class HouseNoDestruct;
	class Clothing_Base;
	class ContaminatedArea_Local;
	class Grenade_Base;
	class Inventory_Base;
	class geb_FoilHat: Clothing_Base
	{
		displayName="$STR_geb_FoilHat";
		descriptionShort="$STR_geb_FoilHatDesc";
		scope=2;
		model = "\alieninvasion\data\clothes\geb_foilhat_g.p3d";
		repairableWithKits[]={2,3,5,8};
		repairCosts[]={25,25,25,25};
		rotationFlags=2;
		inventorySlot="Headgear";
		simulation="clothing";
		vehicleClass="Clothing";
		itemInfo[]=
		{
			"Clothing",
			"Headgear"
		};
		weight=60;
		itemSize[]={3,2};
		absorbency=0;
		heatIsolation=0.25;
		noMask=0;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=100;
					healthLevels[]=
					{
						
						{
							1,
							
							{
								"alieninvasion\data\clothes\foilhat_silver.rvmat",
								"alieninvasion\data\clothes\foilhat_silver.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"alieninvasion\data\clothes\foilhat_silver.rvmat",
								"alieninvasion\data\clothes\foilhat_silver.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"alieninvasion\data\clothes\foilhat_silver.rvmat",
								"alieninvasion\data\clothes\foilhat_silver.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"alieninvasion\data\clothes\foilhat_silver.rvmat",
								"alieninvasion\data\clothes\foilhat_silver.rvmat"
							}
						},
						
						{
							0,
							
							{
								"alieninvasion\data\clothes\foilhat_silver.rvmat",
								"alieninvasion\data\clothes\foilhat_silver.rvmat"
							}
						}
					};
				};
			};
			class GlobalArmor
			{
				class Projectile
				{
					class Health
					{
						damage=0.5;
					};
					class Blood
					{
						damage=0;
					};
					class Shock
					{
						damage=0.5;
					};
				};
				class Melee
				{
					class Health
					{
						damage=0.75;
					};
					class Blood
					{
						damage=0;
					};
					class Shock
					{
						damage=0.75;
					};
				};
				class Infected
				{
					class Health
					{
						damage=0.75;
					};
					class Blood
					{
						damage=0;
					};
					class Shock
					{
						damage=0.75;
					};
				};
				class FragGrenade
				{
					class Health
					{
						damage=0.5;
					};
					class Blood
					{
						damage=0;
					};
					class Shock
					{
						damage=0.5;
					};
				};
			};
		};
		class ClothingTypes
		{
		male = "\alieninvasion\data\clothes\geb_foilhat_m.p3d";
		female = "\alieninvasion\data\clothes\geb_foilhat_f.p3d";
		};
	};
	//Zombie Skeleton
	// Was GreenAlienBase until 2.0.0 (a name another mod could also use); the old name is a scope-1 alias below.
	class geb_GreenAlienBase : ZombieBase
	{
		// Scripts and the CE spawn geb_GreenAlien; scope 1 keeps this nameless base out of admin spawn lists
		scope = 1;
		model = "\alieninvasion\data\aliens\geb_GreenAlien.p3d";
		// The whole body is the body_m selection. Faces in a hidden selection are only drawn once it has a
		// texture, so these must stay set (ZombieBase's own "camo" entry has none and would hide the alien).
		hiddenSelections[] =
		{
			"body_m"
		};
		hiddenSelectionsTextures[] =
		{
			"\alieninvasion\data\aliens\newgreenalien_ca.paa"
		};
		hiddenSelectionsMaterials[] =
		{
			"\alieninvasion\data\aliens\greenalien.rvmat"
		};
		// The infected's animation setup with the alien's own set: geb_alien.asi is vanilla's infected_main.asi
		// with some lines on the alien's own clips (data/aliens/anims, written by tools/alienrig/game_anims.py);
		// the other lines still play vanilla's. The rest is ZombieBase's.
		class enfanimsys
		{
			meshObject = "dz\characters\zombies\z_hermit_m.xob";
			graphname = "dz\anims\workspaces\infected\infected_main\infected.agr";
			defaultinstance = "alieninvasion\data\aliens\anims\geb_alien.asi";
			skeletonName = "hermit_newbindpose.xob";
			startnode = "MasterControl";
		};

		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 250;
					healthLevels[] =
					{

						{
							1.01,
							{}
						},

						{
							0.69999999,
							{}
						},

						{
							0.5,
							{}
						},

						{
							0.30000001,
							{}
						},

						{
							0.0099999998,
							{}
						}
					};
				};
			};
			class DamageZones
			{
				class Head
				{
					class Health
					{
						hitpoints = 150;
						transferToGlobalCoef = 1;
					};
					class ArmorType
					{
						class Projectile
						{
							class Health
							{
								damage = 1.5;
							};
							class Shock
							{
								damage = 1.5;
							};
						};
						class FragGrenade
						{
							class Health
							{
								damage = 3;
							};
						};
					};
					componentNames[] =
					{
						"Head",
						"Neck"
					};
					fatalInjuryCoef = 0.1;
					inventorySlots[] =
					{
						"Headgear",
						"Mask"
					};
				};
				class Torso
				{
					class Health
					{
						hitpoints = 75;
						transferToGlobalCoef = 1;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					componentNames[] =
					{
						"Spine1",
						"Spine3"
					};
					fatalInjuryCoef = -1;
					inventorySlots[] =
					{
						"Vest",
						"Body",
						"Back"
					};
					inventorySlotsCoefs[] = { 1.1,1,0.5 };
				};
				class LeftArm
				{
					class Health
					{
						hitpoints = 50;
						transferToGlobalCoef = 1;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					componentNames[] =
					{
						"LeftArm",
						"LeftForeArm"
					};
					fatalInjuryCoef = -1;
				};
				class RightArm
				{
					class Health
					{
						hitpoints = 50;
						transferToGlobalCoef = 1;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					componentNames[] =
					{
						"RightArm",
						"RightForeArm"
					};
					fatalInjuryCoef = -1;
				};
				class LeftLeg
				{
					class Health
					{
						hitpoints = 50;
						transferToGlobalCoef = 0.33000001;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					componentNames[] =
					{
						"LeftLeg",
						"LeftUpLeg"
					};
					fatalInjuryCoef = -1;
					inventorySlots[] =
					{
						"Legs"
					};
				};
				class RightLeg
				{
					class Health
					{
						hitpoints = 50;
						transferToGlobalCoef = 0.33000001;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					componentNames[] =
					{
						"RightLeg",
						"RightUpLeg"
					};
					fatalInjuryCoef = -1;
					inventorySlots[] =
					{
						"Legs"
					};
				};
				class LeftFoot
				{
					class Health
					{
						hitpoints = 50;
						transferToGlobalCoef = 0.1;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					transferToZonesNames[] =
					{
						"LeftLeg"
					};
					transferToZonesCoefs[] = { 0.1 };
					componentNames[] =
					{
						"LeftFoot"
					};
					fatalInjuryCoef = -1;
					inventorySlots[] =
					{
						"Feet"
					};
				};
				class RightFoot
				{
					class Health
					{
						hitpoints = 50;
						transferToGlobalCoef = 0.1;
					};
					class ArmorType
					{
						class FragGrenade
						{
							class Health
							{
								damage = 2;
							};
						};
					};
					transferToZonesNames[] =
					{
						"RightLeg"
					};
					transferToZonesCoefs[] = { 0.1 };
					componentNames[] =
					{
						"RightFoot"
					};
					fatalInjuryCoef = -1;
					inventorySlots[] =
					{
						"Feet"
					};
				};
			};
		};
		// Barefoot like the patient infected the model is rigged from; vanilla's infected swing sounds; the alien
		// voice (data/aliens/sounds/config.cpp). geb_GreenAlien inherits all of it.
		class AnimEvents
		{
			class Steps
			{
				class Walk1
				{
					soundLookupTable = "walkErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 1;
				};
				class Walk2
				{
					soundLookupTable = "walkErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 2;
				};
				class Walk3
				{
					soundLookupTable = "walkErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 3;
				};
				class Walk4
				{
					soundLookupTable = "walkErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 4;
				};
				class Run1
				{
					soundLookupTable = "runErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 5;
				};
				class Run2
				{
					soundLookupTable = "runErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 6;
				};
				class Run3
				{
					soundLookupTable = "runErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 7;
				};
				class Run4
				{
					soundLookupTable = "runErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 8;
				};
				class Sprint1
				{
					soundLookupTable = "sprintErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 9;
				};
				class Sprint2
				{
					soundLookupTable = "sprintErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 10;
				};
				class Sprint3
				{
					soundLookupTable = "sprintErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 11;
				};
				class Sprint4
				{
					soundLookupTable = "sprintErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 12;
				};
				class Scuff1
				{
					soundLookupTable = "scuffErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 17;
				};
				class Scuff2
				{
					soundLookupTable = "scuffErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 18;
				};
				class Sccuff3
				{
					soundLookupTable = "scuffErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 19;
				};
				class Scuff4
				{
					soundLookupTable = "scuffErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 20;
				};
				class landFeetErc
				{
					soundLookupTable = "landFeetErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 21;
				};
				class landFootErc
				{
					soundLookupTable = "landFootErc_Bare_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 22;
				};
				class Bodyfall
				{
					soundLookupTable = "bodyfall_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 23;
				};
				class Bodyfall_Hand
				{
					soundLookupTable = "bodyfall_hand_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 24;
				};
				class Bodyfall_Slide
				{
					soundLookupTable = "bodyfall_slide_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 25;
				};
				class Prone_Walk_L
				{
					soundLookupTable = "walkProne_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 27;
				};
				class Prone_Walk_R
				{
					soundLookupTable = "walkProne_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 28;
				};
				class Prone_Run_L
				{
					soundLookupTable = "runProne_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 29;
				};
				class Prone_Run_R
				{
					soundLookupTable = "runProne_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 30;
				};
			};
			class Sounds
			{
				class Attack_Light1
				{
					soundSet = "Zmb_Attack_Light1_SoundSet";
					id = 1;
				};
				class Attack_Light2
				{
					soundSet = "Zmb_Attack_Light2_SoundSet";
					id = 2;
				};
				class Attack_Heavy1
				{
					soundSet = "Zmb_Attack_Heavy1_SoundSet";
					id = 3;
				};
				class Attack_Heavy2
				{
					soundSet = "Zmb_Attack_Heavy2_SoundSet";
					id = 4;
				};
				class TwoHands
				{
					soundSet = "Zmb_Attack_TwoHands_SoundSet";
					id = 5;
				};
			};
			class SoundVoice
			{
				class LightHit
				{
					soundSet = "geb_Alien_LightHit_SoundSet";
					id = 1;
				};
				class HeavyHit
				{
					soundSet = "geb_Alien_HeavyHit_SoundSet";
					id = 2;
				};
				class Attack
				{
					soundSet = "geb_Alien_Attack_SoundSet";
					id = 5;
				};
				class Jump
				{
					soundSet = "geb_Alien_Jump_SoundSet";
					id = 10;
				};
				class Land
				{
					soundSet = "geb_Alien_Land_SoundSet";
					id = 11;
				};
				class CallToArmsShort
				{
					soundSet = "geb_Alien_CallToArmsShort_SoundSet";
					id = 20;
				};
			};
		};
		class DamageApplied
		{
			type = "Projectile";
			dispersion = 0;
			bleedThreshold = 1;
			class Health
			{
				damage = 90;
			};
			class Blood
			{
				damage = 2000;
			};
			class Shock
			{
				damage = 100;
			};
		};
		// Vanilla infected melee (ZombieMaleBase). The long-range attack is the scripted psychic zap (geb_GreenAlien.c);
		// this block used to have 100 m range / 100 s cooldown, which landed invisible hits from across a field.
		class AttackActions
		{
			class AttackLong
			{
				attackName="attackLong";
				ammoType="MeleeInfectedLong";
				stanceName="erect";
				moveAnimNames[]=
				{
					"run",
					"sprint"
				};
				minDistance=0.75;
				distance=1.75;
				time=2.5;
				yawAngle=0;
				pitchAngle=0;
				attackWidth=2.2;
				repeatable=0;
				cooldown=2.75;
			};
			class AttackRun
			{
				attackName="attackRun";
				ammoType="MeleeInfected";
				stanceName="erect";
				moveAnimNames[]=
				{
					"run",
					"sprint"
				};
				minDistance=0.5;
				distance=1.3;
				time=1;
				yawAngle=0;
				pitchAngle=0;
				attackWidth=1.5;
				repeatable=1;
				cooldown=1.25;
			};
			class AttackShort
			{
				attackName="attackShort";
				ammoType="MeleeInfected";
				stanceName="erect";
				moveAnimNames[]=
				{
					"idle",
					"walk"
				};
				minDistance=0.5;
				distance=1.3;
				time=1;
				yawAngle=0;
				pitchAngle=0;
				attackWidth=1.5;
				repeatable=1;
				cooldown=1.25;
			};
			class AttackShortLow
			{
				attackName="attackShortLow";
				ammoType="MeleeInfected";
				stanceName="erect";
				moveAnimNames[]=
				{
					"idle",
					"walk",
					"run"
				};
				minDistance=0;
				distance=2;
				time=1;
				yawAngle=0;
				pitchAngle=-45;
				attackWidth=2;
				repeatable=1;
				cooldown=1.25;
			};
			class CrawlAttackMove
			{
				attackName="crawlAttackMove";
				ammoType="MeleeInfected";
				stanceName="crawl";
				moveAnimNames[]=
				{
					"walk"
				};
				distance=2;
				time=1.5;
				yawAngle=0;
				pitchAngle=45;
				attackWidth=2;
				cooldown=1.25;
			};
			class CrawlAttackStill
			{
				attackName="crawlAttackStill";
				ammoType="MeleeInfected";
				stanceName="crawl";
				moveAnimNames[]=
				{
					"idle"
				};
				distance=2;
				time=1.1;
				yawAngle=0;
				pitchAngle=45;
				attackWidth=2;
				cooldown=1.25;
			};
		};
		class Skinning
		{
			class ObtainedSteaks
			{
				item = "geb_GreenAlienMeat";
				count = 2;
				quantityMinMaxCoef[] = { 0.5,1 };
			};
			class ObtainedGuts
			{
				item = "SmallGuts";
				count = 1;
				quantityMinMaxCoef[] = { 0.5,0.80000001 };
			};
			class ObtainedBones
			{
				item = "Bone";
				count = 1;
				quantityMinMaxCoef[] = { 0.2,0.30000001 };
				transferToolDamageCoef = 1;
			};
			// The hide's condition is the average health of these zones (ActionSkinning, quantityCoef > 0), so they
			// must be zones this body has (DamageZones above): animals' Zone_Chest/Zone_Belly don't exist here.
			class ObtainedPelt
			{
				item="geb_GreenAlienSkin";
				count=1;
				itemZones[]=
				{
					"Torso"
				};
				quantityCoef=1;
				transferToolDamageCoef=1;
			};
		};
	}

	//Zombie Skeleton
	class geb_GreenAlien : geb_GreenAlienBase
	{
		displayName = "$STR_GreenAlien";
		descriptionShort = "$STR_GreenAlienDesc";
		scope = 2;

		class Cargo
		{
				itemsCargoSize[] = { 10,15 };
				allowOwnedCargoManipulation = 1;
				openable = 0;
		};
		// Inherits the base's damage zones (head, torso, limbs) and health levels; only the global hitpoints
		// differ. Without the ": DamageSystem" bases this class replaced the whole damage system and lost the zones.
		class DamageSystem: DamageSystem
		{
				class GlobalHealth: GlobalHealth
				{
					class Health: Health
					{
						hitpoints = 150;
					};
				};
		};
	}

	// The base's name until 2.0.0, kept so anything still naming it (admin presets, old event files) finds a class
	class GreenAlienBase: geb_GreenAlienBase
	{
		scope=1;
	};
	
	//Cow Steak
	class geb_GreenAlienMeat : CowSteakMeat
	{
		scope = 2;
		displayName = "$STR_GreenAlienMeat";
		descriptionShort = "$STR_GreenAlienMeatDesc";
		hiddenSelections[] =
		{
			"cs_raw"
		};
		// One per food stage, like vanilla's: raw, baked, boiled, dried, burnt
		hiddenSelectionsTextures[] =
		{
			"\alieninvasion\data\aliens\alienmeat.paa",
			"\alieninvasion\data\aliens\alienmeat_baked_co.paa",
			"\alieninvasion\data\aliens\alienmeat_boiled_co.paa",
			"\alieninvasion\data\aliens\alienmeat_dried_co.paa",
			"\alieninvasion\data\aliens\alienmeat_burnt_co.paa"
		};
		hiddenSelectionsMaterials[] =
		{
			"dz\gear\food\data\meat_steak_raw.rvmat",
			"dz\gear\food\data\meat_steak_baked.rvmat",
			"dz\gear\food\data\meat_steak_boiled.rvmat",
			"dz\gear\food\data\meat_steak_dried.rvmat",
			"dz\gear\food\data\meat_steak_burnt.rvmat",
			"dz\gear\food\data\meat_steak_rotten.rvmat"
		};
	}

	//Bear Pelt
	// The bear pelt's model and texture slot, but a wolf pelt's size and yield: a bear hide's 15 kg, 8x4 and 12
	// leather per hide was far too much from a 1.9 m alien, with 10-15 of them at every crash.
	class geb_GreenAlienSkin: BearPelt
	{
		scope=2;
		displayName="$STR_GreenAlienSkin";
		descriptionShort="$STR_GreenAlienSkinDesc";
		weight=6000;
		itemSize[]={5,3};
		peltGain=6;
		leatherYield=4;
		hiddenSelections[]=
		{
			"Camo"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\aliens\alienpelt.paa"
		};
	}

	// Roswell Hide crafting chain: green versions of vanilla's tanned leather, leather backpack, fur courier
	// bag and fur backpack, on vanilla's models. Recipes are in scripts/4_world/classes/recipes/recipes/.
	class geb_GreenAlienLeather: TannedLeather
	{
		scope=2;
		displayName="$STR_geb_GreenAlienLeather";
		descriptionShort="$STR_geb_GreenAlienLeatherDesc";
		// tanned_leather.p3d has no camo selection, but its "zbytek" (whole model) is a texture section
		hiddenSelections[]=
		{
			"zbytek"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\leather\geb_alienleather_co.paa"
		};
	};
	// Extends the colour base, not LeatherSack_Natural: barrels "dye" anything Natural into class names
	// that no longer exist, which would delete it.
	class geb_GreenAlienLeatherSack: LeatherSack_ColorBase
	{
		scope=2;
		displayName="$STR_geb_GreenAlienLeatherSack";
		descriptionShort="$STR_geb_GreenAlienLeatherSackDesc";
		visibilityModifier=0.80000001;
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\leather\geb_alienleathersack_g_co.paa",
			"\alieninvasion\data\leather\geb_alienleathersack_co.paa",
			"\alieninvasion\data\leather\geb_alienleathersack_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"dz\characters\backpacks\data\Bag_LeatherSack_natural.rvmat",
			"dz\characters\backpacks\data\Bag_LeatherSack_natural.rvmat",
			"dz\characters\backpacks\data\Bag_LeatherSack_natural.rvmat"
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=200;
					transferToAttachmentsCoef=0.5;
					healthLevels[]=
					{
						{
							1,
							{
								"DZ\characters\backpacks\data\Bag_LeatherSack_natural.rvmat",
								"dz\characters\backpacks\data\Bag_LeatherSack_g_natural.rvmat"
							}
						},
						{
							0.69999999,
							{}
						},
						{
							0.5,
							{
								"DZ\characters\backpacks\Data\Bag_LeatherSack_natural_damage.rvmat",
								"dz\characters\backpacks\data\Bag_LeatherSack_g_natural_damage.rvmat"
							}
						},
						{
							0.30000001,
							{}
						},
						{
							0,
							{
								"DZ\characters\backpacks\Data\Bag_LeatherSack_natural_destruct.rvmat",
								"dz\characters\backpacks\data\Bag_LeatherSack_g_natural_destruct.rvmat"
							}
						}
					};
				};
			};
		};
	};
	class geb_GreenAlienCourierBag: FurCourierBag
	{
		scope=2;
		displayName="$STR_geb_GreenAlienCourierBag";
		descriptionShort="$STR_geb_GreenAlienCourierBagDesc";
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\leather\geb_aliencourierbag_co.paa",
			"\alieninvasion\data\leather\geb_aliencourierbag_co.paa",
			"\alieninvasion\data\leather\geb_aliencourierbag_co.paa"
		};
	};
	class geb_GreenAlienImprovisedBag: FurImprovisedBag
	{
		scope=2;
		displayName="$STR_geb_GreenAlienImprovisedBag";
		descriptionShort="$STR_geb_GreenAlienImprovisedBagDesc";
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\leather\geb_alienimprovisedbag_co.paa",
			"\alieninvasion\data\leather\geb_alienimprovisedbag_co.paa",
			"\alieninvasion\data\leather\geb_alienimprovisedbag_co.paa"
		};
	};

	// The Montauk rifle's turning fluid (tools/plasmarifle/fluid.py): the purple vent tube cores and the lime drum,
	// rings, disc and vials, an item of its own in the rifle's hidden slot geb_PlasmaRifleFluid, drawn at the rifle's
	// proxy. Weapons don't run model.cfg's clock animations on their own model; an item does, so the fluid turns here
	// (data/weapons/model.cfg class geb_plasmarifle_fluid), the way vanilla keeps the Magnum's turning cylinder an item
	// (Magnum_Cylinder). geb_PlasmaRifle.c gives every rifle one; nothing spawns it on its own.
	class geb_PlasmaRifleFluid: Inventory_Base
	{
		// scope 1: scripts can create it (the rifle does), but admin tools' spawn lists and the CE leave it out
		scope=1;
		model="\alieninvasion\data\weapons\geb_plasmarifle_fluid.p3d";
		weight=0;
		itemSize[]={1,1};
		inventorySlot[]=
		{
			"geb_PlasmaRifleFluid"
		};
	};

	// Plasma grenade (model: tools/plasmagrenade/build.py; script: geb_PlasmaGrenade.c). Unpinned and thrown, it
	// sticks to whatever it hits (ground, walls, cars, players, infected, animals) and bursts 3 s after leaving the hand.
	class geb_PlasmaGrenade: Grenade_Base
	{
		scope=2;
		displayName="$STR_geb_PlasmaGrenade";
		descriptionShort="$STR_geb_PlasmaGrenadeDesc";
		model="\alieninvasion\data\plasmagrenade\geb_plasmagrenade.p3d";
		ammoType="geb_PlasmaGrenade_Ammo";
		weight=400;
		// The glowing slits (magenta) and pin lights (cyan), model.cfg sections: the script gives them the
		// blue-white _armed texture and the brighter _light_armed material while the grenade is armed
		hiddenSelections[]=
		{
			"lights"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\plasmagrenade\geb_plasmagrenade_lights_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\plasmagrenade\geb_plasmagrenade_light.rvmat"
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=50;
					// the alloy shell wears; the lights don't
					healthLevels[]=
					{
						{
							1,
							{
								"alieninvasion\data\plasmagrenade\geb_plasmagrenade_shell.rvmat"
							}
						},
						{
							0.69999999,
							{
								"alieninvasion\data\plasmagrenade\geb_plasmagrenade_shell.rvmat"
							}
						},
						{
							0.5,
							{
								"alieninvasion\data\plasmagrenade\geb_plasmagrenade_shell_damage.rvmat"
							}
						},
						{
							0.30000001,
							{
								"alieninvasion\data\plasmagrenade\geb_plasmagrenade_shell_damage.rvmat"
							}
						},
						{
							0,
							{
								"alieninvasion\data\plasmagrenade\geb_plasmagrenade_shell_destruct.rvmat"
							}
						}
					};
				};
			};
		};
		class AnimEvents
		{
			class SoundWeapon
			{
				class Grenade_unpin
				{
					soundSet="Grenade_unpin_SoundSet";
					id=201;
				};
				class turnOnRadio_Pin
				{
					soundSet="Grenade_pin_SoundSet";
					id=1006;
				};
			};
		};
	};
	
	//House
	class geb_Aliencrash: HouseNoDestruct
	{
		scope=2;
		storageCategory=4;
		model = "\alieninvasion\data\ufocrash\crash.p3d";

	};

	//House
	class StaticObj_geb_Aliencrash: HouseNoDestruct
	{
		scope=2;
		model = "\alieninvasion\data\ufocrash\crash.p3d";

	};

	//Toxic cloud around a UFO crash (script: geb_AlienRadiationArea)
	class geb_AlienRadiationArea: ContaminatedArea_Local
	{
		scope=2;
	};
};

//Weapons
class cfgWeapons
{
	class Mode_Safe;
	class Mode_SemiAuto;
	class OpticsInfoRifle;

	class Ruger1022;
	class geb_PlasmaRifle_Base: Ruger1022
	{
		scope=0;
		// A rifle's weight (the 10/22 is 2300 g); it was 350 g until 2.0.0, then 3000 g, and 3400 g with the
		// shoulder stock (the Geometry LOD's mass, tools/fix_models.py, says the same)
		weight=3400;
		absorbency=0;
		// Not the Ruger's wrap, scope and suppressor slots: this model has no proxies to show them on. Its one slot is
		// the hidden one holding its turning fluid (geb_PlasmaRifleFluid, tools/plasmarifle/fluid.py).
		attachments[]=
		{
			"geb_PlasmaRifleFluid"
		};
		repairableWithKits[]={0};
		PPDOFProperties[]={1,0.75,40,180,4,10};
		// The Ruger 10/22's own length: with the stock (tools/plasmarifle/model.py) the rifle reaches from the Ruger's
		// muzzle to its butt like the 10/22 does, so it lowers near walls at the same distance. It was 0.6.
		WeaponLength=0.95;
		barrelArmor=1.25;
		initSpeedMultiplier=1.05;
		chamberSize=1;
		chamberedRound="";
		value=0;
		chamberableFrom[]=
		{
			""
		};
		// One cartridge per plasma colour; each cell's ammo picks the muzzle effect and bolt (cfgAmmo below)
		magazines[]=
		{
			"geb_PlasmaCartridge",
			"geb_PlasmaCartridge_Arc",
			"geb_PlasmaCartridge_Lance",
			"geb_PlasmaCartridge_Void"
		};
		magazineSwitchTime=0.65;
		ejectType=1;
		recoilModifier[]={1,1,1};
		swayModifier[]={1.2,1.2,1};
		// This custom P3D has no hide_barrel selection. Registering the Ruger selection here makes
		// ForceSyncSelectionState access an invalid model selection when a cartridge is chambered.
		simpleHiddenSelections[]={};
		// The plasma relay's clack and sputter (tools/plasmarifle/sounds.py), not the M4's click
		drySound[]=
		{
			"alieninvasion\data\weapons\sounds\geb_plasma_dry",
			0.5,
			1,
			20
		};
		hiddenSelections[]={};
		modes[]=
		{
			"SemiAuto"
		};
		// The Ruger animations' handling sound events (ids from DZ\characters SurvivorBase AnimEvents), played as
		// the plasma rifle's own sounds for this rifle only, none of vanilla's foley: vanilla looks an event up on
		// the item in hands first (DayZPlayerImplement.ProcessWeaponEvent), so the Ruger 10/22 keeps its own. All
		// of the rifle's events are here but the hands' (hand 1385, weapon_down 1386); the eighteen jam sounds
		// share six kinds. data/weapons/sounds/config.cpp has the sets, tools/plasmarifle/sounds.py the sounds.
		class AnimEvents
		{
			class SoundWeapon
			{
				class geb_Plasma_charge_open
				{
					soundSet="geb_Plasma_charge_open_SoundSet";
					id=1373;
				};
				class geb_Plasma_charge_open_release
				{
					soundSet="geb_Plasma_charge_open_release_SoundSet";
					id=1374;
				};
				class geb_Plasma_charge_close
				{
					soundSet="geb_Plasma_charge_close_SoundSet";
					id=1375;
				};
				class geb_Plasma_charge_close_release
				{
					soundSet="geb_Plasma_charge_close_release_SoundSet";
					id=1376;
				};
				class geb_Plasma_chamber_load
				{
					soundSet="geb_Plasma_chamber_load_SoundSet";
					id=1377;
				};
				class geb_Plasma_mag_in
				{
					soundSet="geb_Plasma_mag_in_SoundSet";
					id=1378;
				};
				class geb_Plasma_mag_load
				{
					soundSet="geb_Plasma_mag_load_SoundSet";
					id=1379;
				};
				class geb_Plasma_mag_out
				{
					soundSet="geb_Plasma_mag_out_SoundSet";
					id=1380;
				};
				class geb_Plasma_mag_remove
				{
					soundSet="geb_Plasma_mag_remove_SoundSet";
					id=1381;
				};
				class geb_Plasma_dry
				{
					soundSet="geb_Plasma_dry_SoundSet";
					id=1382;
				};
				class geb_Plasma_pullout
				{
					soundSet="geb_Plasma_pullout_SoundSet";
					id=1383;
				};
				class geb_Plasma_chamber_load2
				{
					soundSet="geb_Plasma_chamber_load_SoundSet";
					id=1384;
				};
				class geb_Plasma_jamming1
				{
					soundSet="geb_Plasma_fault_tick_SoundSet";
					id=1387;
				};
				class geb_Plasma_jamming2
				{
					soundSet="geb_Plasma_fault_strain_SoundSet";
					id=1388;
				};
				class geb_Plasma_jamming3
				{
					soundSet="geb_Plasma_fault_soft_SoundSet";
					id=1389;
				};
				class geb_Plasma_jamming4
				{
					soundSet="geb_Plasma_fault_clear_SoundSet";
					id=1390;
				};
				class geb_Plasma_jamming5
				{
					soundSet="geb_Plasma_fault_soft_SoundSet";
					id=1391;
				};
				class geb_Plasma_jamming6
				{
					soundSet="geb_Plasma_fault_soft_SoundSet";
					id=1392;
				};
				class geb_Plasma_jamming7
				{
					soundSet="geb_Plasma_fault_long_SoundSet";
					id=1393;
				};
				class geb_Plasma_jamming8
				{
					soundSet="geb_Plasma_fault_long_SoundSet";
					id=1394;
				};
				class geb_Plasma_jamming9
				{
					soundSet="geb_Plasma_fault_long_SoundSet";
					id=1395;
				};
				class geb_Plasma_jamming10
				{
					soundSet="geb_Plasma_fault_tick_SoundSet";
					id=1396;
				};
				class geb_Plasma_jam
				{
					soundSet="geb_Plasma_fault_snap_SoundSet";
					id=1397;
				};
				class geb_Plasma_jamming_01
				{
					soundSet="geb_Plasma_fault_tick_SoundSet";
					id=3512;
				};
				class geb_Plasma_jamming_02
				{
					soundSet="geb_Plasma_fault_strain_SoundSet";
					id=3513;
				};
				class geb_Plasma_jamming_03
				{
					soundSet="geb_Plasma_fault_clear_SoundSet";
					id=3514;
				};
				class geb_Plasma_jamming_04
				{
					soundSet="geb_Plasma_fault_soft_SoundSet";
					id=3515;
				};
				class geb_Plasma_jamming_05
				{
					soundSet="geb_Plasma_fault_soft_SoundSet";
					id=3516;
				};
				class geb_Plasma_jamming_06
				{
					soundSet="geb_Plasma_fault_snap_SoundSet";
					id=3517;
				};
				class geb_Plasma_jamming_07
				{
					soundSet="geb_Plasma_fault_long_SoundSet";
					id=3518;
				};
			};
		};
		class SemiAuto: Mode_SemiAuto
		{
			// Built like vanilla's rifles: the shot (close / mid / distant / mechanism), its interior version, and
			// its echo in the forest, meadows, houses, indoors and far off, in the slots of vanilla's SKS tails but
			// the plasma's own (until 2.0.0's last change it borrowed the SKS's, a gunshot's echo)
			soundSetShot[]=
			{
				"geb_Plasma_Shot_SoundSet",
				"geb_Plasma_Shot_Interior_SoundSet",
				"geb_Plasma_Tail_SoundSet",
				"geb_Plasma_InteriorTail_SoundSet",
				"geb_Plasma_Tail_2D_SoundSet"
			};
			reloadTime=0.125;
			recoil="recoil_mp5";
			recoilProne="recoil_mp5_prone";
			dispersion=0.0020000001;
			magazineSlot="magazine";
		};
		class OpticsInfo: OpticsInfoRifle
		{
			memoryPointCamera="eye";
			discreteDistance[]={50,100,150,200,250,300};
			discreteDistanceInitIndex=2;
			modelOptics="-";
			distanceZoomMin=50;
			distanceZoomMax=300;
		};
	};
	class geb_PlasmaRifle: geb_PlasmaRifle_Base
	{
		scope=2;
		displayName="$STR_geb_PlasmaRifle";
		descriptionShort="$STR_geb_PlasmaRifleDesc";
		model = "\alieninvasion\data\weapons\geb_plasmarifle.p3d";
		// 0.96 m long with the stock (it was 0.73 m and 5 x 3); the 0.95 m Ruger 10/22 is 9 x 3
		itemSize[]={8,3};
		spawnDamageRange[]={0,0.6};
		// The rear screen (model section "screen", tools/plasmarifle/screen.py). geb_PlasmaRifle.c steps it through
		// geb_plasmarifle_screen_00_co .. _15_co while the rifle is held or lying near the camera. A hidden
		// selection's faces only draw once it has a texture, so frame 00 is set here; the glowing screen material
		// keeps it lit. Both arrays are set, or Ruger1022's camo texture and material would land on index 0.
		hiddenSelections[]=
		{
			"screen"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\weapons\geb_plasmarifle_screen_00_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\weapons\geb_plasmarifle_screen.rvmat"
		};
		class Particles
		{
			class OnFire
			{
				class MuzzleFlash
				{
					// Fallback only: the fired cell's muzzleFlashParticle wins (WeaponParticlesBase.CheckParticleOverride)
					overrideParticle="geb_plasma_ionburst";
					// No vanilla light: MuzzleFlashLight is orange. geb_PlasmaRifle.c (EEFired) lights each shot
					// in the cell's geb_muzzleLightColor instead.
					illuminateWorld=0;
				};
			};
			// Blue-white arcs crackle on the muzzle after a quick string of shots and die off as it cools
			// (tools/plasmashots/shots.py; looping effects that vanilla's overheating system starts, restarts and
			// stops). Every shot adds 1 and restarts the decay timer; each overheatingDecayInterval without a shot
			// takes off 1 + value / maxOverheatingValue. The arcs show from 6 shots fired less than 0.6 s apart,
			// light up to a coefficient ((value - 6) / 18) of 0.3, heavy above it; the value tops out at 18
			// (coefficient 0.67). After a full cartridge emptied quickly they crackle on for about 4 s; steady aimed
			// fire never heats the rifle. Every child class here is an effect entry, so there's no shared base class.
			// geb_PlasmaRifle.c keeps the ammo's muzzleFlashParticle out of these entries.
			class OnOverheating
			{
				maxOverheatingValue=18;
				shotsToStartOverheating=6;
				overheatingDecayInterval=0.6;
				class ArcsLight
				{
					overrideParticle="geb_plasma_overheat";
					positionOffset[]={0.02,0,0};
					onlyWithinOverheatLimits[]={0,0.3};
				};
				class ArcsHeavy
				{
					overrideParticle="geb_plasma_overheat_heavy";
					positionOffset[]={0.02,0,0};
					onlyWithinOverheatLimits[]={0.3,1};
				};
			};
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=100;
					// Wear like vanilla weapons (Ruger1022): body and stock materials swap to copies with vanilla's
					// weapon damage overlay, then the destruct one, the glow dimming (tools/plasmarifle/texture.py)
					healthLevels[]=
					{
						{1.0,{"alieninvasion\data\weapons\plasma.rvmat","alieninvasion\data\weapons\geb_plasmarifle_stock.rvmat"}},
						{0.7,{"alieninvasion\data\weapons\plasma.rvmat","alieninvasion\data\weapons\geb_plasmarifle_stock.rvmat"}},
						{0.5,{"alieninvasion\data\weapons\plasma_damage.rvmat","alieninvasion\data\weapons\geb_plasmarifle_stock_damage.rvmat"}},
						{0.3,{"alieninvasion\data\weapons\plasma_damage.rvmat","alieninvasion\data\weapons\geb_plasmarifle_stock_damage.rvmat"}},
						{0.0,{"alieninvasion\data\weapons\plasma_destruct.rvmat","alieninvasion\data\weapons\geb_plasmarifle_stock_destruct.rvmat"}}
					};
				};
			};
		};
	};
};


//Ammo Types
// The engine registers each entry by its name= (the cfgAmmo class); the class names are only keys, so they carry
// the geb_ prefix (AType_Plasma_Cell* until 2.0.0). The cfgAmmo names themselves stay: magazines and chambers
// store them per round.
class cfgAmmoTypes
{
	class geb_AType_Plasma_Cell
	{
		name="Plasma_Cell";
	};
	class geb_AType_Plasma_Cell_Arc
	{
		name="Plasma_Cell_Arc";
	};
	class geb_AType_Plasma_Cell_Lance
	{
		name="Plasma_Cell_Lance";
	};
	class geb_AType_Plasma_Cell_Void
	{
		name="Plasma_Cell_Void";
	};
};

//Ammo
class cfgAmmo
{
	class Bullet_Base;
	class RGD5Grenade_Ammo;
	// The plasma grenade's burst (geb_PlasmaGrenade): smaller than the M67's (4 m against 5) but twice its damage,
	// with a blue plasma blast (tools/plasmagrenade/effects.py) and its own plasma explosion sound (CfgSoundSets
	// below) over vanilla's grenade echo.
	class geb_PlasmaGrenade_Ammo: RGD5Grenade_Ammo
	{
		indirectHitRange=4;
		particle="geb_plasmagrenade_blast";
		soundSetExplosion[]=
		{
			"geb_PlasmaGrenade_explosion_SoundSet",
			"Grenade_Tail_SoundSet"
		};
		class DamageApplied
		{
			type="FragGrenade";
			class Health
			{
				damage=100;
			};
			class Blood
			{
				damage=0;
			};
			class Shock
			{
				damage=0;
			};
		};
	};
	class Plasma_Cell: Bullet_Base
	{
		scope=2;
		lootCategory="Crafted";
		casing="";
		// Must name a real round: the engine's EffectBulletShow builds the chambered round from it with no null
		// check, so round="" crashed the server whenever a chambered plasma cell was shown (COT/admin spawns with
		// ammo, weapons loaded from storage). The rifle model has no chamber proxy, so this round is never seen.
		round="FxRound_22";
		// The tracer: a small 3D plasma ball (tools/plasmashots/tracers.py) in the cell's colour on the bullet itself,
		// every shot (tracersEvery on the cartridge). At 360 m/s it only blinks along the real hit line; the ball you
		// see is geb_laserParticle below. It was vanilla's flat green tracer until 2.0.0.
		model="\alieninvasion\data\weapons\geb_plasmaball_green.p3d";
		// The loose cell (cfgMagazines) a chambered cell goes back into when the rifle is cleared. It named a class
		// that didn't exist until 2.0.0, so a cleared cell was lost with a "cannot store cartridge" error.
		spawnPileType="Ammo_Plasma_Cell";
		// Muzzle effects are generated by tools/plasmashots/shots.py; this one beats the rifle's overrideParticle
		muzzleFlashParticle="geb_plasma_ionburst";
		// Muzzle light colour (0..1) for this cell's shots, read by the rifle script instead of vanilla's orange light
		geb_muzzleLightColor[]={0.3,1,0.4};
		// The visible plasma ball in this cell's colour, 90 m/s for 90 m, played by geb_PlasmaRifle.c on every shot
		// (tools/plasmashots/shots.py plasma_ball). It was the original laser streak until 2.0.0.
		geb_laserParticle="geb_plasma_laser_ionburst";
        hit=12;
        indirectHit=0;
        indirectHitRange=0;
        visibleFire=22;
        audibleFire=22;
        visibleFireTime=3;
        cost=1.2;
        airLock=1;
        typicalSpeed=360;
        tracerScale=1.5;
        tracerStartTime=0;
        tracerEndTime=3;
        caliber=1;
        airFriction=-0.00018;
        damageBarrel=6;
        damageBarrelDestroyed=60;
        initSpeed=360;
        weight=0.0099999998;
		class DamageApplied
		{
			type="Projectile";
			dispersion=1;
			bleedThreshold=1;
			defaultDamageOverride[]=
			{
				{0.5,1}
			};
			class Health
			{
				damage=100;
			};
			class Blood
			{
				damage=100;
			};
			class Shock
			{
				damage=100;
			};
		};
		class NoiseHit
		{
			strength=80;
			type="shot";
		};
	};
	// The other cell colours: same round, own muzzle effect, light and bolt (a plasma ball in the cell's colour,
	// tools/plasmashots/tracers.py). Scripts test IsKindOf(ammo, "Plasma_Cell").
	// The blue cell fires rings like a ray gun: its bolt is a glowing ring round the flight line and its visible
	// shot a ring with a stream of smaller rings behind it (tracers.py ring_lod, shots.py plasma_rings).
	class Plasma_Cell_Arc: Plasma_Cell
	{
		model="\alieninvasion\data\weapons\geb_plasmaring_blue.p3d";
		muzzleFlashParticle="geb_plasma_arc";
		geb_muzzleLightColor[]={0.3,0.7,1};
		geb_laserParticle="geb_plasma_laser_arc";
	};
	class Plasma_Cell_Lance: Plasma_Cell
	{
		model="\alieninvasion\data\weapons\geb_plasmaball_lime.p3d";
		muzzleFlashParticle="geb_plasma_lance";
		geb_muzzleLightColor[]={0.7,1,0.15};
		geb_laserParticle="geb_plasma_laser_lance";
	};
	// The violet cell shoots lightning: geb_PlasmaRifle.c draws a jagged bolt from the muzzle to what the shot hits
	// (geb_boltParticle segments, geb_boltHitParticle where it lands); its laser is only the muzzle puff and fumes.
	class Plasma_Cell_Void: Plasma_Cell
	{
		model="\alieninvasion\data\weapons\geb_plasmaball_violet.p3d";
		muzzleFlashParticle="geb_plasma_void";
		geb_muzzleLightColor[]={0.7,0.3,1};
		geb_laserParticle="geb_plasma_laser_void";
		geb_boltParticle="geb_plasma_bolt_void";
		geb_boltHitParticle="geb_plasma_bolt_void_hit";
	};
};

//Magazines
class cfgMagazines
{
	class Mag_AKM_30Rnd;
	class Ammunition_Base;
	// A loose plasma cell. Nothing spawns it as loot: it's what a chambered cell turns into when the rifle is
	// cleared (Plasma_Cell spawnPileType), and it loads back into any Montauk Cartridge (ammoItems). Every colour
	// goes into this one pile and keeps its own round, so it fires in the colour it was loaded as. The cartridge's
	// model, with the green cell window.
	class Ammo_Plasma_Cell: Ammunition_Base
	{
		scope=2;
		displayName="$STR_geb_PlasmaCell";
		descriptionShort="$STR_geb_PlasmaCellDesc";
		model="\alieninvasion\data\weapons\geb_plasmacartridge.p3d";
		iconCartridge=1;
		weight=10;
		count=15;
		ammo="Plasma_Cell";
		hiddenSelections[]=
		{
			"camo"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_green_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_green.rvmat"
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=100;
				};
			};
		};
	};
	class geb_PlasmaCartridge: Mag_AKM_30Rnd
	{
		scope=2;
		displayName="$STR_geb_PlasmaCartridge";
		descriptionShort="$STR_geb_PlasmaCartridgeDesc";
		model = "\alieninvasion\data\weapons\geb_plasmacartridge.p3d";
		weight=50;
		count=15;
		itemSize[]={2,1};
		ammo="Plasma_Cell";
		ammoItems[]=
		{
			"Ammo_Plasma_Cell"
		};
		tracersEvery=1;
		// "camo" is the glowing cell window along both sides. Textures and materials come from
		// tools/plasmacells/build.py; they also replace the AKM magazine's, which Mag_AKM_30Rnd puts on "camo".
		hiddenSelections[]=
		{
			"camo"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_green_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_green.rvmat"
		};
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=100;
				};
			};
		};
	};
	// Same cartridge loaded with another cell colour; its window glows in that colour. Scripts, the recharge
	// recipe and the hands animation all match by inheritance.
	class geb_PlasmaCartridge_Arc: geb_PlasmaCartridge
	{
		displayName="$STR_geb_PlasmaCartridgeArc";
		ammo="Plasma_Cell_Arc";
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_blue_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_blue.rvmat"
		};
	};
	class geb_PlasmaCartridge_Lance: geb_PlasmaCartridge
	{
		displayName="$STR_geb_PlasmaCartridgeLance";
		ammo="Plasma_Cell_Lance";
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_lime_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_lime.rvmat"
		};
	};
	class geb_PlasmaCartridge_Void: geb_PlasmaCartridge
	{
		displayName="$STR_geb_PlasmaCartridgeVoid";
		ammo="Plasma_Cell_Void";
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_violet_co.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\weapons\geb_plasmacartridge_violet.rvmat"
		};
	};
};

// The plasma grenade's blast (tools/plasmagrenade/sounds.py, synthesised): close, mid and distant layers on vanilla's
// grenade explosion shader bases, so they fade in and out with distance as Grenade_explosion_SoundSet's do, at about
// the same levels. CfgAmmo geb_PlasmaGrenade_Ammo plays it with vanilla's Grenade_Tail_SoundSet for the echo.
class CfgSoundShaders
{
	class closeExplosionMid_SoundShader;
	class midExplosionMid_SoundShader;
	class distExplosionMid_SoundShader;
	class geb_PlasmaGrenade_closeExplosion_SoundShader: closeExplosionMid_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\plasmagrenade\sounds\geb_plasmagrenade_close_1",
				1
			},
			{
				"alieninvasion\data\plasmagrenade\sounds\geb_plasmagrenade_close_2",
				1
			}
		};
		volume=1;
	};
	class geb_PlasmaGrenade_midExplosion_SoundShader: midExplosionMid_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\plasmagrenade\sounds\geb_plasmagrenade_mid_1",
				1
			},
			{
				"alieninvasion\data\plasmagrenade\sounds\geb_plasmagrenade_mid_2",
				1
			}
		};
		volume=1;
	};
	class geb_PlasmaGrenade_distExplosion_SoundShader: distExplosionMid_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\plasmagrenade\sounds\geb_plasmagrenade_dist",
				1
			}
		};
		volume=1.7782794;
	};
};

class CfgSoundSets
{
	class Heavy_Explosion_Base_SoundSet;
	class geb_PlasmaGrenade_explosion_SoundSet: Heavy_Explosion_Base_SoundSet
	{
		soundShaders[]=
		{
			"geb_PlasmaGrenade_closeExplosion_SoundShader",
			"geb_PlasmaGrenade_midExplosion_SoundShader",
			"geb_PlasmaGrenade_distExplosion_SoundShader"
		};
	};
};

// The rifle's fluid attachment (tools/plasmarifle/fluid.py): its proxy in the rifle's model and its hidden slot,
// set up like vanilla's Magnum cylinder (Proxymagnum_cylinder; Slot_RevolverCylinder has show "false", so no slot
// shows in the inventory and the fluid can't be taken off).
class CfgNonAIVehicles
{
	class ProxyAttachment;
	class Proxygeb_plasmarifle_fluid: ProxyAttachment
	{
		scope=2;
		inventorySlot="geb_PlasmaRifleFluid";
		model="\alieninvasion\data\weapons\geb_plasmarifle_fluid.p3d";
	};
};

class CfgSlots
{
	class Slot_geb_PlasmaRifleFluid
	{
		name="geb_PlasmaRifleFluid";
		displayName="";
		ghostIcon="";
		show="false";
	};
};
