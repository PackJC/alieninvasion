class geb_GreenAlienMeat : CowSteakMeat
{
	// Cooked: a short burst of "alien vision" (night sight). Raw, burnt or rotten: food poisoning on top of
	// whatever raw meat already carries. Both amounts come from alieninvasion.json.
	override void OnConsume(float amount, PlayerBase consumer)
	{
		super.OnConsume(amount, consumer);

		if ( !consumer || !GetFoodStage() )
			return;

		switch ( GetFoodStageType() )
		{
			case FoodStageType.BAKED:
			case FoodStageType.BOILED:
			case FoodStageType.DRIED:
				consumer.StartAlienVision(GetAlienInvasionConfig().AlienVisionSeconds);
				break;

			default:
				consumer.InsertAgent(eAgents.FOOD_POISON, GetAlienInvasionConfig().BadMeatPoison);
				break;
		}
	}
};
