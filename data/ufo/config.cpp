class CfgPatches
{
	class alieninvasion_ufo
	{
		units[]=
		{
			"geb_UFO"
		};
		weapons[]={};
		requiredVersion=0.1;
		requiredAddons[]=
		{
			"DZ_Data",
			"DZ_Scripts",
			"DZ_Vehicles_Wheeled",
			"DZ_Sounds_Effects"
		};
	};
};

class CfgVehicles
{
	class Transport;

	/*
		The flyable UFO: a black brushed-metal saucer six metres across with green light trim, a glass dome over
		a one-seat cockpit, three landing legs that fold away and a drive that hums. A bare Transport (no car
		underneath): scripts/4_world/entities/vehicles/geb_UFO.c makes its physics body and flies it, with the
		numbers in geb_UFOFlight below. Model, textures and sounds come from tools/alienufo/build.py.

		Nothing spawns it: admins place it (COT, Expansion's or VPP's spawners), or a server adds it to its
		types.xml. A spawned one has its canopy open (OnDebugSpawn).
	*/
	class geb_UFO: Transport
	{
		scope=2;
		displayName="$STR_geb_UFO";
		descriptionShort="$STR_geb_UFODesc";
		model="\alieninvasion\data\ufo\geb_ufo.p3d";
		vehicleClass="Transport";
		storageCategory=4;		// saved with the vehicles, as cars are
		insideSoundCoef=0.9;

		// camo: the hull, for retextures. glow and screens: geb_UFO.c lights them while the drive is on and
		// darkens them while it's off (the dark ones are listed here, as it spawns switched off). camo and glass
		// (the canopy's): geb_UFO.c scorches the hull and cracks the glass as the Hull and Canopy zones wear down.
		// field: the hex grid glowing inside the glass while the drive is on; no texture (hidden) while it's off.
		// glass_in: the glass's inside faces, plain tint (no hexagons from the seat); it cracks along with glass
		hiddenSelections[]=
		{
			"camo",
			"glow",
			"screens",
			"glass",
			"field",
			"glass_in"
		};
		hiddenSelectionsTextures[]=
		{
			"\alieninvasion\data\ufo\geb_ufo_hull_co.paa",
			"\alieninvasion\data\ufo\geb_ufo_glow_off_co.paa",
			"\alieninvasion\data\ufo\geb_ufo_screens_off_co.paa",
			"\alieninvasion\data\ufo\geb_ufo_glass_ca.paa",
			"",
			"\alieninvasion\data\ufo\geb_ufo_glass_in_ca.paa"
		};
		hiddenSelectionsMaterials[]=
		{
			"\alieninvasion\data\ufo\geb_ufo_hull.rvmat",
			"\alieninvasion\data\ufo\geb_ufo_glow_off.rvmat",
			"\alieninvasion\data\ufo\geb_ufo_screens_off.rvmat",
			"\alieninvasion\data\ufo\geb_ufo_glass.rvmat",
			"\alieninvasion\data\ufo\geb_ufo_field.rvmat",
			"\alieninvasion\data\ufo\geb_ufo_glass.rvmat"
		};

		// one seat. Get in by looking at the rim on the craft's left with the canopy open
		class Crew
		{
			class Driver
			{
				actionSel="seat_driver";
				proxyPos="crewDriver";
				getInPos="pos_driver";
				getInDir="pos_driver_dir";
				isDriver=1;
			};
		};

		/*
			The canopy is set on the server, as a car door is. The gear, the pod's light ring and the cockpit's
			hologram are set every frame on each client by geb_UFO.c (tiny animPeriod: they follow at once).
		*/
		class AnimationSources
		{
			class canopy
			{
				source="user";
				initPhase=0;
				animPeriod=2.0;
			};
			class gear
			{
				source="user";
				initPhase=0;
				animPeriod=0.0001;
			};
			class spin: gear {};
			class holo: gear {};
			class shimmer: gear {};
			class radar: gear {};
		};

