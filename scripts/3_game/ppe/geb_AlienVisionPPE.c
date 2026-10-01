// Alien vision (cooked alien meat) borrows the pumpkin helmet's night-vision mode so it never fights with real NV
// goggles. Vanilla tints that mode orange ("pumpkin-o-vision"); while alien vision is active it's tinted green.
class geb_AlienVision
{
	static bool s_Active;	// client: set by PlayerBase while the effect runs
}

modded class PPERequester_CameraNV
{
	override protected void SetNVMode(int mode)
	{
		super.SetNVMode(mode);

		if ( mode == NV_PUMPKIN && geb_AlienVision.s_Active )
			SetTargetValueColor(PostProcessEffectType.Glow,PPEGlow.PARAM_COLORIZATIONCOLOR,{0.25,1.0,0.35,0.0},PPEGlow.L_23_NVG,PPOperators.MULTIPLICATIVE);
	}
}
