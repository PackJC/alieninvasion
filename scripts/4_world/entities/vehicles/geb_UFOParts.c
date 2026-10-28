/*
	The flyable UFO's parts (geb_UFO.c): its legs, its sound and its lights.
*/

/*
	One landing leg. Each physics step, with the gear down, it casts a ray down
	the craft's axis from the bottom of its pad to find the ground. A spring and
	damper push the craft up off the hit along the ground's normal, and the pad
	grips: it takes out its sliding as far as its load lets it. The usual
	raycast suspension of game vehicles, through Transport.ApplyForceAt, the
	deterministic vehicle physics a network correction replays.
*/
class geb_UFOLeg
{
	static const float GRAVITY = 9.81;
	static const float PAD_MASS_SHARE = 0.5;	//!< of the load's mass, what the pad stops in one step

	// configuration (geb_UFOFlight >> Legs >> <name>)
	string m_Name;
	vector m_Hub;			//!< model space: the bottom of the pad as built, on the ground
	float m_Stiffness;		//!< N/m
	float m_Compression;	//!< N per m/s, damping as the leg closes
	float m_Rebound;		//!< N per m/s, as it opens
	float m_TravelUp;		//!< m above the built height the pad can be pushed
	float m_TravelDown;		//!< m below it it can hang
	float m_Grip;			//!< the pad's friction

	// this step
	bool m_Contact;
	float m_Load;			//!< N
	float m_Compressed;		//!< m

	static geb_UFOLeg Load(string path, string name, Object ufo)
	{
		string hub;
		g_Game.ConfigGetText(path + " hub", hub);
		if (hub == "" || !ufo.MemoryPointExists(hub))
		{
			ErrorEx(string.Format("AlienInvasion: %1 has no memory point '%2' for leg %3", ufo.GetType(), hub, name));
			return null;
		}

		geb_UFOLeg leg = new geb_UFOLeg();
		leg.m_Name = name;
		leg.m_Hub = ufo.GetMemoryPointPos(hub);
		leg.m_Stiffness = g_Game.ConfigGetFloat(path + " stiffness");
		leg.m_Compression = g_Game.ConfigGetFloat(path + " compression");
		leg.m_Rebound = g_Game.ConfigGetFloat(path + " rebound");
		leg.m_TravelUp = g_Game.ConfigGetFloat(path + " travelUp");
		leg.m_TravelDown = Math.Max(g_Game.ConfigGetFloat(path + " travelDown"), 0.01);
		leg.m_Grip = g_Game.ConfigGetFloat(path + " grip");
		return leg;
	}

	//! Folded away: no contact.
	void Lift()
	{
		m_Contact = false;
		m_Load = 0;
		m_Compressed = 0;
	}

	void Simulate(geb_UFO ufo, float dt, vector upW)
	{
		Lift();
		if (dt <= 0)
			return;

		vector hubW = ufo.ModelToWorld(m_Hub);
		float length = m_TravelUp + m_TravelDown;
		vector from = hubW + upW * m_TravelUp;
		vector to = hubW - upW * m_TravelDown;

		Object hitObject;
		vector hitPos;
		vector hitNormal;
		float hitFraction;
		int layers = PhxInteractionLayers.DEFAULT | PhxInteractionLayers.BUILDING | PhxInteractionLayers.ROADWAY | PhxInteractionLayers.TERRAIN | PhxInteractionLayers.ITEM_LARGE;
		if (!DayZPhysics.RayCastBullet(from, to, layers, ufo, hitObject, hitPos, hitNormal, hitFraction))
			return;

		m_Contact = true;
		m_Compressed = length * (1 - Math.Clamp(hitFraction, 0, 1));

		// the spring and damper, along the ground's normal
		float closing = -vector.Dot(dBodyGetVelocityAt(ufo, hubW), hitNormal);
		float damping = m_Rebound;
		if (closing > 0)
			damping = m_Compression;

		m_Load = Math.Max(m_Stiffness * m_Compressed + closing * damping, 0);
		ufo.ApplyForceAt(hitPos, hitNormal * m_Load);

		// the pad grips: what stopping its slide in one step would take for the mass on it, up to its friction
		vector slip = dBodyGetVelocityAt(ufo, hitPos);
		slip = slip - hitNormal * vector.Dot(slip, hitNormal);
		float stop = m_Load / GRAVITY * PAD_MASS_SHARE / dt;
		vector grip = slip * -stop;
		float limit = m_Load * m_Grip;
		float gripLen = grip.Length();
		if (gripLen > limit && gripLen > 0)
			grip = grip * (limit / gripLen);

		ufo.ApplyForceAt(hitPos, grip);
	}
};

