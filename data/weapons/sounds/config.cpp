class CfgPatches
{
	class alieninvasion_sounds_weapons
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

// The Montauk rifle's sounds: tools/plasmarifle/sounds.py makes every geb_plasma_*.ogg here, and its docstring says
// what each one is. Built like vanilla's rifles (DZ\sounds\hpp\config.cpp, Ruger1022 and SKS): close, mid and
// distant layers picked by distance, an interior version and a mechanism layer near the shooter, three takes each,
// and the plasma's own echo in every one of vanilla's tail slots (each tail shader takes over the SKS one's range,
// curves and surroundings). Until 2.0.0 it played one short clip at every range (plasma1-3.ogg, one recording
// three times; plasma2 is still the alien zap's, data/aliens/sounds/config.cpp). The handling sets are the plasma
// sounds alone, none of vanilla's Ruger foley; data/config.cpp maps them onto the Ruger animations' sound event
// ids for this rifle only.
class CfgSoundShaders
{
	class new_closeShotSMG_SoundShader;
	class new_midShotSMG_SoundShader;
	class new_distShotSMG_SoundShader;
	class base_Closure_SoundShader;
	class new_Interior_CloseShotRifle_SoundShader;
	class new_Interior_MidShotRifle_SoundShader;
	class reloadWeapon_SoundShader;
	class SKS_tailForest_close_SoundShader;
	class SKS_tailForest_mid_SoundShader;
	class SKS_tailForest_dist_SoundShader;
	class SKS_tailMeadows_close_SoundShader;
	class SKS_tailMeadows_mid_SoundShader;
	class SKS_tailMeadows_dist_SoundShader;
	class SKS_tailHouses_close_SoundShader;
	class SKS_tailHouses_mid_SoundShader;
	class SKS_tailHouses_dist_SoundShader;
	class SKS_tailInterior_Close_SoundShader;
	class SKS_tailInterior_Mid_SoundShader;
	class SKS_Tail_Houses_2D_SoundShader;
	class SKS_Tail_Meadowes_2D_SoundShader;

	class geb_Plasma_closeShot_SoundShader: new_closeShotSMG_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_close_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_close_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_close_03",1}
		};
	};
	class geb_Plasma_midShot_SoundShader: new_midShotSMG_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_mid_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_mid_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_mid_03",1}
		};
	};
	class geb_Plasma_distShot_SoundShader: new_distShotSMG_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_dist_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_dist_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_dist_03",1}
		};
		// Heard as far as vanilla rifles' distant shots (2000 m, IZH-18); full level to 1500 m, then fading out
		// like vanilla's Shot_Dist curves
		range=2000;
		rangeCurve[]=
		{
			{0,0},
			{50,0},
			{400,1},
			{1500,1},
			{1950,0.4},
			{2000,0}
		};
	};
	// The capacitor recharging after each shot. base_Closure plays at volume 2 for a bolt's clack; a sustained
	// whine is quieter
	class geb_Plasma_Mech_SoundShader: base_Closure_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_mech_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_mech_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_mech_03",1}
		};
		volume=0.5;
	};
	class geb_Plasma_closeShot_Interior_SoundShader: new_Interior_CloseShotRifle_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_int_close_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_int_close_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_int_close_03",1}
		};
		rangeCurve="closeShotInteriorRifleCurve";
	};
	class geb_Plasma_midShot_Interior_SoundShader: new_Interior_MidShotRifle_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_int_mid_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_int_mid_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_int_mid_03",1}
		};
		rangeCurve="Interior_mid_Shot_and_Tails_Curve";
		range=800;
	};

	// The echo (sounds.py tails): each slot's shader is the SKS one with the plasma's echo in it
	class geb_Plasma_tailForest_close_SoundShader: SKS_tailForest_close_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_close_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_close_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_close_03",1}
		};
	};
	class geb_Plasma_tailForest_mid_SoundShader: SKS_tailForest_mid_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_mid_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_mid_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_mid_03",1}
		};
	};
	class geb_Plasma_tailForest_dist_SoundShader: SKS_tailForest_dist_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_dist_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_dist_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_forest_dist_03",1}
		};
	};
	class geb_Plasma_tailMeadows_close_SoundShader: SKS_tailMeadows_close_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_close_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_close_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_close_03",1}
		};
	};
	class geb_Plasma_tailMeadows_mid_SoundShader: SKS_tailMeadows_mid_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_mid_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_mid_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_mid_03",1}
		};
	};
	class geb_Plasma_tailMeadows_dist_SoundShader: SKS_tailMeadows_dist_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_dist_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_dist_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_dist_03",1}
		};
	};
	class geb_Plasma_tailHouses_close_SoundShader: SKS_tailHouses_close_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_close_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_close_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_close_03",1}
		};
	};
	class geb_Plasma_tailHouses_mid_SoundShader: SKS_tailHouses_mid_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_mid_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_mid_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_mid_03",1}
		};
	};
	class geb_Plasma_tailHouses_dist_SoundShader: SKS_tailHouses_dist_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_dist_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_dist_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_dist_03",1}
		};
	};
	class geb_Plasma_tailInterior_Close_SoundShader: SKS_tailInterior_Close_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_interior_close_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_interior_close_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_interior_close_03",1}
		};
	};
	class geb_Plasma_tailInterior_Mid_SoundShader: SKS_tailInterior_Mid_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_interior_mid_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_interior_mid_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_interior_mid_03",1}
		};
	};
	class geb_Plasma_Tail_Houses_2D_SoundShader: SKS_Tail_Houses_2D_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_2d_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_2d_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_houses_2d_03",1}
		};
	};
	class geb_Plasma_Tail_Meadows_2D_SoundShader: SKS_Tail_Meadowes_2D_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_2d_01",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_2d_02",1},
			{"alieninvasion\data\weapons\sounds\geb_plasma_tail_meadows_2d_03",1}
		};
	};

	// Handling, at vanilla handling's volume (sounds.py levels each like the vanilla sound on its event)
	// the clamp letting go (vanilla's mag_out names a file that doesn't exist)
	class geb_Plasma_Unlatch_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_unlatch",1}
		};
		volume=0.70794576;
	};
	// the cell sliding out and powering down
	class geb_Plasma_CellOut_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_cell_out",1}
		};
		volume=0.70794576;
	};
	// the cell sliding into the well (silent in vanilla, as mag_out)
	class geb_Plasma_CellSlide_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_cell_slide",1}
		};
		volume=0.70794576;
	};
	// the clamp seating it, power-up whine, ready beeps
	class geb_Plasma_CellIn_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_cell_in",1}
		};
		volume=0.70794576;
	};
	// servo, seal unlocking, the chamber venting
	class geb_Plasma_Vent_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_vent",1}
		};
		volume=0.70794576;
	};
	// the servo stopping
	class geb_Plasma_ServoStop_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_servo_stop",1}
		};
		volume=0.70794576;
	};
	// the servo back, the chamber sealing
	class geb_Plasma_Seal_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_seal",1}
		};
		volume=0.70794576;
	};
	// the capacitor arming
	class geb_Plasma_Charge_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_charge",1}
		};
		volume=0.70794576;
	};
	// a charge drawn from the cell into the chamber
	class geb_Plasma_Chamber_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_chamber",1}
		};
		volume=0.70794576;
	};
	// the rifle powering on as it's drawn
	class geb_Plasma_PowerOn_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_power_on",1}
		};
		volume=0.70794576;
	};
	// the plasma relay's clack and sputter instead of the rimfire click
	class geb_Plasma_Dry_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_dry",1}
		};
		volume=0.70794576;
	};
	// Clearing a jam (sounds.py FAULTS), six kinds for vanilla's eighteen jam sounds
	class geb_Plasma_Fault_Snap_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_fault_snap",1}
		};
		volume=0.70794576;
	};
	class geb_Plasma_Fault_Clear_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_fault_clear",1}
		};
		volume=0.70794576;
	};
	class geb_Plasma_Fault_Long_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_fault_long",1}
		};
		volume=0.70794576;
	};
	class geb_Plasma_Fault_Strain_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_fault_strain",1}
		};
		volume=0.70794576;
	};
	class geb_Plasma_Fault_Soft_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_fault_soft",1}
		};
		volume=0.70794576;
	};
	class geb_Plasma_Fault_Tick_SoundShader: reloadWeapon_SoundShader
	{
		samples[]=
		{
			{"alieninvasion\data\weapons\sounds\geb_plasma_fault_tick",1}
		};
		volume=0.70794576;
	};
};
class CfgSoundSets
{
	class RifleNew_Shot_Base_SoundSet;
	class RifleNew_Shot_Interior_Base_SoundSet;
	class baseCharacter_SoundSet;
	class SKS_Tail_SoundSet;
	class SKS_InteriorTail_SoundSet;
	class SKS_Tail_2D_SoundSet;

