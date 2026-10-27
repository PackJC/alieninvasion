// The plasma grenade's arm and disarm prompts, "Prime" and "Disarm": vanilla's unpin and pin actions with words
// that suit a ball with no pin. Rules, animation and sounds are vanilla's, so Disarm works as the pin does: while
// the grenade is still in the hand, until the fuse starts as it leaves it. geb_PlasmaGrenade.SetActions swaps them in.
class geb_ActionPrimePlasmaGrenade : ActionUnpin
{
	void geb_ActionPrimePlasmaGrenade()
	{
		m_Text = "#STR_geb_PlasmaGrenadePrime";
	}
};

class geb_ActionDisarmPlasmaGrenade : ActionPin
{
	void geb_ActionDisarmPlasmaGrenade()
	{
		m_Text = "#STR_geb_PlasmaGrenadeDisarm";
	}
};

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);

		actions.Insert(geb_ActionPrimePlasmaGrenade);
		actions.Insert(geb_ActionDisarmPlasmaGrenade);
	}
};