/*
	The drive's loops, on clients: the hum, the whine and the air. They play
	while the drive is on, each through a sound object built with the sound
	controllers its shader reads (data/ufo/config.cpp), updated every frame:
	rpm the drive's power (0 .. 1000), thrust its effort (0 .. 1), speed in
	km/h, engineOn, doors (the canopy, 0 shut .. 1 open) and campos (1 with the
	camera outside).
*/
class geb_UFOSound
{
	protected ref array<ref SoundParams> m_Params = {};
	protected ref array<ref SoundObjectBuilder> m_Builders = {};
	protected ref array<ref SoundObject> m_Objects = {};
	protected ref array<AbstractWave> m_Waves = {};
	protected ref array<float> m_Values = {0, 0, 0, 0, 0, 1};

	void geb_UFOSound(Object owner, vector position, TStringArray soundSets)
	{
		// the controllers the shaders read, in the order Update() fills m_Values
		TStringArray controllers = {"rpm", "thrust", "speed", "engineOn", "doors", "campos"};
		foreach (string soundSet : soundSets)
		{
			SoundParams params = new SoundParams(soundSet);
			if (!params.IsValid())
				continue;

			SoundObjectBuilder builder = new SoundObjectBuilder(params);
			foreach (string controller : controllers)
				builder.AddVariable(controller, 0);

			SoundObject sound = builder.BuildSoundObject();
			if (!sound)
				continue;

			sound.SetParent(owner);
			sound.SetPosition(position);
			m_Params.Insert(params);
			m_Builders.Insert(builder);
			m_Objects.Insert(sound);
		}
	}

	void ~geb_UFOSound()
	{
		Stop();
	}

	bool IsPlaying()
	{
		return m_Waves.Count() > 0;
	}

	void Update(bool running, float rpm, float thrust, float speed, float doors, float campos)
	{
		if (!running)
		{
			Stop();
			return;
		}

		m_Values[0] = rpm;
		m_Values[1] = thrust;
		m_Values[2] = speed;
		m_Values[3] = 1;
		m_Values[4] = doors;
		m_Values[5] = campos;
		foreach (SoundObject sound : m_Objects)
			sound.UpdateVariables(m_Values);

		if (IsPlaying())
			return;

		for (int i = 0; i < m_Objects.Count(); i++)
		{
			AbstractWave wave = g_Game.GetSoundScene().Play3D(m_Objects[i], m_Builders[i]);
			if (!wave)
				continue;

			wave.Loop(true);
			m_Waves.Insert(wave);
		}
	}

	void Stop()
	{
		foreach (AbstractWave wave : m_Waves)
		{
			if (wave)
				wave.Stop();
		}

		m_Waves.Clear();
	}
};

//! The green glow under the craft while the drive is on; geb_UFO.c dims it with the power.
class geb_UFOLight extends PointLightBase
{
	static const float BRIGHTNESS = 3.5;

	void geb_UFOLight()
	{
		SetVisibleDuringDaylight(true);
		SetRadiusTo(16);
		SetBrightnessTo(BRIGHTNESS);
		SetCastShadow(false);
		SetFlareVisible(false);
		SetDiffuseColor(0.42, 1.0, 0.08);
		SetAmbientColor(0.42, 1.0, 0.08);
		FadeIn(0.8);
		SetFadeOutTime(0.8);
	}
};

//! A faint green light in the cockpit from the screens and the trim.
class geb_UFOCockpitLight extends PointLightBase
{
	void geb_UFOCockpitLight()
	{
		SetVisibleDuringDaylight(false);
		SetRadiusTo(2.6);
		SetBrightnessTo(0.7);
		SetCastShadow(false);
		SetFlareVisible(false);
		SetDiffuseColor(0.45, 1.0, 0.12);
		SetAmbientColor(0.45, 1.0, 0.12);
		FadeIn(0.8);
		SetFadeOutTime(0.5);
	}
};