		/*
			Every Health class needs its healthLevels, as vanilla's vehicles have them: a zone without them reads
			as ruined. A ruined craft won't power up. Hard knocks damage the hull (geb_UFO.EOnContact).
		*/
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints=4000;
					healthLevels[]={{1,{}},{0.7,{}},{0.5,{}},{0.3,{}},{0,{}}};
				};
			};
			class DamageZones
			{
				class Hull
				{
					displayName="$STR_geb_UFOHull";
					fatalInjuryCoef=-1;
					componentNames[]=
					{
						"dmgZone_hull"
					};
					class Health
					{
						hitpoints=4000;
						transferToGlobalCoef=1;
						healthLevels[]={{1,{}},{0.7,{}},{0.5,{}},{0.3,{}},{0,{}}};
					};
					inventorySlots[]={};
				};
				class Canopy
				{
					displayName="$STR_geb_UFOCanopy";
					fatalInjuryCoef=-1;
					componentNames[]=
					{
						"dmgZone_canopy"
					};
					class Health
					{
						hitpoints=800;
						transferToGlobalCoef=0.1;
						healthLevels[]={{1,{}},{0.7,{}},{0.5,{}},{0.3,{}},{0,{}}};
					};
					inventorySlots[]={};
				};
			};
		};

		/*
			How it flies (geb_UFO.c). Speeds in m/s, accelerations in m/s2, angles in degrees, forces in newtons.
			The mass (1,210 kg) and the inertia are the model's: tools/alienufo/build.py prints them.
		*/
		class geb_UFOFlight
		{
			// memory points
			middlePoint="hover_point";		// the middle of the lens
			nosePoint="nose";				// ahead of it: says which way is forward
			doorPoint="pos_door";			// the boarding place on the left rim; where the pilot gets out
			canopySource="canopy";
			reach=4.6;						// m from the middle: someone outside can work the canopy

			// the drive
			spoolTime=1.6;		// s from the start of the power-up action until it takes hold (the action is 0.5 s; the drive comes on by itself after)
			powerLag=0.8;		// s for the drive's power to follow
			maxSpeed=55;		// 198 km/h
			boostSpeed=90;		// 324 km/h with Shift held
			strafeSpeed=22;
			accel=9;			// towards the speed asked for
			boostAccel=15;
			brakeAccel=6;		// keys let go: the inertial dampers bring it to a hover
			speedGain=1.4;		// 1/s: how hard it goes after the difference
			climbRate=10;
			boostClimbRate=16;
			sinkRate=8;
			vertAccel=12;
			climbGain=2.5;
			groundHold=0.6;		// sat on its legs with nothing asking it up, the drive takes this share of the weight
			ceiling=2500;		// m above sea level: it won't climb higher
			yawRate=75;			// deg/s turning on the spot at full stick
			yawAccel=220;		// deg/s2
			yawGain=5;
			tiltMax=16;			// it leans into its push and banks into turns, up to this
			tiltGain=0.9;
			attitudeStiffness=9;	// 1/s2: how firmly it levels itself
			attitudeDamping=5.5;	// 1/s
			inertiaTilt=3360;	// kg m2 about a horizontal axis
			inertiaYaw=6150;	// kg m2 about its own up
			dragArea=2.6;		// m2 of air drag while it falls unpowered
			gearTime=2.2;		// s to fold the legs up or down
			autoLandSink=0.35;	// nobody flying it: sink at this share of sinkRate until it sits on its legs
			idleOffTime=600;	// s it idles on the ground with nobody flying it before it switches itself off (0: never)
			// a knock harder than this (N s; a hard landing that bottoms the legs is about 6,000) damages the
			// hull, by crashDamage health per N s past it
			crashImpulse=9000;
			crashDamage=0.12;
			// while testing: once a second with the drive on, a line in the pilot's script log with the drive,
			// speed, climb, height, attitude, gear, legs and controls. 0 turns it off
			debugLog=1;

			/*
				The legs: each casts a ray down the craft's axis from the bottom of its pad (hub, a memory point),
				a spring and damper hold the craft off the hit and the pad grips. The spring settles by load /
				stiffness, travelDown of it from the built height: with a third of the weight (3,960 N) on each
				leg and stiffness 66,000, the pads sit where they're drawn.
			*/
			class Legs
			{
				class Front
				{
					hub="leg_1_hub";
					stiffness=66000;
					compression=6000;
					rebound=7500;
					travelUp=0.25;
					travelDown=0.06;
					grip=0.9;
				};
				class Left: Front
				{
					hub="leg_2_hub";
				};
				class Right: Front
				{
					hub="leg_3_hub";
				};
			};

			// sounds (data/ufo/sounds, tools/alienufo/sounds.py)
			loopSounds[]=
			{
				"geb_UFO_Hum_SoundSet",
				"geb_UFO_Drive_SoundSet",
				"geb_UFO_Air_SoundSet"
			};
			powerUpSound="geb_UFO_PowerUp_SoundSet";
			powerDownSound="geb_UFO_PowerDown_SoundSet";
			canopyOpenSound="geb_UFO_CanopyOpen_SoundSet";
			canopyCloseSound="geb_UFO_CanopyClose_SoundSet";
			gearDownSound="geb_UFO_GearDown_SoundSet";
			gearUpSound="geb_UFO_GearUp_SoundSet";
		};
	};
};

