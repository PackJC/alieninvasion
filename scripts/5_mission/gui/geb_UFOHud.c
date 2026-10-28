/*
	The flyable UFO's panel on the HUD, in place of a car's speedometer.

	Vanilla's HUD keeps one vehicle HUD per vehicle type (GetVehicleType) and
	shows it to whoever takes the driver's seat; the UFO's type is
	"VehicleTypeGebUFO", and this is its panel: speed, altitude and height
	above the ground, climb, the drive, the gear, the canopy, and the hull's and
	the glass's condition, coloured as vanilla colours an item's.
*/
class geb_UFOHud : VehicleHudBase
{
	protected geb_UFO m_UFO;
	protected TextWidget m_Speed;
	protected TextWidget m_Altitude;
	protected TextWidget m_Climb;
	protected TextWidget m_Drive;
	protected TextWidget m_Gear;
	protected TextWidget m_Canopy;
	protected TextWidget m_Condition;
	protected float m_ClimbShown;		//!< m/s, smoothed
	protected int m_ConditionShown = -1;	//!< the worse of the two levels the line is coloured for

	override void Init(Widget vehicleHudPanels)
	{
		super.Init(vehicleHudPanels);

		m_VehiclePanel = g_Game.GetWorkspace().CreateWidgets("alieninvasion/data/ufo/gui/geb_ufohud.layout", vehicleHudPanels);
		if (!m_VehiclePanel)
			return;

		m_Speed = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Speed"));
		m_Altitude = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Altitude"));
		m_Climb = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Climb"));
		m_Drive = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Drive"));
		m_Gear = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Gear"));
		m_Canopy = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Canopy"));
		m_Condition = TextWidget.Cast(m_VehiclePanel.FindAnyWidget("Condition"));
	}

	protected static string LevelName(int healthLevel)
	{
		switch (healthLevel)
		{
			case GameConstants.STATE_WORN:
				return "WORN";
			case GameConstants.STATE_DAMAGED:
				return "DAMAGED";
			case GameConstants.STATE_BADLY_DAMAGED:
				return "BADLY DAMAGED";
			case GameConstants.STATE_RUINED:
				return "RUINED";
		}
		return "PRISTINE";
	}

	//! The panel's green while it's new, then vanilla's condition colours (Colors.COLOR_WORN ...).
	protected static int LevelColor(int healthLevel)
	{
		switch (healthLevel)
		{
			case GameConstants.STATE_WORN:
				return ARGB(255, 191, 255, 0);
			case GameConstants.STATE_DAMAGED:
				return ARGB(255, 255, 255, 0);
			case GameConstants.STATE_BADLY_DAMAGED:
				return ARGB(255, 255, 191, 0);
			case GameConstants.STATE_RUINED:
				return ARGB(255, 255, 0, 0);
		}
		return ARGB(255, 140, 255, 179);
	}

	override void ShowVehicleInfo(PlayerBase player)
	{
		m_UFO = null;
		m_ClimbShown = 0;
		m_ConditionShown = -1;
		if (player && player.GetCommand_Vehicle())
			m_UFO = geb_UFO.Cast(player.GetCommand_Vehicle().GetTransport());
	}

	override void HideVehicleInfo()
	{
		m_UFO = null;
	}

	protected static string Whole(float value)
	{
		float rounded = Math.Round(value);
		return rounded.ToString();
	}

	protected static string Tenths(float value)
	{
		float rounded = Math.Round(value * 10) / 10;
		string text = rounded.ToString();
		if (text.IndexOf(".") < 0)
			text += ".0";
		return text;
	}

	override void RefreshVehicleHud(float timeslice)
	{
		if (!m_UFO || !m_VehiclePanel || !m_Speed)
			return;

		vector position = m_UFO.GetPosition();
		float ground = g_Game.SurfaceY(position[0], position[2]);
		float climb = GetVelocity(m_UFO)[1];
		m_ClimbShown += (climb - m_ClimbShown) * Math.Clamp(timeslice * 3, 0, 1);

		string speed = Whole(m_UFO.GetSpeedKmh()) + " km/h";
		if (m_UFO.IsBoosting())
			speed = "BOOST  " + speed;
		m_Speed.SetText(speed);
		m_Altitude.SetText("ALT " + Whole(position[1]) + " m   AGL " + Whole(Math.Max(position[1] - ground, 0)) + " m");

		string climbText = Tenths(m_ClimbShown);
		if (m_ClimbShown >= 0.05)
			climbText = "+" + climbText;
		m_Climb.SetText("CLIMB " + climbText + " m/s");

		string drive = "DRIVE OFF";
		switch (m_UFO.GetPowerState())
		{
			case geb_UFOPowerState.STARTING:
				drive = "DRIVE POWERING UP";
				break;
			case geb_UFOPowerState.ON:
				drive = "DRIVE " + Whole(m_UFO.GetPowerLevel() * 100) + "%";
				break;
		}
		m_Drive.SetText(drive);

		float gearPhase = m_UFO.GetGearPhase();
		string gear = "GEAR MOVING";
		if (gearPhase < 0.02)
			gear = "GEAR DOWN";
		else if (gearPhase > 0.98)
			gear = "GEAR UP";
		m_Gear.SetText(gear);

		string canopy = "CANOPY MOVING";
		if (m_UFO.IsCanopyShut())
			canopy = "CANOPY SHUT";
		else if (m_UFO.IsCanopyOpen())
			canopy = "CANOPY OPEN";
		m_Canopy.SetText(canopy);

		if (m_Condition)
		{
			int hull = m_UFO.GetHealthLevel("Hull");
			int glass = m_UFO.GetHealthLevel("Canopy");
			m_Condition.SetText("HULL " + LevelName(hull) + "   GLASS " + LevelName(glass));
			int worst = hull;
			if (glass > worst)
				worst = glass;
			if (worst != m_ConditionShown)
			{
				m_ConditionShown = worst;
				m_Condition.SetColor(LevelColor(worst));
			}
		}
	}
};

//! Hand the HUD its UFO panel, alongside vanilla's car, boat and motorbike ones.
modded class IngameHud
{
	override void Init(Widget hud_panel_widget)
	{
		super.Init(hud_panel_widget);

		geb_UFOHud ufoHud = new geb_UFOHud();
		m_VehicleHudMap.Insert("VehicleTypeGebUFO", ufoHud);
		ufoHud.Init(m_VehicleHudPanels);
		ufoHud.HidePanel();
	}
};
