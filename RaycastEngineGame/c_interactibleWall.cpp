#include "c_interactibleWall.h"
#include "c_engine.h"



InteractibleWall::InteractibleWall() {
	engine = RaycastEngine::getInstance();
	wallPosition = vi2d(-1, -1);
	isActive = false;
	interactionType = InteractibleWall::InteractionType::NONE;
}

InteractibleWall::InteractibleWall(vi2d wallPos, InteractionType type) {
	engine = RaycastEngine::getInstance();
	wallPosition = wallPos;
	isActive = false;
	interactionType = type;
}




void InteractibleWall::Interact() {

	if (interactionType == InteractionType::NONE)
	{
		return;
	}
	else if (interactionType == InteractionType::ENDLEVEL)
	{
		std::cout << "LEVEL END" << std::endl;
		if (engine->worldMapNextMapFile != "") {
			std::string mapFile = "maps/" + engine->worldMapNextMapFile + ".map";
			engine->LoadMap(mapFile);
			engine->gameState = engine->GameState::STATE_GAMEPLAY;
			engine->PlayMapMIDI();
		}
	}
	else if (interactionType == OPENWALLS)
	{
		return;
	}

}