/*
	The drive's loops are fed by geb_UFO.c (geb_UFOSound) through the sound controllers: rpm is the drive's power
	(0 .. 1000, more as it works harder), thrust how hard it works (0 .. 1), speed in km/h, doors the canopy
	(0 shut .. 1 open), campos 1 with the camera outside.
*/
class CfgSoundShaders
{
	class geb_UFO_Hum_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_hum",
				1
			}
		};
		frequency="0.55 + 0.5 * (rpm / 1000)";
		volume="engineOn * (0.35 + 0.65 * (rpm factor [100,800])) * (0.8 + 0.2 * thrust) * (1 - 0.3 * (1 - campos))";
		range=220;
	};
	class geb_UFO_Drive_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_drive",
				1
			}
		};
		frequency="0.85 + 0.25 * thrust + 0.4 * (speed factor [0,300])";
		volume="engineOn * (rpm factor [300,900]) * (0.25 + 0.45 * thrust + 0.5 * (speed factor [10,200])) * (1 - 0.35 * (1 - campos))";
		range=400;
	};
	class geb_UFO_Air_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_air",
				1
			}
		};
		frequency="0.8 + 0.5 * (speed factor [40,300])";
		volume="(speed factor [30,250]) * (1 - 0.6 * (1 - campos) * (1 - doors))";
		range=150;
	};
	class geb_UFO_PowerUp_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_power_up",
				1
			}
		};
		volume=1;
		range=200;
	};
	class geb_UFO_PowerDown_SoundShader: geb_UFO_PowerUp_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_power_down",
				1
			}
		};
	};
	class geb_UFO_CanopyOpen_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_canopy_open",
				1
			}
		};
		volume=0.8;
		range=40;
	};
	class geb_UFO_CanopyClose_SoundShader: geb_UFO_CanopyOpen_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_canopy_close",
				1
			}
		};
	};
	class geb_UFO_GearDown_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_gear_down",
				1
			}
		};
		volume=0.9;
		range=60;
	};
	class geb_UFO_GearUp_SoundShader: geb_UFO_GearDown_SoundShader
	{
		samples[]=
		{
			{
				"alieninvasion\data\ufo\sounds\geb_ufo_gear_up",
				1
			}
		};
	};
};

class CfgSoundSets
{
	class baseEngine_EXT_SoundSet;
	class geb_UFO_Hum_SoundSet: baseEngine_EXT_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_Hum_SoundShader"
		};
		volumeFactor=1;
	};
	class geb_UFO_Drive_SoundSet: baseEngine_EXT_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_Drive_SoundShader"
		};
		volumeFactor=1;
	};
	class geb_UFO_Air_SoundSet: baseEngine_EXT_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_Air_SoundShader"
		};
		volumeFactor=1;
	};
	// one-shots: the same 3D processing and fall-off as the loops, not looped
	class geb_UFO_PowerUp_SoundSet: baseEngine_EXT_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_PowerUp_SoundShader"
		};
		volumeFactor=1;
		loop=0;
	};
	class geb_UFO_PowerDown_SoundSet: geb_UFO_PowerUp_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_PowerDown_SoundShader"
		};
	};
	class geb_UFO_CanopyOpen_SoundSet: geb_UFO_PowerUp_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_CanopyOpen_SoundShader"
		};
	};
	class geb_UFO_CanopyClose_SoundSet: geb_UFO_PowerUp_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_CanopyClose_SoundShader"
		};
	};
	class geb_UFO_GearDown_SoundSet: geb_UFO_PowerUp_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_GearDown_SoundShader"
		};
	};
	class geb_UFO_GearUp_SoundSet: geb_UFO_PowerUp_SoundSet
	{
		soundShaders[]=
		{
			"geb_UFO_GearUp_SoundShader"
		};
	};
};
