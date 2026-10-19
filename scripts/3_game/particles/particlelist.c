// Register all particles below!

modded class ParticleList
{
	static const int UFO_FIRE = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_fire");
	static const int UFO_WRECK = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_wreck");
	// The original plasma shot: no longer used by config, kept so tools/plasmashots/preview.py can compare against it
	static const int geb_plasma_shot = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_shot");
	// Muzzle effects of the four cell colours (tools/plasmashots/shots.py), set per ammo by muzzleFlashParticle
	static const int GEB_PLASMA_IONBURST = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_ionburst");
	static const int GEB_PLASMA_ARC = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_arc");
	static const int GEB_PLASMA_LANCE = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_lance");
	static const int GEB_PLASMA_VOID = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_void");

	// Green copies of vanilla's flesh-hit blood (tools/build_alien_blood.py), played for aliens by geb_AlienBlood.c
	static const int GEB_ALIEN_BLOOD_ENTER = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_enter");
	static const int GEB_ALIEN_BLOOD_EXIT = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_exit");
	static const int GEB_ALIEN_BLOOD_DROPS = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_drops");
	static const int GEB_ALIEN_BLOOD_CHUNKS = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_alien_blood_chunks");
	// Specimen tank (tools/alientank/effects.py), played by geb_AlienTank.c
	static const int GEB_TANK_BUBBLES = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_tank_bubbles");
	static const int GEB_TANK_MOTES = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_tank_motes");
	static const int GEB_TANK_STEAM = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_tank_steam");
}
