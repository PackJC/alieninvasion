// Register all particles below!

modded class ParticleList
{
	static const int UFO_FIRE = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_fire");
	static const int UFO_WRECK = RegisterParticle( "alieninvasion/graphics/particles/" , "ufo_wreck");
	static const int geb_plasma_shot = RegisterParticle( "alieninvasion/graphics/particles/" , "geb_plasma_shot");

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
