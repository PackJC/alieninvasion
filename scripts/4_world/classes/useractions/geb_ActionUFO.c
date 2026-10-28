/*
	The flyable UFO's actions (geb_UFO.c).

	The canopy is an animation on the craft, not a door item, and the craft is
	not a car, so vanilla's car door actions never see it. Outside, the canopy
	actions are the craft's own (geb_UFO.SetActions); from the seat they, and
	powering up and down, are the player's (geb_UFOCrew.c).

	Power works the drive's switch on the server and on the pilot's client
	alike (OnStart, OnEnd, OnExecute run on both), as vanilla's engine actions
	do for a vehicle without moves: power up is held, as a car's start is, and
	the drive takes hold after geb_UFOFlight spoolTime; power down only on the
	ground, where switching off doesn't drop the craft out of the sky.
*/
class geb_UFOActions
{
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

	protected geb_UFO GetUFO(ActionTarget target)
	{
		if (!target)
			return null;

		geb_UFO ufo = geb_UFO.Cast(target.GetObject());
		if (!ufo)
			ufo = geb_UFO.Cast(target.GetParent());

		return ufo;
	}

	protected bool IsSwingNeeded(geb_UFO ufo)
	{
		if (m_IsOpening)
			return !ufo.IsCanopyOpen();

		return !ufo.IsCanopyShut();
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		// from the seat, the inside actions below
		if (player.GetCommand_Vehicle())
			return false;

		geb_UFO ufo = GetUFO(target);
		if (!ufo || ufo.GetCanopySource() == "")
			return false;

		if (!ufo.IsWithinReach(player.GetPosition()))
			return false;

		return IsSwingNeeded(ufo);
	}

	override void OnStartServer(ActionData action_data)
	{
		super.OnStartServer(action_data);

		SwingCanopy(GetUFO(action_data.m_Target));
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

	override void OnStartServer(ActionData action_data)
	{
		// the outside action's swing finds no UFO at a target from the seat
		super.OnStartServer(action_data);

		SwingCanopy(geb_UFOActions.GetRiddenUFO(action_data.m_Player));
	}

	override bool CanBeUsedInVehicle()
	{
		return true;
	}

	override bool HasTarget()
	{
		return false;
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

		actions.Insert(geb_ActionOpenUFOCanopy);
		actions.Insert(geb_ActionCloseUFOCanopy);
		actions.Insert(geb_ActionOpenUFOCanopyInside);
		actions.Insert(geb_ActionCloseUFOCanopyInside);
		actions.Insert(geb_ActionUFOPowerOn);
		actions.Insert(geb_ActionUFOPowerOff);
	}
};
