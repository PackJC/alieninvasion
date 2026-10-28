// Register all particles below!

modded class ParticleList
{
	static const int UFO_FIRE = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_fire");
	static const int UFO_SMOKE = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_smoke");	// the wreck's smoke column: vanilla's heli-wreck smoke thinned (tools/crash_effects.py)
	static const int UFO_WRECK = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_wreck");
	// The original plasma laser, as it was: geb_PlasmaRifle.c plays it for a cell that names no laser of its own
	static const int geb_plasma_shot = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_shot");
	// Muzzle effects of the four cell colours (tools/plasmashots/shots.py), set per ammo by muzzleFlashParticle
	static const int GEB_PLASMA_IONBURST = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_ionburst");
	static const int GEB_PLASMA_ARC = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_arc");
	static const int GEB_PLASMA_LANCE = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_lance");
	static const int GEB_PLASMA_VOID = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_void");
	// Each cell's plasma ball (tools/plasmashots/shots.py; the original laser until 2.0.0), named per ammo by CfgAmmo geb_laserParticle
	static const int GEB_PLASMA_LASER_IONBURST = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_laser_ionburst");
	static const int GEB_PLASMA_LASER_ARC = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_laser_arc");
	static const int GEB_PLASMA_LASER_LANCE = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_laser_lance");
	static const int GEB_PLASMA_LASER_VOID = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_laser_void");
	// The violet cell's lightning (tools/plasmashots/shots.py): one bolt segment, and where the bolt lands, played by geb_PlasmaRifle.c
	static const int GEB_PLASMA_BOLT_VOID = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_bolt_void");
	static const int GEB_PLASMA_BOLT_VOID_HIT = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_bolt_void_hit");
	// Looping arcs on a hot muzzle (tools/plasmashots/shots.py), named by the rifle's Particles > OnOverheating entries
	static const int GEB_PLASMA_OVERHEAT = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_overheat");
	static const int GEB_PLASMA_OVERHEAT_HEAVY = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_overheat_heavy");
	// The aliens' psychic zap (beam from the head, hit on the victim) and death burst (tools/plasmashots/alien.py),
	// played by geb_GreenAlien.c
	static const int GEB_ALIEN_ZAP_BEAM = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_zap_beam");
	static const int GEB_ALIEN_ZAP_HIT = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_zap_hit");
	static const int GEB_ALIEN_DEATH = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_death");

	// Green copies of vanilla's flesh-hit blood (tools/build_alien_blood.py), played for aliens by geb_AlienBlood.c
	static const int GEB_ALIEN_BLOOD_ENTER = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_enter");
	static const int GEB_ALIEN_BLOOD_EXIT = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_exit");
	static const int GEB_ALIEN_BLOOD_DROPS = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_drops");
	static const int GEB_ALIEN_BLOOD_CHUNKS = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_chunks");
	// Specimen tank (tools/alientank/effects.py), played by geb_AlienTank.c
	static const int GEB_TANK_BUBBLES = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_tank_bubbles");
	static const int GEB_TANK_MOTES = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_tank_motes");
	static const int GEB_TANK_STEAM = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_tank_steam");
	// Plasma grenade (tools/plasmagrenade/effects.py): the armed glow and trail, played by geb_PlasmaGrenade.c, and
	// the blast, named by CfgAmmo geb_PlasmaGrenade_Ammo particle
	static const int GEB_PLASMAGRENADE_ARMED = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasmagrenade_armed");
	static const int GEB_PLASMAGRENADE_BLAST = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasmagrenade_blast");
	// Flyable UFO (tools/alienufo/effects.py), played by geb_UFO.c: the downwash's dust, the green sparks under the
	// pod and the damaged hull's smoke
	static const int GEB_UFO_DUST = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_ufo_dust");
	static const int GEB_UFO_MOTES = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_ufo_motes");
	static const int GEB_UFO_SMOKE = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_ufo_smoke");
}
