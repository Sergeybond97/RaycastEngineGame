#include "c_interactibleWall.h"
#include "c_engine.h"



InteractibleWall::InteractibleWall(RaycastEngine* eng) {
	engine = eng;
	wallPosition = vi2d(-1, -1);
	isActive = false;
	interactionType = InteractibleWall::InteractionType::NONE;
}

InteractibleWall::InteractibleWall(RaycastEngine* eng, vi2d wallPos, InteractionType type) {
	engine = eng;
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


