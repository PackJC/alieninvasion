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
			"DZ_Weapons_Ammunition"
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
	class GreenAlienBase : ZombieBase
	{

		scope = 2;
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

		class GlobalHealth
		{
			class Health
			{
				hitpoints =400;
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
			class ObtainedPelt
			{
				item="geb_GreenAlienSkin";
				count=1;
				itemZones[]=
				{
					"Zone_Chest",
					"Zone_Belly"
				};
				quantityCoef=1;
				transferToolDamageCoef=1;
			};
		};
	}

	//Zombie Skeleton
	class geb_GreenAlien : GreenAlienBase
	{
		displayName = "$STR_GreenAlien";
		descriptionShort = "$STR_GreenAlienDesc";
		scope = 2;
		model = "\alieninvasion\data\aliens\geb_GreenAlien.p3d";

		class GlobalHealth
		{
				class Health
				{
					hitpoints = 400;
				};
		};

		class Cargo
		{
				itemsCargoSize[] = { 10,15 };
				allowOwnedCargoManipulation = 1;
				openable = 0;
		};
		class DamageSystem
		{
				class GlobalHealth
				{
					class Health
					{
						hitpoints = 150;
					};
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
			class ObtainedPelt
			{
				item="geb_GreenAlienSkin";
				count=1;
				itemZones[]=
				{
					"Zone_Chest",
					"Zone_Belly"
				};
				quantityCoef=1;
				transferToolDamageCoef=1;
			};
		};
	}
	
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
	class geb_GreenAlienSkin: BearPelt
	{
		scope=2;
		displayName="$STR_GreenAlienSkin";
		descriptionShort="$STR_GreenAlienSkinDesc";
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
		weight=350;
		absorbency=0;
		repairableWithKits[]={0};
		PPDOFProperties[]={1,0.75,40,180,4,10};
		WeaponLength=0.6;
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
		drySound[]=
		{
			"dz\sounds\weapons\firearms\m4a1\m4_dry",
			0.5,
			1,
			20
		};
		hiddenSelections[]={};
		modes[]=
		{
			"SemiAuto"
		};
		class SemiAuto: Mode_SemiAuto
		{
			soundSetShot[]=
			{
				"Plasma_SoundSet"
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
		itemSize[]={5,3};
		spawnDamageRange[]={0,0.6};
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
			class OnOverheating
			{
				maxOverheatingValue=75;
				shotsToStartOverheating=5;
				overheatingDecayInterval=1;
				class SmokingBarrelBase
				{
					positionOffset[]={0.2,0,0};
					onlyWithinRainLimits[]={0,0.2};
				};
				class SmokingBarrel1: SmokingBarrelBase
				{
					overrideParticle="smoking_barrel_small";
					onlyWithinOverheatLimits[]={0,0.2};
				};
				class SmokingBarrelHot1: SmokingBarrelBase
				{
					overrideParticle="smoking_barrel";
					onlyWithinOverheatLimits[]={0.2,0.60000002};
				};
				class SmokingBarrelHot3: SmokingBarrelBase
				{
					overrideParticle="smoking_barrel_heavy";
					onlyWithinOverheatLimits[]={0.60000002,1};
				};
				class SmokingBarrelHotSteam
				{
					overrideParticle="smoking_barrel_steam";
					positionOffset[]={0.34999999,0,0};
					onlyWithinOverheatLimits[]={0,1};
					onlyWithinRainLimits[]={0.2,1};
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
				};
			};
		};
	};
};


//Ammo Types
class cfgAmmoTypes
{
	class AType_Plasma_Cell
	{
		name="Plasma_Cell";
	};
	class AType_Plasma_Cell_Arc
	{
		name="Plasma_Cell_Arc";
	};
	class AType_Plasma_Cell_Lance
	{
		name="Plasma_Cell_Lance";
	};
	class AType_Plasma_Cell_Void
	{
		name="Plasma_Cell_Void";
	};
};

//Ammo
class cfgAmmo
{
	class Bullet_Base;
	class Plasma_Cell: Bullet_Base
	{
		scope=2;
		lootCategory="Crafted";
		casing="";
		// Must name a real round: the engine's EffectBulletShow builds the chambered round from it with no null
		// check, so round="" crashed the server whenever a chambered plasma cell was shown (COT/admin spawns with
		// ammo, weapons loaded from storage). The rifle model has no chamber proxy, so this round is never seen.
		round="FxRound_22";
		// Vanilla green tracer as the plasma bolt (traces from the muzzle on every shot; see tracersEvery on the cartridge)
		model="\dz\weapons\projectiles\tracer_green.p3d";
		spawnPileType="Ammo_Plasma_Cell";
		// Muzzle effects are generated by tools/plasmashots/shots.py; this one beats the rifle's overrideParticle
		muzzleFlashParticle="geb_plasma_ionburst";
		// Muzzle light colour (0..1) for this cell's shots, read by the rifle script instead of vanilla's orange light
		geb_muzzleLightColor[]={0.3,1,0.4};
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
	// The other cell colours: same round, own muzzle effect, light and bolt. Lime fires vanilla's yellow tracer;
	// vanilla has no blue or violet one, so tools/plasmashots/tracers.py builds those bolts in the green tracer's
	// shape and the cell's colour. Scripts test IsKindOf(ammo, "Plasma_Cell").
	class Plasma_Cell_Arc: Plasma_Cell
	{
		model="\alieninvasion\data\weapons\geb_tracer_blue.p3d";
		muzzleFlashParticle="geb_plasma_arc";
		geb_muzzleLightColor[]={0.3,0.7,1};
	};
	class Plasma_Cell_Lance: Plasma_Cell
	{
		model="\dz\weapons\projectiles\tracer_yellow.p3d";
		muzzleFlashParticle="geb_plasma_lance";
		geb_muzzleLightColor[]={0.7,1,0.15};
	};
	class Plasma_Cell_Void: Plasma_Cell
	{
		model="\alieninvasion\data\weapons\geb_tracer_violet.p3d";
		muzzleFlashParticle="geb_plasma_void";
		geb_muzzleLightColor[]={0.7,0.3,1};
	};
};

//Magazines
class cfgMagazines
{
	class Mag_AKM_30Rnd;
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
