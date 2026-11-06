/*
	The flyable UFO's actions (geb_UFO.c).

	The canopy is an animation on the craft, not a door item, and the craft is
	not a car, so vanilla's car door actions never see it. All of these are the
	player's own actions (geb_UFOCrew.c): outside, opening the canopy, getting in
	and closing it; from the seat, opening and closing it and powering up and
	down.

	Outside, they find the craft themselves (geb_UFOActions.LookedAt). Vanilla's
	action targets only count what you look at within 3 m of your feet
	(ActionTargets.ComputeUtility), and on a six-metre saucer the dome, the open
	canopy and the seat stand higher than that, so looking at them offered
	nothing, and getting in meant finding one block on the left rim (Cole,
	2026-11-05: players had trouble getting in and opening it). Now, from beside
	the craft: look at any part of it to open the canopy, or with it open to get
	in; look at the canopy itself to close it.

	Power works the drive's switch on the server and on the pilot's client
	alike (OnStart, OnEnd, OnExecute run on both), as vanilla's engine actions
	do for a vehicle without moves: power up is held, as a car's start is, and
	the drive takes hold after geb_UFOFlight spoolTime; power down only on the
	ground, where switching off doesn't drop the craft out of the sky.
*/
class geb_UFOActions
{
	static const float LOOK_RANGE = 8;		//!< m from the camera an outside action sees the craft
	static const int PILOT_SEAT = 0;		//!< crew position of the craft's one seat (data/ufo/config.cpp Crew Driver)

	// LookedAt, worked out once a frame however many actions ask
	protected static int s_LookTime = -1;
	protected static PlayerBase s_LookPlayer;
	protected static geb_UFO s_LookUFO;
	protected static bool s_LookCanopy;

	//! The closest UFO the player can reach from where they stand (geb_UFO.IsWithinReach), or null.
	static geb_UFO NearestUFO(PlayerBase player)
	{
		if (!player)
			return null;

		geb_UFO best;
		float bestDistSq = 1000000;
		vector pos = player.GetPosition();
		array<geb_UFO> all = geb_UFO.GetAll();
		foreach (geb_UFO ufo : all)
		{
			if (!ufo || !ufo.IsWithinReach(pos))
				continue;

			float distSq = vector.DistanceSq(ufo.GetPosition(), pos);
			if (distSq < bestDistSq)
			{
				bestDistSq = distSq;
				best = ufo;
			}
		}
		return best;
	}

	/*!
		The UFO an outside action works on: the one beside the player that they're looking at, and whether they're
		looking at its canopy (the View Geometry block "canopy", which swings up with it). Their own machine casts
		a ray from the camera at the craft's View Geometry, as far as LOOK_RANGE. The server has no camera: it
		takes the craft beside them, the player's machine having checked what they look at.
	*/
	static geb_UFO LookedAt(PlayerBase player, out bool atCanopy)
	{
		int now = g_Game.GetTime();
		if (now != s_LookTime || player != s_LookPlayer)
		{
			s_LookTime = now;
			s_LookPlayer = player;
			s_LookUFO = FindLookedAt(player, s_LookCanopy);
		}

		atCanopy = s_LookCanopy;
		return s_LookUFO;
	}

	protected static geb_UFO FindLookedAt(PlayerBase player, out bool atCanopy)
	{
		atCanopy = false;
		geb_UFO near = NearestUFO(player);
		if (!near)
			return null;

		if (g_Game.IsDedicatedServer())
		{
			atCanopy = true;
			return near;
		}

		vector from = g_Game.GetCurrentCameraPosition();
		vector to = from + g_Game.GetCurrentCameraDirection() * LOOK_RANGE;
		vector hitPos;
		vector hitDir;
		int component = -1;
		set<Object> hits = new set<Object>;
		if (!DayZPhysics.RaycastRV(from, to, hitPos, hitDir, component, hits, null, player, false, false, ObjIntersectView))
			return null;

		// the first thing the ray meets: the craft itself, or someone sitting in it
		for (int i = 0; i < hits.Count(); i++)
		{
			Object hit = hits.Get(i);
			if (!hit)
				continue;

			if (hit == near)
			{
				atCanopy = near.IsActionComponentPartOfSelection(component, "canopy", "view");
				return near;
			}
			if (hit.GetParent() == near)
				return near;

			return null;
		}
		return null;
	}

