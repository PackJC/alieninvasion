/*
	geb_UFO - the flyable saucer (data/ufo/config.cpp; model, textures and sounds from tools/alienufo).

	A bare Transport: DayZ's vehicle base with the crew, seats and network
	movement, but no car simulation underneath. This script simulates all of
	it every physics step, the way GebsPlanes flies its planes:

	- the drive: off, powering up while the pilot holds the start action, on.
	  Powered, it holds the craft up against gravity, flies it towards the
	  speed and climb the pilot asks for and brakes it to a hover when the keys
	  are let go (the "inertial dampers"); it turns on the spot, leans into its
	  own acceleration and banks into turns, and levels itself;
	- the landing gear: three legs that fold up under the hull and down again
	  (G). Down, each leg is a ray from its foot with a spring and damper, and
	  its pad grips the ground. Up, the craft rests on its hull;
	- unpowered, it falls like a stone with a little air drag.

	Nobody at the controls with the drive on (the pilot jumped out, or left it
	running), it lowers its legs, settles to the ground and idles there, as a
	car left running does, until geb_UFOFlight idleOffTime switches it off.

	Multiplayer: as with every DayZ vehicle, the pilot's client simulates the
	craft and so does the server, from the same moves, correcting the pilot's
	machine wherever the two part company. So the pilot's controls ride in the
	network move (geb_UFOMove), the drive's and the gear's state in the owner
	state a correction rewinds (geb_UFOOwnerState), every force goes in through
	Transport.ApplyForce / ApplyTorque / ApplyForceAt (the deterministic
	vehicle physics a correction replays), and nothing reads what each machine
	keeps its own copy of, such as the weather.

	The drive's switch is the exception: the power actions throw it on the
	server and on the pilot's machine alike, as vanilla's engine actions do
	for a vehicle without moves, so powering up never waits on a move getting
	through.

	A body that has sat still falls asleep, and asleep it gets no simulation
	steps at all: nothing would power it up, fly it or land it. So the craft
	keeps itself awake while someone is at the controls or the drive is on.

	Its numbers are in the config's geb_UFOFlight class; the points it needs
	(legs, the door, lights) are memory points in the model.

	Clients show it: lit while the drive is on (the glow, the screens, a hex
	field turning inside the glass), throwing dust off the ground below and
	shedding green sparks from the emitter, and as it's damaged a scorched
	hull, cracked glass and smoke.
*/

//! What the pilot is doing: everything the simulation depends on that isn't already in the physics state.
class geb_UFOControls
{
	float m_Forward;	//!< -1 back .. 1 ahead
	float m_Strafe;		//!< -1 left .. 1 right
	float m_Yaw;		//!< -1 turn left .. 1 turn right
	float m_Climb;		//!< -1 down .. 1 up
	bool m_Boost;
	bool m_GearDown = true;
	bool m_Power;			//!< the drive's switch (thrown by the power actions on each machine, not in the move)
	bool m_Starter;			//!< the start action, while it is held (likewise)
	bool m_StarterLatched;	//!< the start action ran its course: the drive spools up the rest of the way by itself

	void ResetSticks()
	{
		m_Forward = 0;
		m_Strafe = 0;
		m_Yaw = 0;
		m_Climb = 0;
		m_Boost = false;
		m_Starter = false;
		m_StarterLatched = false;
	}
};

//! The vehicle's network move, with the pilot's sticks and gear added (the drive's switch is not in it).
class geb_UFOMove : TransportMove
{
	float m_Forward;
	float m_Strafe;
	float m_Yaw;
	float m_Climb;
	bool m_Boost;
	bool m_GearDown;

	void Store(geb_UFOControls c)
	{
		m_Forward = c.m_Forward;
		m_Strafe = c.m_Strafe;
		m_Yaw = c.m_Yaw;
		m_Climb = c.m_Climb;
		m_Boost = c.m_Boost;
		m_GearDown = c.m_GearDown;
	}

	void Restore(geb_UFOControls c)
	{
		c.m_Forward = m_Forward;
		c.m_Strafe = m_Strafe;
		c.m_Yaw = m_Yaw;
		c.m_Climb = m_Climb;
		c.m_Boost = m_Boost;
		c.m_GearDown = m_GearDown;
	}

	protected override event void Write(PawnMoveWriter ctx, PawnMove prev)
	{
		super.Write(ctx, prev);

		ctx.Write(m_Forward);
		ctx.Write(m_Strafe);
		ctx.Write(m_Yaw);
		ctx.Write(m_Climb);
		ctx.Write(m_Boost);
		ctx.Write(m_GearDown);
	}

	protected override event void Read(PawnMoveReader ctx, PawnMove prev)
	{
		super.Read(ctx, prev);

		ctx.Read(m_Forward);
		ctx.Read(m_Strafe);
		ctx.Read(m_Yaw);
		ctx.Read(m_Climb);
		ctx.Read(m_Boost);
		ctx.Read(m_GearDown);
	}

	protected override event int EstimateMaximumSize()
	{
		// four floats and two flags, with room for the serializer's own framing
		return super.EstimateMaximumSize() + 56;
	}
};

//! Simulation state a network correction has to rewind along with the body.
class geb_UFOOwnerState : TransportOwnerState
{
	int m_PowerState;
	float m_SpoolTime;
	float m_Power;
	float m_GearPhase;
	float m_SettledTime;

	protected override event void Write(PawnStateWriter ctx)
	{
		super.Write(ctx);

		ctx.Write(m_PowerState);
		ctx.Write(m_SpoolTime);
		ctx.Write(m_Power);
		ctx.Write(m_GearPhase);
		ctx.Write(m_SettledTime);
	}

	protected override event void Read(PawnStateReader ctx)
	{
		super.Read(ctx);

		ctx.Read(m_PowerState);
		ctx.Read(m_SpoolTime);
		ctx.Read(m_Power);
		ctx.Read(m_GearPhase);
		ctx.Read(m_SettledTime);
	}

	protected override event int EstimateMaximumSize()
	{
		return super.EstimateMaximumSize() + 40;
	}
};

enum geb_UFOPowerState
{
	OFF,
	STARTING,	//!< powering up while the start action is held
	ON
};

class geb_UFO : Transport
{
	static const float GRAVITY = 9.81;
	static const float STICK_RATE = 2.5;		//!< keyboard sticks: travel per second towards where they're pushed
	static const float STICK_RETURN = 4.0;		//!< and back to the middle when let go
	static const float SYNC_INTERVAL = 0.1;		//!< s between updates of what other players see and hear
	static const float SYNC_STEP = 0.02;
	static const float LIT_POWER = 0.3;			//!< the lights come on above this much power
	static const float AIR_DENSITY = 1.225;
	static const int STORE_VERSION = 1;

	static const string GLOW_LIT_TEXTURE = "alieninvasion\\data\\ufo\\geb_ufo_glow_co.paa";
	static const string GLOW_LIT_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_glow.rvmat";
	static const string GLOW_DARK_TEXTURE = "alieninvasion\\data\\ufo\\geb_ufo_glow_off_co.paa";
	static const string GLOW_DARK_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_glow_off.rvmat";
	static const string SCREEN_FRAME_TEXTURE = "alieninvasion\\data\\ufo\\geb_ufo_screens_%1_co.paa";	//!< lit: 00 .. 07
	static const int SCREEN_FRAMES = 8;
	static const float SCREEN_STEP = 0.1667;		//!< s a frame: the displays run at 6 frames a second
	static const float SCREEN_RANGE_SQ = 1600;		//!< (m) the displays only run with a camera within 40 m
	static const string SCREENS_LIT_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_screens.rvmat";
	static const string SCREENS_DARK_TEXTURE = "alieninvasion\\data\\ufo\\geb_ufo_screens_off_co.paa";
	static const string SCREENS_DARK_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_screens_off.rvmat";
	static const string HULL_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_hull.rvmat";
	static const string HULL_DAMAGE_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_hull_damage.rvmat";
	static const string HULL_DESTRUCT_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_hull_destruct.rvmat";
	static const string GLASS_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_glass.rvmat";
	static const string GLASS_DAMAGE_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_glass_damage.rvmat";
	static const string GLASS_DESTRUCT_MATERIAL = "alieninvasion\\data\\ufo\\geb_ufo_glass_destruct.rvmat";
	static const string FIELD_TEXTURE = "alieninvasion\\data\\ufo\\geb_ufo_field_ca.paa";
	static const int CAMO_INDEX = 0;			//!< hiddenSelections[] = {"camo", "glow", "screens", "glass", "field", "glass_in"}
	static const int GLOW_INDEX = 1;
	static const int SCREENS_INDEX = 2;
	static const int GLASS_INDEX = 3;
	static const int FIELD_INDEX = 4;
	static const int GLASS_IN_INDEX = 5;		//!< the glass's inside faces: plain tint, cracking with the rest
	static const float FIELD_TURN_TIME = 40;	//!< s for the hex field to turn once round inside the glass
	static const float DUST_HEIGHT = 14;		//!< m above the ground: no downwash dust higher than this
	static const float LOOK_REFRESH_TIME = 0.5;	//!< s of setting the materials again after the canopy stops or a hit

