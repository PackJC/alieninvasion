// Gutting an alien leaves green blood on bare hands. Decided before vanilla's OnFinishProgressServer, which
// bloodies the hands and queues the body for deletion; gutting anything else turns them back to red.
modded class ActionSkinning
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		EntityAI body = EntityAI.Cast(action_data.m_Target.GetObject());
		PluginLifespan lifespan = PluginLifespan.Cast(GetPlugin(PluginLifespan));
		if ( body && lifespan && lifespan.CanMakeHandsBloody(action_data.m_Player) )
			action_data.m_Player.geb_SetAlienBloodHands(body.IsInherited(GreenAlienBase));

		super.OnFinishProgressServer(action_data);
	}
}