	//! The UFO the player is flying, or null.
	static geb_UFO GetPilotedUFO(PlayerBase player)
	{
		if (!player)
			return null;

		HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();
		if (!vehCommand)
			return null;

		geb_UFO ufo = geb_UFO.Cast(vehCommand.GetTransport());
		if (!ufo || ufo.CrewDriver() != player)
			return null;

		return ufo;
	}

	//! The UFO the player sits in (not getting in or out), or null.
	static geb_UFO GetRiddenUFO(PlayerBase player)
	{
		if (!player)
			return null;

		HumanCommandVehicle vehCommand = player.GetCommand_Vehicle();
		if (!vehCommand || vehCommand.IsGettingIn() || vehCommand.IsGettingOut())
			return null;

		return geb_UFO.Cast(vehCommand.GetTransport());
	}
};

class geb_ActionUFOCanopyBase : ActionInteractBase
{
	protected bool m_IsOpening = true;

	void geb_ActionUFOCanopyBase()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_OPENDOORFW;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_LockTargetOnUse = false;
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTNone();
	}

	//! The craft finds itself (geb_UFOActions.LookedAt), out of vanilla's 3 m reach for action targets
	override bool HasTarget()
	{
		return false;
	}

	//! The craft whose canopy it swings: outside, the one the player looks at
	protected geb_UFO ActionUFO(PlayerBase player)
	{
		bool atCanopy;
		return geb_UFOActions.LookedAt(player, atCanopy);
	}

	protected bool IsSwingNeeded(geb_UFO ufo)
	{
		if (m_IsOpening)
			return !ufo.IsCanopyOpen();

		return !ufo.IsCanopyShut();
	}

	//! It opens looking at any part of the craft, and closes only looking at the canopy itself: anywhere, it
	//! would stand in the way of getting in as you look at the craft to climb in.
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		// from the seat, the inside actions below
		if (player.GetCommand_Vehicle())
			return false;

		bool atCanopy;
		geb_UFO ufo = geb_UFOActions.LookedAt(player, atCanopy);
		if (!ufo || ufo.GetCanopySource() == "")
			return false;

		if (!m_IsOpening && !atCanopy)
			return false;

		return IsSwingNeeded(ufo);
	}

	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		SwingCanopy(ActionUFO(action_data.m_Player));
	}

	protected void SwingCanopy(geb_UFO ufo)
	{
		if (!ufo)
			return;

		float phase = 0;
		if (m_IsOpening)
			phase = 1;

		ufo.SetAnimationPhase(ufo.GetCanopySource(), phase);
	}
};

class geb_ActionOpenUFOCanopy : geb_ActionUFOCanopyBase
{
	void geb_ActionOpenUFOCanopy()
	{
		m_IsOpening = true;
		m_Text = "#open";
	}
};

class geb_ActionCloseUFOCanopy : geb_ActionUFOCanopyBase
{
	void geb_ActionCloseUFOCanopy()
	{
		m_IsOpening = false;
		m_Text = "#close";
	}
};

/*!
	From the seat. Seated, the player plays a car driver's seated door animation: a standing one never finishes
	in a seat, and the action hangs with everything else behind it.
*/
class geb_ActionUFOCanopyInsideBase : geb_ActionUFOCanopyBase
{
	void geb_ActionUFOCanopyInsideBase()
	{
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		geb_UFO ufo = geb_UFOActions.GetRiddenUFO(player);
		if (!ufo || ufo.GetCanopySource() == "")
			return false;

		return IsSwingNeeded(ufo);
	}

	//! From the seat, the craft they sit in
	override protected geb_UFO ActionUFO(PlayerBase player)
	{
		return geb_UFOActions.GetRiddenUFO(player);
	}