	protected bool m_FlightLoaded;
	protected vector m_Fwd;			//!< model axes: forward (the nose, -Z), up, and to the right
	protected vector m_Up;
	protected vector m_Side;
	protected string m_DoorPoint;
	protected string m_CanopySource;
	protected float m_Reach;		//!< m from the middle within which someone outside can work the canopy

	// the drive
	protected float m_SpoolDuration;
	protected float m_PowerLag;
	protected float m_MaxSpeed;
	protected float m_BoostSpeed;
	protected float m_StrafeSpeed;
	protected float m_Accel;
	protected float m_BoostAccel;
	protected float m_BrakeAccel;
	protected float m_SpeedGain;
	protected float m_ClimbRate;
	protected float m_BoostClimbRate;
	protected float m_SinkRate;
	protected float m_VertAccel;
	protected float m_ClimbGain;
	protected float m_GroundHold;
	protected float m_Ceiling;
	protected float m_YawRate;
	protected float m_YawAccel;
	protected float m_YawGain;
	protected float m_TiltMax;
	protected float m_TiltGain;
	protected float m_AttitudeStiffness;
	protected float m_AttitudeDamping;
	protected float m_InertiaTilt;
	protected float m_InertiaYaw;
	protected float m_DragArea;
	protected float m_GearTime;
	protected float m_AutoLandSink;
	protected float m_IdleOffTime;
	protected float m_CrashImpulse;
	protected float m_CrashDamage;

	// the legs
	protected ref array<ref geb_UFOLeg> m_Legs;

	// state
	protected ref geb_UFOControls m_FlightControls;
	protected int m_PowerState;			//!< geb_UFOPowerState
	protected float m_SpoolTime;
	protected float m_Power;			//!< 0 .. 1, how much the drive is giving
	protected float m_GearPhase;		//!< 0 down .. 1 folded up
	protected float m_SettledTime;		//!< s idling on the ground, drive on, with nobody flying it
	protected float m_GroundShare;		//!< share of the weight the legs carry, this step
	protected float m_Effort;			//!< 0 .. 1, how hard the drive is working (for the sound)
	protected float m_SyncTimer;
	protected bool m_KeptAwake;			//!< the body held awake here (UpdateSleep)

	// pilot input
	protected bool m_GearKeyHeld;
	protected ref map<string, UAInput> m_FlightInputs;

	// what other players see and hear (server -> clients)
	protected float m_NetPower;
	protected int m_NetPowerState;
	protected float m_NetGear;
	protected bool m_NetGearDown = true;
	protected bool m_NetSwitch;				//!< the drive's switch (geb_UFOControls.m_Power)
	protected int m_NetLookRefresh;			//!< counts hits round 0..15: each one, clients set their materials again
	protected bool m_TookOver;				//!< this client is the pilot and has taken the craft over (TakeOver)
	protected float m_NetEffort;

	// what this client shows and plays
	protected float m_SpinTurns;
	protected float m_HoloTurns;
	protected float m_RadarTurns;
	protected int m_ScreenFrame;		//!< the lit displays' frame shown (SCREEN_FRAME_TEXTURE)
	protected float m_ScreenTime;
	protected bool m_LitShown;
	protected bool m_LitSet;
	protected int m_HullLook = -1;		//!< the hull's material shown: 0 new, 1 damaged, 2 ruined (-1: to set)
	protected int m_GlassLook = -1;
	protected bool m_FieldShown;
	protected bool m_FieldSet;
	protected float m_FieldTurns;
	protected float m_CanopyShown;		//!< the canopy's phase last frame: moving, it resets the materials
	protected float m_LookRefreshTime;	//!< s left of setting the materials again every frame (RefreshLooks)
	protected int m_LookRefreshSeen;
	protected bool m_FieldFlickerOn;
	protected float m_FieldFlickerTime;
	protected ParticleSource m_DustFx;
	protected ParticleSource m_MotesFx;
	protected ParticleSource m_SmokeFx;
	protected float m_DustRate;			//!< the rate each effect was last set to (-1: just started)
	protected float m_MotesRate;
	protected float m_SmokeRate;
	protected int m_HeardPowerState;
	protected bool m_HeardGearDown = true;
	protected bool m_HeardSet;
	protected ref geb_UFOSound m_LoopSound;
	protected ref TStringArray m_LoopSoundSets;
	protected string m_PowerUpSound;
	protected string m_PowerDownSound;
	protected string m_CanopyOpenSound;
	protected string m_CanopyCloseSound;
	protected string m_GearDownSound;
	protected string m_GearUpSound;
	protected geb_UFOLight m_UnderLight;
	protected geb_UFOCockpitLight m_CockpitLight;

	// vehicle actions (get in, the canopy), shared by every craft of a type
	static ref map<typename, ref TInputActionMap> s_geb_UFOActionMaps = new map<typename, ref TInputActionMap>();
	protected TInputActionMap m_InputActionMap;
	protected bool m_ActionsInitialized;

	// test readout (geb_UFOFlight debugLog): a line a second in the pilot's and the server's script logs
	protected bool m_DebugLog;
	protected float m_DebugTimer;
	protected int m_DebugSteps;			//!< simulation steps since the last line
	protected int m_DebugMovesOut;		//!< moves this machine sent (the pilot's)
	protected int m_DebugMovesIn;		//!< moves this machine flew from the pilot (the server)

	void geb_UFO()
	{
		// no car simulation makes the body: the craft makes its own from its Geometry LOD's pieces and masses
		CreateDynamicPhysics(PhxInteractionLayers.VEHICLE);
		dBodyActive(this, ActiveState.ACTIVE);

		SetEventMask(EntityEvent.SIMULATE);
		SetEventMask(EntityEvent.POSTSIMULATE);
		SetEventMask(EntityEvent.FRAME);
		SetEventMask(EntityEvent.CONTACT);

		m_Legs = new array<ref geb_UFOLeg>();
		m_FlightControls = new geb_UFOControls();
		m_FlightInputs = new map<string, UAInput>();
		m_LoopSoundSets = new TStringArray();

		RegisterNetSyncVariableFloat("m_NetPower", 0, 1, 2);
		RegisterNetSyncVariableInt("m_NetPowerState", 0, 2);
		RegisterNetSyncVariableFloat("m_NetGear", 0, 1, 2);
		RegisterNetSyncVariableBool("m_NetGearDown");
		RegisterNetSyncVariableBool("m_NetSwitch");
		RegisterNetSyncVariableInt("m_NetLookRefresh", 0, 15);
		RegisterNetSyncVariableFloat("m_NetEffort", 0, 1, 2);
	}

	void ~geb_UFO()
	{
		DestroyLights();
	}

	override void EEInit()
	{
		super.EEInit();

		LoadFlight();
	}

	override void EEDelete(EntityAI parent)
	{
		DestroyLights();
		StopEffects();
		if (m_LoopSound)
			m_LoopSound.Stop();

		super.EEDelete(parent);
	}

	//--------------------------------------------------------------------------
	// config and model
	//--------------------------------------------------------------------------

	protected string FlightPath()
	{
		return "CfgVehicles " + GetType() + " geb_UFOFlight";
	}

	protected float FlightFloat(string key)
	{
		return g_Game.ConfigGetFloat(FlightPath() + " " + key);
	}

	protected string FlightText(string key)
	{
		string value;
		g_Game.ConfigGetText(FlightPath() + " " + key, value);
		return value;
	}

	protected vector FlightPoint(string key)
	{
		string point = FlightText(key);
		if (point == "" || !MemoryPointExists(point))
		{
			ErrorEx(string.Format("AlienInvasion: %1 has no memory point '%2' (%3)", GetType(), point, key));
			return vector.Zero;
		}

		return GetMemoryPointPos(point);
	}

