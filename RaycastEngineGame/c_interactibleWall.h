#pragma once
#include "e_core.h"

class RaycastEngine;


class InteractibleWall {

public:

	RaycastEngine* engine;

	vi2d wallPosition;
	bool isActive;

	enum InteractionType { NONE, ENDLEVEL, OPENWALLS };
	InteractionType interactionType;

	std::vector<int> wallsToOpen;


	InteractibleWall(RaycastEngine* eng);
	InteractibleWall(RaycastEngine* eng, vi2d wallPos, InteractionType type);



	void Interact();
};

