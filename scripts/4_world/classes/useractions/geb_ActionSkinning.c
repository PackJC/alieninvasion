// Gutting an alien leaves green blood on bare hands. Decided before vanilla's OnFinishProgressServer, which
// bloodies the hands and queues the body for deletion; gutting anything else turns them back to red.
// An alien's cargo (the spare cartridge some carry) is dropped beside it first: vanilla deletes the body with
// whatever it holds, which never mattered while infected couldn't be skinned.
modded class ActionSkinning
{
	override void OnFinishProgressServer(ActionData action_data)
	{
		EntityAI body = EntityAI.Cast(action_data.m_Target.GetObject());
		PluginLifespan lifespan = PluginLifespan.Cast(GetPlugin(PluginLifespan));
		if ( body && lifespan && lifespan.CanMakeHandsBloody(action_data.m_Player) )
			action_data.m_Player.geb_SetAlienBloodHands(body.IsInherited(geb_GreenAlienBase));

		if ( body && body.IsInherited(geb_GreenAlienBase) )
			geb_DropAlienInventory(body);

		super.OnFinishProgressServer(action_data);
	}

	// Everything the alien holds directly goes to the ground; a container keeps its own contents.
	protected void geb_DropAlienInventory(EntityAI body)
	{
		array<EntityAI> children = new array<EntityAI>;
		body.GetInventory().EnumerateInventory(InventoryTraversalType.LEVELORDER, children);
		foreach (EntityAI child : children)
		{
			if ( child && child.GetHierarchyParent() == body )
				body.GetInventory().DropEntity(InventoryMode.SERVER, body, child);
		}
	}
}
