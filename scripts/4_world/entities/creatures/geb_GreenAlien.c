// Script classes matching the config classes. Infected voices are looked up by script class name
// (<class>_<mind state>_SoundSet), so without these the aliens ran as plain ZombieBase: silent, with a
// "No sound callback for 'ZombieBase'" warning on every mind-state change.
//
// Aliens also get (all tunable in alieninvasion.json):
//  - a psychic zap on the player they're chasing, from mid range and in line of sight; a foil hat blocks it.
//    Up close they use normal infected melee (config AttackActions).
//  - a green burst and scream when they die.
//  - extra damage from plasma bolts, less from every other firearm.
class GreenAlienBase : ZombieBase
{
	protected float m_ZapCooldown;		// server: seconds until this alien may zap again
	protected float m_ZapCheckTimer;	// server: throttles the zap check to once a second

	override bool ModCommandHandlerBefore(float pDt, int pCurrentCommandID, bool pCurrentCommandFinished)
	{
		if ( GetGame().IsServer() && IsAlive() )
			UpdatePsychicZap(pDt);

		return super.ModCommandHandlerBefore(pDt, pCurrentCommandID, pCurrentCommandFinished);
	}

	protected void UpdatePsychicZap(float dt)
	{
		m_ZapCooldown -= dt;
		m_ZapCheckTimer -= dt;
		if ( m_ZapCooldown > 0 || m_ZapCheckTimer > 0 )
			return;

		m_ZapCheckTimer = 1;

		geb_AlienInvasionConfig cfg = GetAlienInvasionConfig();
		if ( !cfg.PsychicZapEnabled )
			return;

		DayZInfectedInputController input = GetInputController();
		if ( !input )
			return;

		int mindState = input.GetMindState();
		if ( mindState != DayZInfectedConstants.MINDSTATE_CHASE && mindState != DayZInfectedConstants.MINDSTATE_FIGHT )
			return;

		PlayerBase victim = PlayerBase.Cast(input.GetTargetEntity());
		if ( !victim || !victim.IsAlive() || victim.IsWearingFoilHat() )
			return;

		float distance = vector.Distance(GetPosition(), victim.GetPosition());
		if ( distance < cfg.PsychicZapMinRange || distance > cfg.PsychicZapRange || !CanSee(victim) )
			return;

		// Same damage the old long-range "melee" dealt, now with something to see and hear
		vector hitPos = victim.ModelToWorld(victim.GetDefaultHitPosition());
		DamageSystem.CloseCombatDamageName(this, victim, victim.GetHitComponentForAI(), "MeleeZombieMale", hitPos);
		PlayEffectForAll(GEB_RPC_ALIEN_ZAP, hitPos);
		m_ZapCooldown = cfg.PsychicZapCooldown;
	}

	// Nothing between the alien's head and the target's chest
	protected bool CanSee(EntityAI target)
	{
		vector from = GetPosition() + "0 1.5 0";
		vector to = target.GetPosition() + "0 1.2 0";
		vector contactPos;
		vector contactDir;
		int contactComponent;
		set<Object> hits = new set<Object>;
		if ( !DayZPhysics.RaycastRV(from, to, contactPos, contactDir, contactComponent, hits, null, this, true, false, ObjIntersectView) )
			return true;

		if ( hits.Count() == 0 )
			return false;	// terrain in the way

		// The first thing hit may be the target's clothing or the item in their hands
		Object first = hits.Get(0);
		EntityAI firstEntity = EntityAI.Cast(first);
		return first == target || (firstEntity && firstEntity.GetHierarchyRoot() == target);
	}

	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);

		if ( GetGame().IsServer() )
			PlayEffectForAll(GEB_RPC_ALIEN_DEATH, GetPosition() + "0 1 0");
	}

	override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		// Rescale firearm damage after the fact: plasma takes extra health, other bullets get part of theirs back.
		// A hit that already killed the alien is left alone.
		if ( !GetGame().IsServer() || !damageResult || damageType != DamageType.FIRE_ARM || !IsAlive() )
			return;

		geb_AlienInvasionConfig cfg = GetAlienInvasionConfig();
		float multiplier = cfg.OtherFirearmDamageVsAliens;
		if ( ammo == "Plasma_Cell" )
			multiplier = cfg.PlasmaDamageVsAliens;

		float dealt = damageResult.GetDamage("", "Health");
		if ( dealt > 0 && multiplier != 1 )
			AddHealth("", "Health", dealt * (1 - multiplier));
	}

	// ---- Effects (server decides, every nearby client plays) ----

	protected void PlayEffectForAll(int effectRpc, vector pos)
	{
		if ( !GetGame().IsMultiplayer() )
			PlayEffect(effectRpc, pos);
		else
			RPCSingleParam(effectRpc, new Param1<vector>(pos), true);
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if ( rpc_type != GEB_RPC_ALIEN_ZAP && rpc_type != GEB_RPC_ALIEN_DEATH )
			return;

		Param1<vector> data = new Param1<vector>(vector.Zero);
		if ( ctx.Read(data) )
			PlayEffect(rpc_type, data.param1);
	}

	protected void PlayEffect(int effectRpc, vector pos)
	{
		if ( GetGame().IsDedicatedServer() )
			return;

		string soundSet = "geb_AlienZap_SoundSet";
		if ( effectRpc == GEB_RPC_ALIEN_DEATH )
			soundSet = "geb_AlienDeath_SoundSet";

		ParticleManager.GetInstance().PlayInWorld(ParticleList.geb_plasma_shot, pos);
		EffectSound sound = SEffectManager.PlaySound(soundSet, pos);
		if ( sound )
			sound.SetAutodestroy(true);
	}
};

class geb_GreenAlien : GreenAlienBase
{
};