	override bool CanBeUsedInVehicle()
	{
		return true;
	}
};

class geb_ActionOpenUFOCanopyInside : geb_ActionUFOCanopyInsideBase
{
	void geb_ActionOpenUFOCanopyInside()
	{
		m_IsOpening = true;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_DRIVER_DOOR_OPEN;
		m_Text = "#open";
	}
};

class geb_ActionCloseUFOCanopyInside : geb_ActionUFOCanopyInsideBase
{
	void geb_ActionCloseUFOCanopyInside()
	{
		m_IsOpening = false;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_DRIVER_DOOR_CLOSE;
		m_Text = "#close";
	}
};

/*!
	Getting in: looking at any part of the craft from beside it, with the canopy open. Vanilla's
	ActionGetInTransport takes the seat from the part you look at (Crew actionSel), so it only came up on one block
	on the left rim, or the seat over it out of its 3 m reach. Otherwise it is vanilla's: the get-in animation
	from the boarding place, and the seat taken through an inventory juncture.
*/
class geb_ActionGetInUFO : ActionBase
{
	void geb_ActionGetInUFO()
	{
		m_StanceMask = DayZPlayerConstants.STANCEMASK_CROUCH | DayZPlayerConstants.STANCEMASK_ERECT;
		m_Text = "#get_in_vehicle";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTNone();
	}

	override typename GetInputType()
	{
		return ContinuousInteractActionInput;
	}

	override bool HasProgress()
	{
		return false;
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (player.GetCommand_Vehicle())
			return false;

		if (player.GetItemInHands() && player.GetItemInHands().IsHeavyBehaviour())
			return false;

		bool atCanopy;
		geb_UFO ufo = geb_UFOActions.LookedAt(player, atCanopy);
		if (!ufo || !ufo.CanGetIn())
			return false;

		int seat = geb_UFOActions.PILOT_SEAT;
		if (ufo.CrewMember(seat))
			return false;

		return ufo.CrewCanGetThrough(seat) && ufo.IsAreaAtDoorFreeDiag(seat);
	}

	//! The craft they get into: the one they look at, else (the server) the one beside them
	protected geb_UFO ActionUFO(PlayerBase player)
	{
		bool atCanopy;
		geb_UFO ufo = geb_UFOActions.LookedAt(player, atCanopy);
		if (!ufo)
			ufo = geb_UFOActions.NearestUFO(player);
		return ufo;
	}

	override void Start(ActionData action_data)
	{
		super.Start(action_data);

		geb_UFO ufo = ActionUFO(action_data.m_Player);
		if (!ufo)
			return;

		int seat = geb_UFOActions.PILOT_SEAT;
		HumanCommandVehicle vehCommand = action_data.m_Player.StartCommand_Vehicle(ufo, seat, ufo.GetSeatAnimationType(seat));
		if (vehCommand)
		{
			vehCommand.SetVehicleType(ufo.GetAnimInstance());
			GetDayZGame().GetBacklit().OnEnterCar();
		}
	}

	override bool CanBeUsedInRestrain()
	{
		return true;
	}

	override void OnUpdate(ActionData action_data)
	{
		if (action_data.m_State == UA_START)
		{
			if (!action_data.m_Player.GetCommand_Vehicle() || !action_data.m_Player.GetCommand_Vehicle().IsGettingIn())
				End(action_data);
		}
	}

	override int GetActionCategory()
	{
		return AC_INTERACT;
	}

	override void OnEnd(ActionData action_data)
	{
	}

	//! The seat, reserved on the server before anyone else can take it
	override bool AddActionJuncture(ActionData action_data)
	{
		geb_UFO ufo = ActionUFO(action_data.m_Player);
		if (!ufo)
			return false;

		InventoryLocation il = new InventoryLocation;
		il.SetVehicle(ufo, action_data.m_Player, geb_UFOActions.PILOT_SEAT);
		if (!g_Game.AddInventoryJunctureEx(action_data.m_Player, action_data.m_Player, il, true, 10000, action_data))
			return false;

		action_data.m_ReservedInventoryLocations.Insert(il);
		return true;
	}
};