	protected void LoadFlight()
	{
		if (m_FlightLoaded)
			return;

		m_FlightLoaded = true;

		string path = FlightPath();
		if (!g_Game.ConfigIsExisting(path))
		{
			ErrorEx("AlienInvasion: " + GetType() + " has no geb_UFOFlight config class");
			return;
		}

		// forward is from the middle towards the nose point; up is the model's up
		vector middle = FlightPoint("middlePoint");
		vector nose = FlightPoint("nosePoint");
		m_Up = "0 1 0";
		m_Fwd = nose - middle;
		m_Fwd[1] = 0;
		m_Fwd.Normalize();
		m_Side = Cross(m_Up, m_Fwd);
		m_DoorPoint = FlightText("doorPoint");
		m_CanopySource = FlightText("canopySource");
		m_Reach = FlightFloat("reach");

		m_SpoolDuration = Math.Max(FlightFloat("spoolTime"), 0.1);
		m_PowerLag = Math.Max(FlightFloat("powerLag"), 0.05);
		m_MaxSpeed = FlightFloat("maxSpeed");
		m_BoostSpeed = FlightFloat("boostSpeed");
		m_StrafeSpeed = FlightFloat("strafeSpeed");
		m_Accel = FlightFloat("accel");
		m_BoostAccel = FlightFloat("boostAccel");
		m_BrakeAccel = FlightFloat("brakeAccel");
		m_SpeedGain = FlightFloat("speedGain");
		m_ClimbRate = FlightFloat("climbRate");
		m_BoostClimbRate = FlightFloat("boostClimbRate");
		m_SinkRate = FlightFloat("sinkRate");
		m_VertAccel = FlightFloat("vertAccel");
		m_ClimbGain = FlightFloat("climbGain");
		m_GroundHold = FlightFloat("groundHold");
		m_Ceiling = FlightFloat("ceiling");
		m_YawRate = FlightFloat("yawRate") * Math.DEG2RAD;
		m_YawAccel = FlightFloat("yawAccel") * Math.DEG2RAD;
		m_YawGain = FlightFloat("yawGain");
		m_TiltMax = FlightFloat("tiltMax") * Math.DEG2RAD;
		m_TiltGain = FlightFloat("tiltGain");
		m_AttitudeStiffness = FlightFloat("attitudeStiffness");
		m_AttitudeDamping = FlightFloat("attitudeDamping");
		m_InertiaTilt = FlightFloat("inertiaTilt");
		m_InertiaYaw = FlightFloat("inertiaYaw");
		m_DragArea = FlightFloat("dragArea");
		m_GearTime = Math.Max(FlightFloat("gearTime"), 0.1);
		m_AutoLandSink = FlightFloat("autoLandSink");
		m_IdleOffTime = FlightFloat("idleOffTime");
		m_CrashImpulse = FlightFloat("crashImpulse");
		m_CrashDamage = FlightFloat("crashDamage");
		m_DebugLog = g_Game.ConfigGetInt(path + " debugLog") != 0;

		g_Game.ConfigGetTextArray(path + " loopSounds", m_LoopSoundSets);
		m_PowerUpSound = FlightText("powerUpSound");
		m_PowerDownSound = FlightText("powerDownSound");
		m_CanopyOpenSound = FlightText("canopyOpenSound");
		m_CanopyCloseSound = FlightText("canopyCloseSound");
		m_GearDownSound = FlightText("gearDownSound");
		m_GearUpSound = FlightText("gearUpSound");

		string legs = path + " Legs";
		int count = g_Game.ConfigGetChildrenCount(legs);
		for (int i = 0; i < count; i++)
		{
			string legName;
			g_Game.ConfigGetChildName(legs, i, legName);
			geb_UFOLeg leg = geb_UFOLeg.Load(legs + " " + legName, legName, this);
			if (leg)
				m_Legs.Insert(leg);
		}
	}

