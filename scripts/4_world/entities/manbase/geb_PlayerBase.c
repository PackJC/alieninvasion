modded class PlayerBase
{
	// Aliens can't lock on to someone in an intact foil hat unless they're right next to them (FoilHatDetectRange).
	override bool CanBeTargetedByAI(EntityAI ai)
	{
		if ( !super.CanBeTargetedByAI(ai) )
			return false;

		// Hat check first: this runs for every infected evaluating every player, and the hat is the rare case.
		if ( ai && IsWearingFoilHat() && ai.IsInherited(GreenAlienBase) )
			return vector.Distance(ai.GetPosition(), GetPosition()) <= GetAlienInvasionConfig().FoilHatDetectRange;

		return true;
	}

	bool IsWearingFoilHat()
	{
		EntityAI hat = GetInventory().FindAttachment(InventorySlots.HEADGEAR);
		return hat && hat.IsInherited(geb_FoilHat) && !hat.IsRuined();
	}

	//! Server side: night sight for `seconds` on this player's screen (cooked alien meat).
	void StartAlienVision(float seconds)
	{
		if ( !GetGame().IsMultiplayer() )
			ApplyAlienVision(seconds);
		else if ( GetIdentity() )
			RPCSingleParam(GEB_RPC_ALIEN_VISION, new Param1<float>(seconds), true, GetIdentity());
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if ( rpc_type != GEB_RPC_ALIEN_VISION )
			return;

		Param1<float> data = new Param1<float>(0);
		if ( ctx.Read(data) )
			ApplyAlienVision(data.param1);
	}

	// Uses the pumpkin helmet's vision mode so it never fights with real NV goggles. Eating again restarts the timer.
	protected void ApplyAlienVision(float seconds)
	{
		geb_AlienVision.s_Active = true;	// before AddActiveNV so the camera picks the green tint
		AddActiveNV(NVTypes.NV_PUMPKIN);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(EndAlienVision);
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(EndAlienVision, seconds * 1000, false);
	}

	override void EEDelete(EntityAI parent)
	{
		super.EEDelete(parent);

		// A local player deleted mid-effect (disconnect) never reaches EndAlienVision
		GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).Remove(EndAlienVision);
		if ( GetGame().GetPlayer() == this )
			geb_AlienVision.s_Active = false;
	}

	protected void EndAlienVision()
	{
		geb_AlienVision.s_Active = false;

		// A worn pumpkin helmet keeps the same vision mode on; leave it to the helmet.
		EntityAI headgear = GetInventory().FindAttachment(InventorySlots.HEADGEAR);
		if ( headgear && headgear.IsInherited(PumpkinHelmet) )
			return;

		RemoveActiveNV(NVTypes.NV_PUMPKIN);
	}
};