class geb_ActionUFOPowerOnCB : ActionContinuousBaseCB
{
	//! As long as a car's start: the seated start animation ends after about a second whatever the action asks
	//! for, cutting a longer one off unfinished. The drive spools up the rest of the way by itself (LatchStarter).
	override void CreateActionComponent()
	{
		m_ActionData.m_ActionComponent = new CAContinuousTime(UATimeSpent.START_ENGINE);
	}
};

class geb_ActionUFOPowerOn : ActionContinuousBase
{
	void geb_ActionUFOPowerOn()
	{
		m_CallbackClass = geb_ActionUFOPowerOnCB;
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_STARTENGINE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_LockTargetOnUse = false;
		m_Text = "#STR_geb_UFOPowerOn";
	}

	override void CreateConditionComponents()
	{
		m_ConditionTarget = new CCTNone();
		m_ConditionItem = new CCINone();
	}

	//! Not while the drive is on or coming on by itself after a start.
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		geb_UFO ufo = geb_UFOActions.GetPilotedUFO(player);
		return ufo && !ufo.IsPoweringOn() && ufo.CanPower();
	}

	//! The drive spools up and comes on while the action runs. Re-checked every step, the start condition would
	//! cut the action off as soon as it spooled, let go of the starter and start it again while the button is
	//! held, round and round, so the drive never came on.
	override bool ActionConditionContinue(ActionData action_data)
	{
		return geb_UFOActions.GetPilotedUFO(action_data.m_Player) != null;
	}

	//! On the server and the pilot's client both: the drive's switch isn't in the network move.
	override void OnStart(ActionData action_data)
	{
		super.OnStart(action_data);

		geb_UFO ufo = geb_UFOActions.GetPilotedUFO(action_data.m_Player);
		if (ufo)
			ufo.HoldStarter(true);
	}

	//! Held to the end: the drive comes on by itself from here (on the server and the pilot's client both).
	override void OnFinishProgress(ActionData action_data)
	{
		super.OnFinishProgress(action_data);

		geb_UFO ufo = geb_UFOActions.GetPilotedUFO(action_data.m_Player);
		if (ufo)
			ufo.LatchStarter();
	}

	override void OnEnd(ActionData action_data)
	{
		super.OnEnd(action_data);

		geb_UFO ufo = geb_UFOActions.GetPilotedUFO(action_data.m_Player);
		if (ufo)
			ufo.HoldStarter(false);
	}

	override bool CanBeUsedInVehicle()
	{
		return true;
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool UseMainItem()
	{
		return false;
	}
};

class geb_ActionUFOPowerOff : ActionSingleUseBase
{
	void geb_ActionUFOPowerOff()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_STOPENGINE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ALL;
		m_Text = "#STR_geb_UFOPowerOff";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTNone();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		geb_UFO ufo = geb_UFOActions.GetPilotedUFO(player);
		return ufo && ufo.GetPowerState() != geb_UFOPowerState.OFF && ufo.IsLanded();
	}

	//! On the server and the pilot's client both, as powering up.
	override void OnExecute(ActionData action_data)
	{
		super.OnExecute(action_data);

		geb_UFO ufo = geb_UFOActions.GetPilotedUFO(action_data.m_Player);
		if (ufo)
			ufo.SwitchOff();
	}

	override bool CanBeUsedInVehicle()
	{
		return true;
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool UseMainItem()
	{
		return false;
	}
};

//! Every action class has to be registered before anything can use it.
modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(geb_ActionGetInUFO);
		actions.Insert(geb_ActionOpenUFOCanopy);
		actions.Insert(geb_ActionCloseUFOCanopy);
		actions.Insert(geb_ActionOpenUFOCanopyInside);
		actions.Insert(geb_ActionCloseUFOCanopyInside);
		actions.Insert(geb_ActionUFOPowerOn);
		actions.Insert(geb_ActionUFOPowerOff);
	}
};
