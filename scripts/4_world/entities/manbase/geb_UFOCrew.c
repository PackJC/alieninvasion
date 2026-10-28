/*
	The pilot of a flyable UFO (geb_UFO.c).

	What they can do from the seat - power up and down, open and close the
	canopy - are the player's actions, as vanilla's start-engine and car door
	actions are (geb_ActionUFO.c).

	Getting out: the pilot's seat uses a car driver's animations, and a car
	driver climbs out to the left, which on a six-metre saucer leaves them
	standing on the hull. When someone's time in a UFO ends, put them on the
	ground beside the left rim (geb_UFO.PlaceAtDoor). Done on the server and on
	the player's own machine, the same moment on both, so the move doesn't come
	back as a network correction. Not when they leave unconscious or dead:
	those stay where the game puts them.
*/
modded class PlayerBase
{
	protected geb_UFO m_geb_UFORidden;

	override void SetActions(out TInputActionMap InputActionMap)
	{
		super.SetActions(InputActionMap);

		AddAction(geb_ActionUFOPowerOn, InputActionMap);
		AddAction(geb_ActionUFOPowerOff, InputActionMap);
		AddAction(geb_ActionOpenUFOCanopyInside, InputActionMap);
		AddAction(geb_ActionCloseUFOCanopyInside, InputActionMap);
	}

	//! every step in a vehicle: remember whether it is a UFO
	override bool ModCommandHandlerInside(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		if (pCurrentCommandID == DayZPlayerConstants.COMMANDID_VEHICLE)
		{
			HumanCommandVehicle vehCommand = GetCommand_Vehicle();
			if (vehCommand)
				m_geb_UFORidden = geb_UFO.Cast(vehCommand.GetTransport());
		}

		return super.ModCommandHandlerInside(pDt, pCurrentCommandID, pCurrentCommandFinished);
	}

	override void OnCommandVehicleFinish()
	{
		super.OnCommandVehicleFinish();

		geb_UFO ufo = m_geb_UFORidden;
		m_geb_UFORidden = null;

		if (!ufo || !IsAlive() || IsUnconscious() || m_ShouldBeUnconscious)
			return;

		if (GetInstanceType() == DayZPlayerInstanceType.INSTANCETYPE_REMOTE)
			return;

		ufo.PlaceAtDoor(this);
	}
};
