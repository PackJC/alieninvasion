// Script classes matching the config classes. Infected voices are looked up by script class name
// (<class>_<mind state>_SoundSet), so without these the aliens ran as plain ZombieBase: silent, with a
// "No sound callback for 'ZombieBase'" warning on every mind-state change.
//
// Aliens also get (all tunable in alieninvasion.json):
//  - a psychic zap on the player they're chasing, from mid range and in line of sight; a foil hat blocks it.
//    Clients see a green beam from the alien's head to the victim. Up close they use normal infected melee
//    (config AttackActions).
//  - a green burst and scream when they die.
//  - extra damage from plasma bolts, less from every other firearm.
// Unlike vanilla's infected (ZombieBase.CanBeSkinned is false) they can be skinned, like an animal.
//
// Named GreenAlienBase until 2.0.0; data/config.cpp keeps that name as a scope-1 alias of this class.
class geb_GreenAlienBase : ZombieBase
{
	protected float m_ZapCooldown;		// server: seconds until this alien may zap again
	protected float m_ZapCheckTimer;	// server: throttles the zap check to once a second
	protected bool m_geb_ApplyingScaledDamage;

	// The Skin action (ActionSkinning.ActionCondition) on a dead alien. A frozen body can't be cut, as with animals.
	override bool CanBeSkinned()
	{
		return !GetIsFrozen();
	}

	// Gas doesn't hurt them, like vanilla's NBC infected (EffectTrigger.CanAddObjectAsInsider skips creatures that
	// resist): they spawn around their own crash site, inside its toxic cloud, which slowly killed them.
	override bool ResistContaminatedEffect()
	{
		return true;
	}

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

	override bool EEOnDamageCalculated(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
	{
		// TotalDamageResult is read-only. Replace a scaled firearm hit before it is applied,
		// preserving its source, ammo, hit zone and impact-speed coefficient. The guard lets
		// the replacement pass through the normal damage/animation/death callbacks once.
		if ( m_geb_ApplyingScaledDamage || !GetGame().IsServer() || !damageResult || damageType != DamageType.FIRE_ARM || !IsAlive() )
			return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		geb_AlienInvasionConfig cfg = GetAlienInvasionConfig();
		float multiplier = cfg.OtherFirearmDamageVsAliens;
		if ( GetGame().IsKindOf(ammo, "Plasma_Cell") )		// every cell colour inherits Plasma_Cell
			multiplier = cfg.PlasmaDamageVsAliens;

		if ( multiplier == 1 )
			return super.EEOnDamageCalculated(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

		if ( multiplier > 0 )
		{
			m_geb_ApplyingScaledDamage = true;
			ProcessDirectDamage(damageType, source, dmgZone, ammo, modelPos, speedCoef * multiplier, ProcessDirectDamageFlags.ALL_TRANSFER);
			m_geb_ApplyingScaledDamage = false;
		}
		return false; // Suppress the original hit, including when the configured multiplier is zero.
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

	// Both effects played the rifle's laser (geb_plasma_shot) until 2.0.0, unrotated: its streak flies along world
	// -x, so a dying alien looked like it shot itself. Now the zap is a beam from the alien's head to the victim
	// and the death a green burst with the green blood, nothing that flies off (tools/plasmashots/alien.py).
	protected void PlayEffect(int effectRpc, vector pos)
	{
		if ( GetGame().IsDedicatedServer() )
			return;

		string soundSet = "geb_AlienZap_SoundSet";
		if ( effectRpc == GEB_RPC_ALIEN_DEATH )
		{
			soundSet = "geb_AlienDeath_SoundSet";
			PlayDeathBurst(pos);
		}
		else
		{
			PlayZapBeam(pos);
		}

		EffectSound sound = SEffectManager.PlaySound(soundSet, pos);
		if ( sound )
			sound.SetAutodestroy(true);
	}

	// The beam's bolts fly at this speed in m/s (tools/plasmashots/alien.py BEAM_SPEED)
	static const float GEB_ZAP_BEAM_SPEED = 120;

	// The zap: a beam from this alien's head to where it hit (target, the victim's chest), and a crackle there.
	// The beam's emitters fire along their +Y; VectorToAngles + "0 -90 0" turns +Y onto the direction, the way
	// vanilla points its exit and ricochet effects (BulletImpactBase). Its bolts live just long enough to arrive.
	protected void PlayZapBeam(vector target)
	{
		vector from = GetPosition() + "0 1.6 0";
		int head = GetBoneIndexByName("Head");
		if ( head != -1 )
			from = GetBonePositionWS(head);

		float distance = vector.Distance(from, target);
		if ( distance > 0.5 )
		{
			vector dir = vector.Direction(from, target).Normalized();
			from = from + dir * 0.15;	// start just in front of the face
			distance = distance - 0.15;

			vector ori = dir.VectorToAngles() + "0 -90 0";
			Particle beam = ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_ALIEN_ZAP_BEAM, from);
			if ( beam )
			{
				beam.SetOrientation(ori);
				beam.SetParameter(0, EmitorParam.LIFETIME, distance / GEB_ZAP_BEAM_SPEED);
				beam.SetParameter(1, EmitorParam.LIFETIME, distance / GEB_ZAP_BEAM_SPEED);
			}
		}

		ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_ALIEN_ZAP_HIT, target);
	}

	// The death: a green flash, crackle and ring at the chest with the green blood spray (geb_AlienBlood.c's particle)
	protected void PlayDeathBurst(vector pos)
	{
		ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_ALIEN_DEATH, pos);
		ParticleManager.GetInstance().PlayInWorld(ParticleList.GEB_ALIEN_BLOOD_EXIT, pos);
	}
};

class geb_GreenAlien : geb_GreenAlienBase
{
};
