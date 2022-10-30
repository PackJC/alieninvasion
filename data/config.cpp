class CfgPatches
{
	class alieninvasion
	{
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Characters",
			"DZ_Structures",
			"DZ_Weapons_Firearms"
		};
	};
};

class cfgVehicles
{
	//Inherited from classes
	class ZombieBase;
	class BearPelt;
	class Edible_Base;
	class FoodAnimationSources;
	class MeatStageTransitions;
	class CowSteakMeat;
	class HouseNoDestruct;
	class Clothing_Base;
	class geb_FoilHat: Clothing_Base
	{
		descriptionShort="$STR_geb_FoilHat";
		displayName="$STR_geb_FoilHatDesc";
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
		weight=470;
		itemSize[]={4,3};
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
								"Pith_Helmet\Data\PithHelmet.rvmat",
								"Pith_Helmet\Data\PithHelmet.rvmat"
							}
						},
						
						{
							0.69999999,
							
							{
								"Pith_Helmet\Data\PithHelmet.rvmat",
								"Pith_Helmet\Data\PithHelmet.rvmat"
							}
						},
						
						{
							0.5,
							
							{
								"Pith_Helmet\Data\PithHelmet_damage.rvmat",
								"Pith_Helmet\Data\PithHelmet_damage.rvmat"
							}
						},
						
						{
							0.30000001,
							
							{
								"Pith_Helmet\Data\PithHelmet_damage.rvmat",
								"Pith_Helmet\Data\PithHelmet_damage.rvmat"
							}
						},
						
						{
							0,
							
							{
								"Pith_Helmet\Data\PithHelmet_destruct.rvmat",
								"Pith_Helmet\Data\PithHelmet_destruct.rvmat"
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
		class AnimEvents
		{
			class Steps
			{
				class Walk1
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 1;
				};
				class Walk2
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 2;
				};
				class Walk3
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 3;
				};
				class Walk4
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 4;
				};
				class Run1
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 5;
				};
				class Run2
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 6;
				};
				class Run3
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 7;
				};
				class Run4
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 8;
				};
				class Sprint1
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 9;
				};
				class Sprint2
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 10;
				};
				class Sprint3
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 11;
				};
				class Sprint4
				{
					soundLookupTable = "";
					noise = "ZombieStepNoise";
					id = 12;
				};
				class Scuff1
				{
					soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 17;
				};
				class Scuff2
				{
					soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 18;
				};
				class Sccuff3
				{
					soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 19;
				};
				class Scuff4
				{
					soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 20;
				};
				class landFeetErc
				{
					soundLookupTable = "landFeetErc_Sneakers_Zmb_LookupTable";
					noise = "ZombieStepNoise";
					id = 21;
				};
				class landFootErc
				{
					soundLookupTable = "landFootErc_Sneakers_Zmb_LookupTable";
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
					soundSet = "";
					id = 1;
				};
				class Attack_Light2
				{
					soundSet = "";
					id = 2;
				};
				class Attack_Heavy1
				{
					soundSet = "";
					id = 3;
				};
				class Attack_Heavy2
				{
					soundSet = "";
					id = 4;
				};
				class TwoHands
				{
					soundSet = "";
					id = 5;
				};
			};
			class SoundVoice
			{
				class LightHit
				{
					soundSet = "";
					id = 1;
				};
				class HeavyHit
				{
					soundSet = "";
					id = 2;
				};
				class Attack
				{
					soundSet = "";
					id = 5;
				};
				class Jump
				{
					soundSet = "";
					id = 10;
				};
				class Land
				{
					soundSet = "";
					id = 11;
				};
				class CallToArmsShort
				{
					soundSet = "";
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
		class AttackActions
		{
			class AttackLong
			{
				attackName = "attackLong";
				ammoType = "MeleeZombieMale";
				stanceName = "erect";
				moveAnimNames[] =
				{
					"run",
					"sprint"
				};
				minDistance = 0;
				distance = 100.75;
				time = 0.001;
				yawAngle = 90;
				pitchAngle = 30;
				attackWidth = 5.1999998;
				repeatable = 0;
				cooldown = 100.75;
			};
			class AttackRun
			{
				attackName = "attackRun";
				ammoType = "MeleeZombieMale";
				stanceName = "erect";
				moveAnimNames[] =
				{
					"run",
					"sprint"
				};
				minDistance = 0;
				distance = 100.75;
				time = 0.001;
				yawAngle = 90;
				pitchAngle = 30;
				attackWidth = 5.1999998;
				repeatable = 0;
				cooldown = 100.75;
			};
			class AttackShort
			{
				attackName = "attackShort";
				ammoType = "MeleeZombieMale";
				stanceName = "erect";
				moveAnimNames[] =
				{
					"idle",
					"walk"
				};
				minDistance = 0;
				distance = 100.75;
				time = 0.001;
				yawAngle = 90;
				pitchAngle = 30;
				attackWidth = 5.1999998;
				repeatable = 0;
				cooldown = 100.75;
			};
			class AttackShortLow
			{
				attackName = "attackShortLow";
				ammoType = "MeleeZombieMale";
				stanceName = "erect";
				moveAnimNames[] =
				{
					"idle",
					"walk",
					"run"
				};
				minDistance = 0;
				distance = 100.75;
				time = 0.001;
				yawAngle = 90;
				pitchAngle = 30;
				attackWidth = 5.1999998;
				repeatable = 0;
				cooldown = 100.75;
			};
			class CrawlAttackMove
			{
				attackName = "crawlAttackMove";
				ammoType = "MeleeZombieMale";
				stanceName = "crawl";
				moveAnimNames[] =
				{
					"walk"
				};
				minDistance = 0;
				distance = 100.75;
				time = 0.001;
				yawAngle = 90;
				pitchAngle = 30;
				attackWidth = 5.1999998;
				repeatable = 0;
				cooldown = 100.75;
			};
			class CrawlAttackStill
			{
				attackName = "crawlAttackStill";
				ammoType = "MeleeZombieMale";
				stanceName = "crawl";
				moveAnimNames[] =
				{
					"idle"
				};
				minDistance = 0;
				distance = 100.75;
				time = 0.001;
				yawAngle = 90;
				pitchAngle = 30;
				attackWidth = 5.1999998;
				repeatable = 0;
				cooldown = 100.75;
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
				item="geb_AlienSkin";
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

		hiddenSelections[] =
		{
			"body_m"
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
		class AnimEvents
		{
				class Steps
				{
					class Walk1
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 1;
					};
					class Walk2
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 2;
					};
					class Walk3
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 3;
					};
					class Walk4
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 4;
					};
					class Run1
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 5;
					};
					class Run2
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 6;
					};
					class Run3
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 7;
					};
					class Run4
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 8;
					};
					class Sprint1
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 9;
					};
					class Sprint2
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 10;
					};
					class Sprint3
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 11;
					};
					class Sprint4
					{
						soundLookupTable = "Alien_Calm_soundset";
						noise = "ZombieStepNoise";
						id = 12;
					};
					class Scuff1
					{
						soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
						noise = "ZombieStepNoise";
						id = 17;
					};
					class Scuff2
					{
						soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
						noise = "ZombieStepNoise";
						id = 18;
					};
					class Sccuff3
					{
						soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
						noise = "ZombieStepNoise";
						id = 19;
					};
					class Scuff4
					{
						soundLookupTable = "scuffErc_Sneakers_Zmb_LookupTable";
						noise = "ZombieStepNoise";
						id = 20;
					};
					class landFeetErc
					{
						soundLookupTable = "landFeetErc_Sneakers_Zmb_LookupTable";
						noise = "ZombieStepNoise";
						id = 21;
					};
					class landFootErc
					{
						soundLookupTable = "landFootErc_Sneakers_Zmb_LookupTable";
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
						soundSet = "Alien_Attack_soundset";
						id = 1;
					};
					class Attack_Light2
					{
						soundSet = "Alien_Attack_soundset";
						id = 2;
					};
					class Attack_Heavy1
					{
						soundSet = "Alien_Attack_soundset";
						id = 3;
					};
					class Attack_Heavy2
					{
						soundSet = "Alien_Attack_soundset";
						id = 4;
					};
					class TwoHands
					{
						soundSet = "Alien_Attack_soundset";
						id = 5;
					};
				};
				class SoundVoice
				{
					class LightHit
					{
						soundSet = "Alien_Attack_soundset";
						id = 1;
					};
					class HeavyHit
					{
						soundSet = "Alien_Attack_soundset";
						id = 2;
					};
					class Attack
					{
						soundSet = "Alien_Attack_soundset";
						id = 5;
					};
					class Jump
					{
						soundSet = "Alien_Attack_soundset";
						id = 10;
					};
					class Land
					{
						soundSet = "Alien_Attack_soundset";
						id = 11;
					};
					class CallToArmsShort
					{
						soundSet = "Alien_Attack_soundset";
						id = 20;
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
				item="geb_AlienSkin";
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
		hiddenSelectionsTextures[] =
		{
			"\alieninvasion\data\aliens\alienmeat.paa";
			"dz\gear\food\data\meat_steak_baked_CO.paa",
			"dz\gear\food\data\meat_steak_boiled_co.paa",
			"dz\gear\food\data\meat_steak_dried_CO.paa",
			"dz\gear\food\data\meat_steak_burned_CO.paa"
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
			"\alieninvasion\data\aliens\alienpelt.paa";
		};
	}	
	
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
		magazines[]=
		{
			"geb_PlasmaCartridge"
		};
		magazineSwitchTime=0.65;
		ejectType=1;
		recoilModifier[]={1,1,1};
		swayModifier[]={1.2,1.2,1};
		simpleHiddenSelections[]=
		{
			"hide_barrel"
		};
		drySound[]=
		{
			"dz\sounds\weapons\firearms\m4a1\m4_dry",
			0.5,
			1,
			20
		};
		reloadAction="ReloadFal";
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
					overrideParticle="geb_plasma_shot";
					illuminateWorld=1;
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
		round="";
		//model = "\alieninvasion\data\weapons\geb_plasmaorb.p3d";
		spawnPileType="Ammo_Plasma_Cell";
		muzzleFlashParticle="Plasma_Shot";
        hit=12;
        indirectHit=0;
        indirectHitRange=0;
        visibleFire=22;
        audibleFire=22;
        visibleFireTime=3;
        cost=1.2;
        airLock=1;
        typicalSpeed=360;
        tracerScale=1.2;
        tracerStartTime=-1;
        tracerEndTime=1;
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
		itemSize[]={3,2};
		ammo="Plasma_Cell";
		ammoItems[]=
		{
			"Ammo_Plasma_Cell"
		};
		tracersEvery=0;
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