	static vector Cross(vector a, vector b)
	{
		return Vector(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
	}

	//! A horizontal unit vector along v, or along fallback when v is near vertical.
	static vector Level(vector v, vector fallback)
	{
		vector flat = Vector(v[0], 0, v[2]);
		if (flat.LengthSq() < 0.0001)
			flat = Vector(fallback[0], 0, fallback[2]);
		if (flat.LengthSq() < 0.0001)
			return "0 0 1";
		flat.Normalize();
		return flat;
	}

	//--------------------------------------------------------------------------
	// what a Transport has to say for itself
	//--------------------------------------------------------------------------

	override string GetVehicleType()
	{
		return "VehicleTypeGebUFO";
	}

	override int Get3rdPersonCameraType()
	{
		return DayZPlayerCameras.DAYZCAMERA_3RD_VEHICLE;
	}

	// one seat, low with the legs stretched ahead under the console (the Gunter 2's)
	override int GetAnimInstance()
	{
		return VehicleAnimInstances.GOLF;
	}

	// The pilot's seat has to keep a car driver's animations: the game gives the controls and the vehicle
	// HUD only to a driver's seat. A driver climbs in and out on the left, which is where the craft is boarded.
	override int GetSeatAnimationType(int posIdx)
	{
		return DayZPlayerConstants.VEHICLESEAT_DRIVER;
	}

	// six metres across: stand well back
	override float GetTransportCameraDistance()
	{
		return 11.0;
	}

	override vector GetTransportCameraOffset()
	{
		return "0 2.2 0";
	}

	//--------------------------------------------------------------------------
	// pilot input (the pilot's client)
	//--------------------------------------------------------------------------

	bool IsLocalPilot()
	{
		Human driver = CrewDriver();
		return driver && driver == g_Game.GetPlayer() && driver.IsControllingVehicle();
	}

	protected float InputValue(string name)
	{
		UAInput input;
		if (!m_FlightInputs.Find(name, input))
		{
			input = GetUApi().GetInputByName(name);
			m_FlightInputs.Insert(name, input);
		}

		if (!input)
			return 0;

		return input.LocalValue();
	}

	//! Keyboard stick: moves towards where it's pushed, springs back when let go.
	protected static float MoveStick(float current, float target, float dt)
	{
		float rate = geb_UFO.STICK_RATE;
		if (target == 0 || target * current < 0)
			rate = geb_UFO.STICK_RETURN;

		float step = rate * dt;
		if (Math.AbsFloat(target - current) <= step)
			return target;
		if (target > current)
			return current + step;
		return current - step;
	}

	protected void ReadPilotInput(float dt)
	{
		geb_UFOControls c = m_FlightControls;

		float fwd = InputValue("UAgebUFOForward") - InputValue("UAgebUFOBack");
		float strafe = InputValue("UAgebUFOStrafeRight") - InputValue("UAgebUFOStrafeLeft");
		float yaw = InputValue("UAgebUFOTurnRight") - InputValue("UAgebUFOTurnLeft");
		float climb = InputValue("UAgebUFOUp") - InputValue("UAgebUFODown");

		c.m_Forward = MoveStick(c.m_Forward, Math.Clamp(fwd, -1, 1), dt);
		c.m_Strafe = MoveStick(c.m_Strafe, Math.Clamp(strafe, -1, 1), dt);
		c.m_Yaw = MoveStick(c.m_Yaw, Math.Clamp(yaw, -1, 1), dt);
		c.m_Climb = MoveStick(c.m_Climb, Math.Clamp(climb, -1, 1), dt);
		c.m_Boost = InputValue("UAgebUFOBoost") > 0.5;

		// the gear: one toggle per press. The legs won't fold while they carry the craft
		bool gearKey = InputValue("UAgebUFOGear") > 0.5;
		if (gearKey && !m_GearKeyHeld)
		{
			if (!c.m_GearDown)
				c.m_GearDown = true;
			else if (m_GroundShare < 0.15)
				c.m_GearDown = false;
		}
		m_GearKeyHeld = gearKey;
	}

	override void OnInput(float dt)
	{
		super.OnInput(dt);

		if (IsLocalPilot())
			ReadPilotInput(dt);
	}

	/*!
		The power actions, on the server and on the pilot's client both (geb_ActionUFO.c). Holding the start
		action throws the switch and turns the drive over, and SimulatePower spools it up. The action is as short
		as a car's start (a seated animation won't last the spool-up), so once it has run its course the starter
		latches and the drive comes on by itself; let go before that and it winds down again.
	*/
	void HoldStarter(bool held)
	{
		geb_UFOControls c = m_FlightControls;
		if (held)
		{
			c.m_Power = true;
			c.m_Starter = true;
			WakeUp();
			DebugNote("starter held");
			return;
		}

		if (!c.m_StarterLatched)
			c.m_Starter = false;

		DebugNote("starter let go, latched " + YesNo(c.m_StarterLatched) + ", drive " + typename.EnumToString(geb_UFOPowerState, m_PowerState));
	}

	void LatchStarter()
	{
		if (m_FlightControls.m_Starter)
			m_FlightControls.m_StarterLatched = true;

		DebugNote("starter latched");
	}

	//! The drive is on, or coming on by itself after a start.
	bool IsPoweringOn()
	{
		return GetPowerState() == geb_UFOPowerState.ON || m_FlightControls.m_StarterLatched;
	}

	void SwitchOff()
	{
		m_FlightControls.m_Power = false;
		m_FlightControls.m_Starter = false;
		m_FlightControls.m_StarterLatched = false;

		DebugNote("switched off");
	}

	//! The pilot gone: the sticks centre. The drive stays on, so the craft can land itself (AutoLand).
	override void OnDriverExit(Human player)
	{
		super.OnDriverExit(player);

		m_FlightControls.ResetSticks();
	}

	//--------------------------------------------------------------------------
	// network move: the controls go with the vehicle's own inputs
	//--------------------------------------------------------------------------

	protected override event typename GetMoveType()
	{
		return geb_UFOMove;
	}

	protected override event typename GetOwnerStateType()
	{
		return geb_UFOOwnerState;
	}

	protected override event void ObtainMove(/*inout*/ PawnMove pMove)
	{
		super.ObtainMove(pMove);

		geb_UFOMove move = geb_UFOMove.Cast(pMove);
		if (move)
			move.Store(m_FlightControls);
		m_DebugMovesOut++;
	}

	protected override event void ConsumeMove(PawnMove pMove)
	{
		super.ConsumeMove(pMove);

		geb_UFOMove move = geb_UFOMove.Cast(pMove);
		if (move)
			move.Restore(m_FlightControls);
		m_DebugMovesIn++;
	}

	protected override event bool ReplayMove(PawnMove pMove)
	{
		geb_UFOMove move = geb_UFOMove.Cast(pMove);
		if (move)
			move.Restore(m_FlightControls);

		return super.ReplayMove(pMove);
	}

	//! After a correction the pilot's machine re-runs its moves since; each step has to fly on the controls it
	//! was first flown on
	protected override event void SimulateMove(PawnMove pMove)
	{
		super.SimulateMove(pMove);

		geb_UFOMove move = geb_UFOMove.Cast(pMove);
		if (move)
			move.Restore(m_FlightControls);
	}

	protected override event void ObtainState(/*inout*/ PawnOwnerState pState)
	{
		super.ObtainState(pState);

		geb_UFOOwnerState state = geb_UFOOwnerState.Cast(pState);
		if (state)
		{
			state.m_PowerState = m_PowerState;
			state.m_SpoolTime = m_SpoolTime;
			state.m_Power = m_Power;
			state.m_GearPhase = m_GearPhase;
			state.m_SettledTime = m_SettledTime;
		}
	}

	protected override event void RewindState(PawnOwnerState pState, /*inout*/ PawnMove pMove, inout NetworkRewindType pRewindType)
	{
		super.RewindState(pState, pMove, pRewindType);

		// an additive rewind keeps the simulation where it is and adds the difference, as the player's own
		// does: the state stays
		if (pRewindType == NetworkRewindType.ADDITIVE)
			return;

		geb_UFOOwnerState state = geb_UFOOwnerState.Cast(pState);
		if (state)
		{
			m_PowerState = state.m_PowerState;
			m_SpoolTime = state.m_SpoolTime;
			m_Power = state.m_Power;
			m_GearPhase = state.m_GearPhase;
			m_SettledTime = state.m_SettledTime;
		}
	}

	//--------------------------------------------------------------------------
	// simulation (the server and the pilot's client)
	//--------------------------------------------------------------------------

	override void EOnSimulate(IEntity other, float dt)
	{
		if (IsProxy() || dt <= 0)
			return;

		if (!m_FlightLoaded)
			LoadFlight();

		m_DebugSteps++;

		if (!CrewDriver())
			AutoLand();

		SimulatePower(dt);
		SimulateGear(dt);
		SimulateLegs(dt);
		SimulateFlight(dt);
	}

	//! Whether the drive can run: the craft isn't ruined.
	bool CanPower()
	{
		return !IsDamageDestroyed() && GetHealthLevel("") < GameConstants.STATE_RUINED;
	}

	//! Nobody flying it with the drive on: sticks centred, legs down, sink to the ground and idle there
	//! (SimulatePower switches it off after idleOffTime).
	protected void AutoLand()
	{
		geb_UFOControls c = m_FlightControls;
		c.ResetSticks();
		if (m_PowerState == geb_UFOPowerState.ON)
		{
			c.m_GearDown = true;
			c.m_Climb = -m_AutoLandSink;
		}
	}

	protected void SimulatePower(float dt)
	{
		geb_UFOControls c = m_FlightControls;
		switch (m_PowerState)
		{
			case geb_UFOPowerState.OFF:
				if (c.m_Power && c.m_Starter && CanPower())
				{
					m_PowerState = geb_UFOPowerState.STARTING;
					m_SpoolTime = 0;
				}
				break;

			case geb_UFOPowerState.STARTING:
				if (!c.m_Power || !c.m_Starter || !CanPower())
				{
					m_PowerState = geb_UFOPowerState.OFF;
					c.m_Starter = false;
					c.m_StarterLatched = false;
					break;
				}

				m_SpoolTime += dt;
				if (m_SpoolTime >= m_SpoolDuration)
				{
					// on: the starter has done its job
					m_PowerState = geb_UFOPowerState.ON;
					c.m_Starter = false;
					c.m_StarterLatched = false;
				}
				break;

			case geb_UFOPowerState.ON:
				if (!c.m_Power || !CanPower())
					m_PowerState = geb_UFOPowerState.OFF;
				break;
		}

		// left running with nobody at the controls, it idles where it has landed (on its legs, which carry
		// 1 - groundHold of it parked, or on its hull), as a car left running does; it switches itself off after
		// idleOffTime of that, so forgotten ones don't hum on for good
		if (m_PowerState == geb_UFOPowerState.ON && !CrewDriver() && IsLanded() && GetVelocity(this).LengthSq() < 0.25)
		{
			m_SettledTime += dt;
			if (m_IdleOffTime > 0 && m_SettledTime > m_IdleOffTime)
			{
				c.m_Power = false;
				m_PowerState = geb_UFOPowerState.OFF;
			}
		}
		else
		{
			m_SettledTime = 0;
		}

		float target = 0;
		if (m_PowerState == geb_UFOPowerState.ON)
			target = 1;
		else if (m_PowerState == geb_UFOPowerState.STARTING)
			target = 0.25 * m_SpoolTime / m_SpoolDuration;

		m_Power += (target - m_Power) * Math.Clamp(dt / m_PowerLag, 0, 1);
		if (target == 0 && m_Power < 0.002)
			m_Power = 0;
	}

	//! The legs fold up and down at a steady rate; they won't fold while they carry the craft.
	protected void SimulateGear(float dt)
	{
		float target = 1;
		if (m_FlightControls.m_GearDown)
			target = 0;

		if (target > m_GearPhase && m_GroundShare > 0.15)
			target = m_GearPhase;

		float step = dt / m_GearTime;
		if (Math.AbsFloat(target - m_GearPhase) <= step)
			m_GearPhase = target;
		else if (target > m_GearPhase)
			m_GearPhase += step;
		else
			m_GearPhase -= step;
	}

	bool IsGearDown()
	{
		return GetGearPhase() < 0.02;
	}

	protected void SimulateLegs(float dt)
	{
		m_GroundShare = 0;
		vector upW = VectorToParent(m_Up);
		float load = 0;
		bool down = m_GearPhase < 0.02;
		foreach (geb_UFOLeg leg : m_Legs)
		{
			if (down)
			{
				leg.Simulate(this, dt, upW);
				load += leg.m_Load;
			}
			else
			{
				leg.Lift();
			}
		}

		float weight = dBodyGetMass(this) * GRAVITY;
		if (weight > 0)
			m_GroundShare = Math.Clamp(load / weight, 0, 1);
	}

	/*!
		The drive. Powered, it gives what it takes to fly the craft towards the speed and climb the pilot asks
		for, up to its limits, and the craft leans into that push and banks into its turns. Sat on its legs with
		nothing asking it up, it lets the legs take most of the weight. Unpowered, only the air holds it back.
	*/
	protected void SimulateFlight(float dt)
	{
		geb_UFOControls c = m_FlightControls;
		float mass = dBodyGetMass(this);
		vector vel = GetVelocity(this);
		vector upW = VectorToParent(m_Up);
		vector fwdW = VectorToParent(m_Fwd);
		vector sideW = VectorToParent(m_Side);
		float p = m_Power;
		m_Effort = 0;

		if (p <= 0.001)
		{
			float speed = vel.Length();
			if (speed > 1 && m_DragArea > 0)
				ApplyForce(vel * (-0.5 * AIR_DENSITY * m_DragArea * speed));
			return;
		}

		// Sat on its legs with nothing asking it up, it is parked: the legs carry most of it and the drive
		// neither pushes nor turns it (it would skate on its pads). Lift off first. Lifting, it gets its push
		// and its turn back as the legs unload.
		bool parked = m_GroundShare > 0.05 && c.m_Climb <= 0.05;
		float airborne = 1 - m_GroundShare;
		if (parked)
			airborne = 0;
		vector fwdH = Level(fwdW, upW * -1);
		vector sideH = Level(sideW, upW * -1);

		float vmax = m_MaxSpeed;
		float accel = m_Accel;
		float climbRate = m_ClimbRate;
		if (c.m_Boost)
		{
			vmax = m_BoostSpeed;
			accel = m_BoostAccel;
			climbRate = m_BoostClimbRate;
		}

		// across the ground: towards the velocity asked for, braking to a hover with the keys let go
		vector want = fwdH * (c.m_Forward * vmax) + sideH * (c.m_Strafe * m_StrafeSpeed);
		vector velH = Vector(vel[0], 0, vel[2]);
		vector push = (want - velH) * m_SpeedGain;
		float limit = m_BrakeAccel;
		if (Math.AbsFloat(c.m_Forward) + Math.AbsFloat(c.m_Strafe) > 0.05)
			limit = accel;
		float pushLen = push.Length();
		if (pushLen > limit)
			push = push * (limit / pushLen);

		// up and down: hold the height with the keys let go
		float climbWant = c.m_Climb * m_SinkRate;
		if (c.m_Climb > 0)
			climbWant = c.m_Climb * climbRate;
		if (GetPosition()[1] > m_Ceiling && climbWant > 0)
			climbWant = 0;
		float aV = Math.Clamp((climbWant - vel[1]) * m_ClimbGain, -m_VertAccel, m_VertAccel);
		float lift = mass * (GRAVITY + aV);
		if (parked)
			lift = mass * GRAVITY * m_GroundHold;
		lift = Math.Max(lift, 0) * p;

		ApplyForce(Vector(0, lift, 0) + push * (mass * p * airborne));

		// turning on the spot
		vector w = dBodyGetAngularVelocity(this);
		float wy = vector.Dot(w, upW);
		float aYaw = Math.Clamp((c.m_Yaw * m_YawRate - wy) * m_YawGain, -m_YawAccel, m_YawAccel);
		vector torque = upW * (aYaw * m_InertiaYaw * airborne);

		// level, leaning into its own push: the turns' push bends it into a bank
		vector lean = push * (m_TiltGain / GRAVITY * airborne);
		float leanLen = lean.Length();
		float maxLean = Math.Tan(m_TiltMax);
		if (leanLen > maxLean)
			lean = lean * (maxLean / leanLen);
		vector upWant = Vector(lean[0], 1, lean[2]);
		upWant.Normalize();
		vector tiltErr = Cross(upW, upWant);
		vector wTilt = w - upW * wy;
		vector aTilt = tiltErr * m_AttitudeStiffness - wTilt * m_AttitudeDamping;
		torque = torque + aTilt * (m_InertiaTilt * airborne);

		ApplyTorque(torque * p);

		if (parked)
			aV = 0;
		m_Effort = Math.Clamp(pushLen * airborne / Math.Max(m_BoostAccel, 1) + Math.AbsFloat(aV) / Math.Max(m_VertAccel, 1) * 0.6, 0, 1);
	}

	//! The drive's state here: simulated, or as the server sends it.
	int GetPowerState()
	{
		if (IsProxy())
			return m_NetPowerState;

		return m_PowerState;
	}

	float GetPowerLevel()
	{
		if (IsProxy())
			return m_NetPower;

		return m_Power;
	}

	float GetGearPhase()
	{
		if (IsProxy())
			return m_NetGear;

		return m_GearPhase;
	}

	bool IsGearSetDown()
	{
		if (IsProxy())
			return m_NetGearDown;

		return m_FlightControls.m_GearDown;
	}

	//! Sat on its legs, or (gear up) barely moving close above the ground.
	bool IsLanded()
	{
		if (m_GroundShare > 0.3)
			return true;

		vector pos = GetPosition();
		float height = pos[1] - g_Game.SurfaceY(pos[0], pos[2]);
		return height < 1.2 && GetVelocity(this).LengthSq() < 1.0;
	}

	//--------------------------------------------------------------------------
	// server: damage, what other players see
	//--------------------------------------------------------------------------

	override void EOnPostSimulate(IEntity other, float timeSlice)
	{
		super.EOnPostSimulate(other, timeSlice);

		if (!g_Game.IsServer())
			return;

		m_SyncTimer += timeSlice;
		if (m_SyncTimer >= SYNC_INTERVAL)
		{
			m_SyncTimer = 0;
			SyncState();
		}
	}

	protected void SyncState()
	{
		bool dirty = false;

		if (Math.AbsFloat(m_NetPower - m_Power) >= SYNC_STEP || (m_Power == 0 && m_NetPower != 0))
		{
			m_NetPower = m_Power;
			dirty = true;
		}
		if (m_NetPowerState != m_PowerState)
		{
			m_NetPowerState = m_PowerState;
			dirty = true;
		}
		if (Math.AbsFloat(m_NetGear - m_GearPhase) >= SYNC_STEP || ((m_GearPhase == 0 || m_GearPhase == 1) && m_NetGear != m_GearPhase))
		{
			m_NetGear = m_GearPhase;
			dirty = true;
		}
		if (m_NetGearDown != m_FlightControls.m_GearDown)
		{
			m_NetGearDown = m_FlightControls.m_GearDown;
			dirty = true;
		}
		if (m_NetSwitch != m_FlightControls.m_Power)
		{
			m_NetSwitch = m_FlightControls.m_Power;
			dirty = true;
		}
		if (Math.AbsFloat(m_NetEffort - m_Effort) >= 0.05)
		{
			m_NetEffort = m_Effort;
			dirty = true;
		}

		if (dirty)
			SetSynchDirty();
	}

	//! A knock harder than crashImpulse damages the hull, more the harder it is.
	override protected void EOnContact(IEntity other, Contact extra)
	{
		super.EOnContact(other, extra);

		if (!g_Game.IsServer() || m_CrashDamage <= 0 || extra.Impulse < m_CrashImpulse)
			return;

		vector hitPos = WorldToModel(extra.Position);
		ProcessDirectDamage(DamageType.CUSTOM, null, "Hull", "EnviroDmg", hitPos, (extra.Impulse - m_CrashImpulse) * m_CrashDamage);
		dBodyActive(this, ActiveState.ACTIVE);
	}

	override void OnStoreSave(ParamsWriteContext ctx)
	{
		super.OnStoreSave(ctx);

		ctx.Write(STORE_VERSION);
		ctx.Write(m_FlightControls.m_GearDown);
		ctx.Write(GetAnimationPhase(m_CanopySource));
	}

	override bool OnStoreLoad(ParamsReadContext ctx, int version)
	{
		if (!super.OnStoreLoad(ctx, version))
			return false;

		int mine;
		if (!ctx.Read(mine))
			return false;

		bool gearDown;
		if (!ctx.Read(gearDown))
			return false;

		float canopy;
		if (!ctx.Read(canopy))
			return false;

		LoadFlight();
		m_FlightControls.m_GearDown = gearDown;
		m_GearPhase = 1;
		if (gearDown)
			m_GearPhase = 0;
		if (m_CanopySource != "")
			SetAnimationPhase(m_CanopySource, canopy);
		return true;
	}

	override void AfterStoreLoad()
	{
		super.AfterStoreLoad();

		m_NetGear = m_GearPhase;
		m_NetGearDown = m_FlightControls.m_GearDown;
		SetSynchDirty();
	}

	//--------------------------------------------------------------------------
	// for the HUD (scripts/5_mission/gui/geb_UFOHud.c)
	//--------------------------------------------------------------------------

	float GetSpeedKmh()
	{
		return GetVelocity(this).Length() * 3.6;
	}

	bool IsBoosting()
	{
		return m_FlightControls.m_Boost;
	}

	bool IsCanopyOpen()
	{
		return m_CanopySource != "" && GetAnimationPhase(m_CanopySource) > 0.9;
	}

	bool IsCanopyShut()
	{
		return m_CanopySource == "" || GetAnimationPhase(m_CanopySource) < 0.01;
	}

	//--------------------------------------------------------------------------
	// test readout (geb_UFOFlight debugLog=1)
	//--------------------------------------------------------------------------

	protected static string Num(float value, int decimals = 0)
	{
		float scale = Math.Pow(10, decimals);
		float rounded = Math.Round(value * scale) / scale;
		return rounded.ToString();
	}

	protected static string YesNo(bool value)
	{
		if (value)
			return "yes";

		return "no";
	}

	//! Which machine a test line comes from.
	protected string DebugMachine()
	{
		if (g_Game.IsServer())
			return "server";

		return "pilot";
	}

	//! A test line when something happens to the drive (the power actions).
	void DebugNote(string what)
	{
		if (m_DebugLog)
			Print("AlienInvasion UFO (" + DebugMachine() + "): " + what);
	}

	/*!
		Once a second while someone is at the controls or the drive is on, a line in the server's and the pilot's
		script logs: who simulates the craft and how (steps and moves in the last second, awake), then the drive,
		speed and climb, height, the legs and the controls.
	*/
	protected void DebugReadout(float dt)
	{
		m_DebugTimer += dt;
		if (m_DebugTimer < 1)
			return;

		m_DebugTimer = 0;
		int steps = m_DebugSteps;
		int movesOut = m_DebugMovesOut;
		int movesIn = m_DebugMovesIn;
		m_DebugSteps = 0;
		m_DebugMovesOut = 0;
		m_DebugMovesIn = 0;

		if (!g_Game.IsServer() && !IsLocalPilot())
			return;
		if (!CrewDriver() && m_PowerState == geb_UFOPowerState.OFF)
			return;

		string net = "AlienInvasion UFO (" + DebugMachine() + "): owner " + YesNo(IsOwner()) + ", authority " + YesNo(IsAuthority()) + ", proxy " + YesNo(IsProxy());
		net += ", strategy " + typename.EnumToString(NetworkMoveStrategy, GetNetworkMoveStrategy()) + ", awake " + YesNo(dBodyIsActive(this));
		net += " | last second: " + steps + " steps, " + movesOut + " moves sent, " + movesIn + " moves flown";
		net += " | switch " + YesNo(m_FlightControls.m_Power) + ", starter " + YesNo(m_FlightControls.m_Starter) + ", synced drive " + typename.EnumToString(geb_UFOPowerState, m_NetPowerState);
		Print(net);

		string legState;
		foreach (geb_UFOLeg leg : m_Legs)
		{
			if (legState != "")
				legState += ", ";
			legState += leg.m_Name + " ";
			if (leg.m_Contact)
				legState += Num(leg.m_Load) + " N";
			else
				legState += "up";
		}

		vector pos = GetPosition();
		vector vel = GetVelocity(this);
		vector ori = GetOrientation();
		string state = "on";
		if (m_PowerState == geb_UFOPowerState.STARTING)
			state = "starting";
		else if (m_PowerState == geb_UFOPowerState.OFF)
			state = "off";

		string line = "AlienInvasion UFO (" + DebugMachine() + "): drive " + state + ", power " + Num(m_Power * 100) + "%, effort " + Num(m_Effort * 100) + "%";
		line += " | speed " + Num(vel.Length() * 3.6) + " km/h, climb " + Num(vel[1], 1) + " m/s, height " + Num(pos[1] - g_Game.SurfaceY(pos[0], pos[2]), 1) + " m";
		line += " | yaw " + Num(ori[0]) + ", pitch " + Num(ori[1], 1) + ", roll " + Num(ori[2], 1);
		line += " | gear " + Num(m_GearPhase, 2) + ", on legs " + Num(m_GroundShare * 100) + "% (" + legState + ")";
		line += " | fwd " + Num(m_FlightControls.m_Forward, 2) + ", strafe " + Num(m_FlightControls.m_Strafe, 2) + ", yaw " + Num(m_FlightControls.m_Yaw, 2) + ", climb " + Num(m_FlightControls.m_Climb, 2);
		Print(line);
	}

	//--------------------------------------------------------------------------
	// what you see and hear (clients)
	//--------------------------------------------------------------------------

	override void EOnFrame(IEntity other, float timeSlice)
	{
		if (!g_Game.IsServer())
		{
			bool pilot = IsLocalPilot();
			if (pilot && !m_TookOver)
				TakeOver();
			m_TookOver = pilot;
		}

		// every frame, asleep or not: a sleeping body gets no simulation steps to do this in
		UpdateSleep();
		if (m_DebugLog)
			DebugReadout(timeSlice);

		#ifndef SERVER
		UpdateVisuals(timeSlice);
		UpdateSound();
		#endif
	}

	/*!
		This client has just taken the seat. Until now the server flew the craft (it idled where it was left,
		or someone else had it), so this machine's drive and gear are stale: start from the server's. The
		drive's switch matters most, as it isn't in the move: a craft left running stays running.
	*/
	protected void TakeOver()
	{
		geb_UFOControls c = m_FlightControls;
		c.m_Power = m_NetSwitch;
		c.m_GearDown = m_NetGearDown;
		m_PowerState = m_NetPowerState;
		m_Power = m_NetPower;
		m_GearPhase = m_NetGear;
		// powering up by itself after a start: carry on
		c.m_StarterLatched = m_PowerState == geb_UFOPowerState.STARTING;
		c.m_Starter = c.m_StarterLatched;
		m_SpoolTime = 0;

		DebugNote("took over, drive " + typename.EnumToString(geb_UFOPowerState, m_PowerState) + ", switch " + YesNo(c.m_Power));
	}

	/*!
		A body that has sat still for a moment falls asleep, and asleep it gets no simulation steps: the drive
		wouldn't power up nor the craft fly. On the machines that simulate it (the server, the pilot's), it is
		held awake while someone is at the controls or the drive is on, and may sleep again once it's parked
		and empty.
	*/
	protected void UpdateSleep()
	{
		bool simulates = g_Game.IsServer() || IsLocalPilot();
		bool awake = simulates && (CrewDriver() != null || m_PowerState != geb_UFOPowerState.OFF);
		if (awake)
		{
			// checked every frame, not just when it should change: the engine puts the body to sleep itself when
			// the craft changes hands. It did as a pilot took the seat back (Cole, 2026-10-28): the server's copy
			// stopped simulating and taking his moves, his machine flew on alone, and actions that wait on the
			// server (opening the canopy, so getting out) hung
			if (!m_KeptAwake || !dBodyIsActive(this))
				WakeUp();
			return;
		}

		if (m_KeptAwake)
		{
			m_KeptAwake = false;
			dBodyActive(this, ActiveState.ACTIVE);
		}
	}

	protected void WakeUp()
	{
		m_KeptAwake = true;
		dBodyActive(this, ActiveState.ALWAYS_ACTIVE);
	}

	protected void SetPhase(string source, float phase)
	{
		if (Math.AbsFloat(GetAnimationPhase(source) - phase) > 0.001)
			SetAnimationPhase(source, phase);
	}

	protected void UpdateVisuals(float dt)
	{
		if (!m_FlightLoaded)
			return;

		float power = GetPowerLevel();
		SetPhase("gear", GetGearPhase());

		// the pod's light ring turns faster the harder the drive works; the hologram turns while it's lit
		float effort = m_NetEffort;
		if (!IsProxy())
			effort = m_Effort;
		m_SpinTurns += dt * power * (0.12 + 0.5 * effort);
		m_SpinTurns -= Math.Floor(m_SpinTurns);
		SetAnimationPhase("spin", m_SpinTurns);
		if (power > LIT_POWER)
		{
			m_HoloTurns += dt * 0.2;
			m_HoloTurns -= Math.Floor(m_HoloTurns);
			SetAnimationPhase("holo", m_HoloTurns);

			// the radar display turns (its sweep is painted on it); the displays run through their frames while
			// someone is close enough to see them
			m_RadarTurns += dt * 0.35;
			m_RadarTurns -= Math.Floor(m_RadarTurns);
			SetAnimationPhase("radar", m_RadarTurns);
			m_ScreenTime += dt;
			if (m_ScreenTime >= SCREEN_STEP && geb_CameraWithin(this, SCREEN_RANGE_SQ))
			{
				m_ScreenTime = 0;
				m_ScreenFrame = geb_NextScreenFrame(this, SCREEN_FRAME_TEXTURE, SCREEN_FRAMES, m_ScreenFrame, SCREENS_INDEX);
			}
		}

		// moving the canopy puts the config's materials back on the hidden selections (dark glow, new hull and
		// glass), as a door does a car's lights; vanilla's door action forces a light update round it. Put ours
		// back every frame while it moves and for a moment after it stops
		float canopy = 0;
		if (m_CanopySource != "")
			canopy = GetAnimationPhase(m_CanopySource);
		if (Math.AbsFloat(canopy - m_CanopyShown) > 0.0001)
			m_LookRefreshTime = LOOK_REFRESH_TIME;
		m_CanopyShown = canopy;
		if (m_LookRefreshTime > 0)
		{
			m_LookRefreshTime -= dt;
			RefreshLooks();
		}

		ShowLit(power > LIT_POWER);
		UpdateLights(power);
		UpdateDamageLook();
		UpdateField(dt, power > LIT_POWER);
		UpdateEffects(power, effort);
	}

	//! Set this craft's own materials and textures again on the next look update (UpdateVisuals).
	protected void RefreshLooks()
	{
		m_LitSet = false;
		m_HullLook = -1;
		m_GlassLook = -1;
		m_FieldSet = false;
	}

	//! A hit resets the hidden selections too (vanilla's car forces a light update on every hit): tell clients.
	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if (!g_Game.IsServer())
			return;

		m_NetLookRefresh = (m_NetLookRefresh + 1) % 16;
		SetSynchDirty();
	}