	// geb_ names so another mod's plasma gun can't clash
	class geb_Plasma_Shot_SoundSet: RifleNew_Shot_Base_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_closeShot_SoundShader",
			"geb_Plasma_midShot_SoundShader",
			"geb_Plasma_distShot_SoundShader",
			"geb_Plasma_Mech_SoundShader"
		};
	};
	class geb_Plasma_Shot_Interior_SoundSet: RifleNew_Shot_Interior_Base_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_closeShot_Interior_SoundShader",
			"geb_Plasma_midShot_Interior_SoundShader"
		};
	};
	// The echo, in place of vanilla's SKS tails: the SKS sets (their volumes, start delay and pitch spread) with
	// the plasma's shaders, at its own pitch (the SKS sets play theirs lower)
	class geb_Plasma_Tail_SoundSet: SKS_Tail_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_tailForest_close_SoundShader",
			"geb_Plasma_tailForest_mid_SoundShader",
			"geb_Plasma_tailForest_dist_SoundShader",
			"geb_Plasma_tailMeadows_close_SoundShader",
			"geb_Plasma_tailMeadows_mid_SoundShader",
			"geb_Plasma_tailMeadows_dist_SoundShader",
			"geb_Plasma_tailHouses_close_SoundShader",
			"geb_Plasma_tailHouses_mid_SoundShader",
			"geb_Plasma_tailHouses_dist_SoundShader"
		};
		frequencyFactor=1;
	};
	class geb_Plasma_InteriorTail_SoundSet: SKS_InteriorTail_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_tailInterior_Close_SoundShader",
			"geb_Plasma_tailInterior_Mid_SoundShader"
		};
		frequencyFactor=1;
	};
	class geb_Plasma_Tail_2D_SoundSet: SKS_Tail_2D_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Tail_Houses_2D_SoundShader",
			"geb_Plasma_Tail_Meadows_2D_SoundShader"
		};
		frequencyFactor=1;
	};

	// Handling, one set per Ruger animation sound event (data/config.cpp AnimEvents)
	class geb_Plasma_mag_out_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Unlatch_SoundShader"
		};
	};
	class geb_Plasma_mag_remove_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_CellOut_SoundShader"
		};
	};
	class geb_Plasma_mag_in_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_CellSlide_SoundShader"
		};
	};
	class geb_Plasma_mag_load_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_CellIn_SoundShader"
		};
	};
	class geb_Plasma_charge_open_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Vent_SoundShader"
		};
	};
	class geb_Plasma_charge_open_release_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_ServoStop_SoundShader"
		};
	};
	class geb_Plasma_charge_close_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Seal_SoundShader"
		};
	};
	class geb_Plasma_charge_close_release_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Charge_SoundShader"
		};
	};
	class geb_Plasma_chamber_load_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Chamber_SoundShader"
		};
	};
	class geb_Plasma_pullout_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_PowerOn_SoundShader"
		};
	};
	class geb_Plasma_dry_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Dry_SoundShader"
		};
	};
	class geb_Plasma_fault_snap_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Fault_Snap_SoundShader"
		};
	};
	class geb_Plasma_fault_clear_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Fault_Clear_SoundShader"
		};
	};
	class geb_Plasma_fault_long_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Fault_Long_SoundShader"
		};
	};
	class geb_Plasma_fault_strain_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Fault_Strain_SoundShader"
		};
	};
	class geb_Plasma_fault_soft_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Fault_Soft_SoundShader"
		};
	};
	class geb_Plasma_fault_tick_SoundSet: baseCharacter_SoundSet
	{
		soundShaders[]=
		{
			"geb_Plasma_Fault_Tick_SoundShader"
		};
	};
};