	override void OnVariablesSynchronized()
	{
		super.OnVariablesSynchronized();

		if (m_NetLookRefresh != m_LookRefreshSeen)
		{
			m_LookRefreshSeen = m_NetLookRefresh;
			m_LookRefreshTime = LOOK_REFRESH_TIME;
		}
	}

	/*!
		The hex field inside the glass: shown while the drive is lit, turning slowly round so its bright sweeps
		run round the dome. It stutters with the glass cracked and is gone with the glass ruined.
	*/
	protected void UpdateField(float dt, bool lit)
	{
		bool show = lit && m_GlassLook < 2;
		if (show && m_GlassLook == 1)
		{
			m_FieldFlickerTime -= dt;
			if (m_FieldFlickerTime <= 0)
			{
				m_FieldFlickerOn = Math.RandomFloat01() < 0.7;
				m_FieldFlickerTime = Math.RandomFloat(0.05, 0.35);
			}
			show = m_FieldFlickerOn;
		}

		if (!m_FieldSet || show != m_FieldShown)
		{
			m_FieldSet = true;
			m_FieldShown = show;
			if (show)
				SetObjectTexture(FIELD_INDEX, FIELD_TEXTURE);
			else
				SetObjectTexture(FIELD_INDEX, "");
		}

		if (lit)
		{
			m_FieldTurns += dt / FIELD_TURN_TIME;
			m_FieldTurns -= Math.Floor(m_FieldTurns);
			SetAnimationPhase("shimmer", m_FieldTurns);
		}
	}

	/*!
		The particles (tools/alienufo/effects.py): the downwash's dust on the ground below while the drive is on
		low over land, more the lower and the harder it works; green sparks falling from the emitter while it's
		lit; smoke from a badly damaged (half) or ruined hull. Each is turned up and down by its birth rate rather
		than restarted, and stopping one lets what's out fade.
	*/
	protected void UpdateEffects(float power, float effort)
	{
		float motes = 0;
		if (power > LIT_POWER)
			motes = power;
		m_MotesFx = UpdateAttachedEffect(m_MotesFx, ParticleList.GEB_UFO_MOTES, "emitter", motes, m_MotesRate);

		float smoke = 0;
		int hullLevel = GetHealthLevel("Hull");
		if (hullLevel >= GameConstants.STATE_RUINED)
			smoke = 1;
		else if (hullLevel >= GameConstants.STATE_BADLY_DAMAGED)
			smoke = 0.5;
		m_SmokeFx = UpdateAttachedEffect(m_SmokeFx, ParticleList.GEB_UFO_SMOKE, "smoke", smoke, m_SmokeRate);

		vector ground = GetPosition();
		float height = ground[1];
		ground[1] = g_Game.SurfaceY(ground[0], ground[2]);
		height -= ground[1];
		float dust = 0;
		if (power > 0.05 && height < DUST_HEIGHT && !g_Game.SurfaceIsSea(ground[0], ground[2]) && !g_Game.SurfaceIsPond(ground[0], ground[2]))
			dust = power * Math.Clamp((DUST_HEIGHT - height) / (DUST_HEIGHT - 2), 0, 1) * (0.35 + 0.65 * effort);

		if (dust <= 0.01)
		{
			StopEffect(m_DustFx);
			m_DustFx = null;
			return;
		}

		if (!m_DustFx)
		{
			m_DustFx = ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_UFO_DUST, ground);
			if (!m_DustFx)
				return;
			m_DustFx.SetParticleAutoDestroyFlags(ParticleAutoDestroyFlags.ON_STOP);
			m_DustRate = -1;
		}
		m_DustFx.SetPosition(ground);
		SetEffectRate(m_DustFx, dust, m_DustRate);
	}

	//! An effect on the craft at one of its memory points, at `rate` of its full birth rate (0 stops it).
	protected ParticleSource UpdateAttachedEffect(ParticleSource fx, int particleId, string memoryPoint, float rate, inout float shownRate)
	{
		if (rate <= 0.01)
		{
			StopEffect(fx);
			return null;
		}

		if (!fx)
		{
			fx = ParticleManager.GetInstance().PlayOnObject(particleId, this, GetMemoryPointPos(memoryPoint), "0 0 0", false);
			if (!fx)
				return null;
			fx.SetParticleAutoDestroyFlags(ParticleAutoDestroyFlags.ON_STOP);
			shownRate = -1;
		}

		SetEffectRate(fx, rate, shownRate);
		return fx;
	}

	protected static void SetEffectRate(ParticleSource fx, float rate, inout float shownRate)
	{
		if (Math.AbsFloat(rate - shownRate) < 0.03)
			return;

		shownRate = rate;
		fx.ScaleParticleParamFromOriginal(EmitorParam.BIRTH_RATE, rate);
	}

	protected static void StopEffect(ParticleSource fx)
	{
		if (fx)
			fx.Stop();
	}

	protected void StopEffects()
	{
		StopEffect(m_DustFx);
		StopEffect(m_MotesFx);
		StopEffect(m_SmokeFx);
		m_DustFx = null;
		m_MotesFx = null;
		m_SmokeFx = null;
	}

	/*!
		A change of health level puts the config's materials back on the hidden selections: the glow and the
		screens go dark and the hull and glass look new. Put this craft's own back on the next frame, as
		vanilla's chemlight relights itself.
	*/
	override void EEHealthLevelChanged(int oldLevel, int newLevel, string zone)
	{
		super.EEHealthLevelChanged(oldLevel, newLevel, zone);

		RefreshLooks();
	}

	//! As vanilla's cars: pristine and worn look new, damaged and badly damaged show it, ruined is burnt out.
	protected static int DamageLook(int healthLevel)
	{
		if (healthLevel >= GameConstants.STATE_RUINED)
			return 2;
		if (healthLevel >= GameConstants.STATE_DAMAGED)
			return 1;

		return 0;
	}

	//! The hull scorched and the canopy's glass cracked as the Hull and Canopy zones wear down.
	protected void UpdateDamageLook()
	{
		int hull = DamageLook(GetHealthLevel("Hull"));
		if (hull != m_HullLook)
		{
			m_HullLook = hull;
			string hullMaterial = HULL_MATERIAL;
			if (hull == 1)
				hullMaterial = HULL_DAMAGE_MATERIAL;
			else if (hull == 2)
				hullMaterial = HULL_DESTRUCT_MATERIAL;
			SetObjectMaterial(CAMO_INDEX, hullMaterial);
		}

		int glass = DamageLook(GetHealthLevel("Canopy"));
		if (glass != m_GlassLook)
		{
			m_GlassLook = glass;
			string glassMaterial = GLASS_MATERIAL;
			if (glass == 1)
				glassMaterial = GLASS_DAMAGE_MATERIAL;
			else if (glass == 2)
				glassMaterial = GLASS_DESTRUCT_MATERIAL;
			SetObjectMaterial(GLASS_INDEX, glassMaterial);
			SetObjectMaterial(GLASS_IN_INDEX, glassMaterial);
		}
	}

	//! The glowing parts and the screens: lit with the power on, dark glass with it off.
	protected void ShowLit(bool lit)
	{
		if (m_LitSet && lit == m_LitShown)
			return;

		m_LitSet = true;
		m_LitShown = lit;
		if (lit)
		{
			SetObjectTexture(GLOW_INDEX, GLOW_LIT_TEXTURE);
			SetObjectMaterial(GLOW_INDEX, GLOW_LIT_MATERIAL);
			SetObjectTexture(SCREENS_INDEX, string.Format(SCREEN_FRAME_TEXTURE, m_ScreenFrame.ToStringLen(2)));
			SetObjectMaterial(SCREENS_INDEX, SCREENS_LIT_MATERIAL);
		}
		else
		{
			SetObjectTexture(GLOW_INDEX, GLOW_DARK_TEXTURE);
			SetObjectMaterial(GLOW_INDEX, GLOW_DARK_MATERIAL);
			SetObjectTexture(SCREENS_INDEX, SCREENS_DARK_TEXTURE);
			SetObjectMaterial(SCREENS_INDEX, SCREENS_DARK_MATERIAL);
		}
	}

	protected void UpdateLights(float power)
	{
		if (power <= LIT_POWER)
		{
			DestroyLights();
			return;
		}

		if (!m_UnderLight && MemoryPointExists("light_under"))
		{
			m_UnderLight = geb_UFOLight.Cast(ScriptedLightBase.CreateLight(geb_UFOLight, "0 0 0"));
			if (m_UnderLight)
				m_UnderLight.AttachOnMemoryPoint(this, "light_under");
		}
		if (!m_CockpitLight && MemoryPointExists("light_cockpit"))
		{
			m_CockpitLight = geb_UFOCockpitLight.Cast(ScriptedLightBase.CreateLight(geb_UFOCockpitLight, "0 0 0"));
			if (m_CockpitLight)
				m_CockpitLight.AttachOnMemoryPoint(this, "light_cockpit");
		}

		if (m_UnderLight)
			m_UnderLight.SetBrightnessTo(geb_UFOLight.BRIGHTNESS * power);
	}

	protected void DestroyLights()
	{
		if (m_UnderLight)
		{
			m_UnderLight.FadeOut();
			m_UnderLight = null;
		}
		if (m_CockpitLight)
		{
			m_CockpitLight.FadeOut();
			m_CockpitLight = null;
		}
	}

	/*!
		The drive's sounds: powering up and down, the hum, the whine and the rush of air while it's on (bent by
		its power, effort and speed), the gear's hydraulics and the canopy's servo.
	*/
	protected void UpdateSound()
	{
		if (!m_FlightLoaded)
			return;

		int state = GetPowerState();
		bool gearDown = IsGearSetDown();
		if (!m_HeardSet)
		{
			// what it was doing before this client saw it is not news
			m_HeardSet = true;
			m_HeardPowerState = state;
			m_HeardGearDown = gearDown;
		}

		if (state != m_HeardPowerState)
		{
			if (state == geb_UFOPowerState.STARTING)
				PlayOnce(m_PowerUpSound);
			else if (state == geb_UFOPowerState.OFF && m_HeardPowerState == geb_UFOPowerState.ON)
				PlayOnce(m_PowerDownSound);

			m_HeardPowerState = state;
		}

		if (gearDown != m_HeardGearDown)
		{
			if (gearDown)
				PlayOnce(m_GearDownSound);
			else
				PlayOnce(m_GearUpSound);

			m_HeardGearDown = gearDown;
		}

		float power = GetPowerLevel();
		bool running = power > 0.05;
		if (!m_LoopSound && running && m_LoopSoundSets.Count() > 0)
			m_LoopSound = new geb_UFOSound(this, GetMemoryPointPos("sound_hum"), m_LoopSoundSets);

		if (!m_LoopSound)
			return;

		float effort = m_NetEffort;
		if (!IsProxy())
			effort = m_Effort;

		float campos = 1;
		PlayerBase player = PlayerBase.Cast(g_Game.GetPlayer());
		if (player && player.GetParent() == this && player.IsCameraInsideVehicle())
			campos = 0;

		float canopy = 0;
		if (m_CanopySource != "")
			canopy = GetAnimationPhase(m_CanopySource);

		m_LoopSound.Update(running, power * (700 + 300 * effort), effort, GetSpeedKmh(), canopy, campos);
	}

	protected void PlayOnce(string soundSet)
	{
		if (soundSet != "")
			SEffectManager.PlaySoundOnObject(soundSet, this);
	}

	//! The canopy's servo, as a car door's sound: opening from shut, closing from open.
	override void OnAnimationPhaseStarted(string animSource, float phase)
	{
		super.OnAnimationPhaseStarted(animSource, phase);

		#ifndef SERVER
		string canopy = m_CanopySource;
		canopy.ToLower();
		animSource.ToLower();
		if (canopy == "" || animSource != canopy)
			return;

		if (phase < 0.5)
			PlayOnce(m_CanopyOpenSound);
		else
			PlayOnce(m_CanopyCloseSound);
		#endif
	}

	//--------------------------------------------------------------------------
	// the canopy, boarding and leaving
	//--------------------------------------------------------------------------

	string GetCanopySource()
	{
		return m_CanopySource;
	}

	//! Whether someone standing at pos is close enough to the hull to work the canopy from outside.
	bool IsWithinReach(vector pos)
	{
		vector here = GetPosition();
		here[1] = 0;
		pos[1] = 0;
		return vector.Distance(here, pos) <= m_Reach;
	}

	//! Whether someone standing at pos is at the boarding place on the left rim.
	bool IsAtDoor(vector pos, float distance)
	{
		if (m_DoorPoint == "" || !MemoryPointExists(m_DoorPoint))
			return false;

		vector door = ModelToWorld(GetMemoryPointPos(m_DoorPoint));
		door[1] = 0;
		pos[1] = 0;
		return vector.Distance(door, pos) <= distance;
	}

	/*!
		Puts someone who has just got out on the ground beside the left rim: the driver's get-out animation
		leaves them standing on the hull. Jumping out in the air, they fall from beside the rim.
	*/
	void PlaceAtDoor(Human player)
	{
		if (!player || m_DoorPoint == "" || !MemoryPointExists(m_DoorPoint))
			return;

		vector pos = ModelToWorld(GetMemoryPointPos(m_DoorPoint));
		float ground = g_Game.SurfaceRoadY(pos[0], pos[2]);
		if (pos[1] - ground < 1.5)
			pos[1] = ground;

		player.SetPosition(pos);
	}

	//! In and out only with the canopy open.
	override bool CrewCanGetThrough(int posIdx)
	{
		return IsCanopyOpen();
	}

	/*!
		Getting in: from the boarding place on the left rim looking at the rim, or from anywhere round the craft
		looking at the seat itself over the rim (the View Geometry block "seat_view" on its back and headrest,
		in sight with the canopy open). The get-in animation starts from the boarding place either way.
	*/
	override bool CanReachSeatFromDoors(string pSeatSelection, vector pFromPos, float pDistance = 1.0)
	{
		string selection = pSeatSelection;
		selection.ToLower();
		if (selection == "seat_view")
			return IsWithinReach(pFromPos);

		return IsAtDoor(pFromPos, pDistance + 1.0);
	}

	override bool CanReachDoorsFromSeat(string pDoorsSelection, int pCurrentSeat)
	{
		return true;
	}

	override bool CanReachSeatFromSeat(int currentSeat, int nextSeat)
	{
		return false;
	}

	//--------------------------------------------------------------------------
	// actions on the craft (the ones from the seat are the player's: scripts/4_world/entities/manbase/
	// geb_UFOCrew.c)
	//--------------------------------------------------------------------------

	void InitializeActions()
	{
		m_InputActionMap = s_geb_UFOActionMaps.Get(Type());
		if (!m_InputActionMap)
		{
			// the shared map owns it; the craft only keeps a pointer, as CarScript does
			TInputActionMap actionMap = new TInputActionMap();
			s_geb_UFOActionMaps.Insert(Type(), actionMap);
			m_InputActionMap = actionMap;
			SetActions();
		}
	}

	override void GetActions(typename action_input_type, out array<ActionBase_Basic> actions)
	{
		if (!m_ActionsInitialized)
		{
			m_ActionsInitialized = true;
			InitializeActions();
		}

		actions = m_InputActionMap.Get(action_input_type);
		if (actions)
			return;

		// Vanilla asks a vehicle you sit in for its actions of every input type and counts them without
		// checking for none (a car always has its lights action). So every type gets a list, if only an empty
		// one; the shared map owns it
		array<ActionBase_Basic> none = new array<ActionBase_Basic>();
		m_InputActionMap.Insert(action_input_type, none);
		actions = none;
	}

	void SetActions()
	{
		AddAction(ActionGetInTransport);
		AddAction(geb_ActionOpenUFOCanopy);
		AddAction(geb_ActionCloseUFOCanopy);
	}

	void AddAction(typename actionName)
	{
		ActionBase action = ActionManagerBase.GetAction(actionName);
		if (!action)
		{
			Debug.LogError("Action " + actionName + " doesn't exist!");
			return;
		}

		typename inputType = action.GetInputType();
		if (!inputType)
		{
			m_ActionsInitialized = false;
			return;
		}

		array<ActionBase_Basic> actions = m_InputActionMap.Get(inputType);
		if (!actions)
		{
			actions = new array<ActionBase_Basic>();
			m_InputActionMap.Insert(inputType, actions);
		}

		actions.Insert(action);
	}

	//! Admin-tool spawn: the canopy open, ready to climb in.
	override void OnDebugSpawn()
	{
		if (m_CanopySource != "")
			SetAnimationPhase(m_CanopySource, 1);
	}
};
