#pragma once
#include "e_core.h"


class RaycastEngine;


class Thing
{

public:

	RaycastEngine* engine;

	uint32_t id;

	// Sprite info
	int texture = 0;
	int spritePartIndex = 0;
	bool enableRender = true;

	// Position
	vf2d position = vf2d{ 10.0, 10.0 };

	// Collision
	bool enableCollision = true;
	float collisionSize = 0.5;

	int thingType = 0; // 0 - decoration, 1 - enemy, 2 - item


	Thing(RaycastEngine* eng);
	Thing(RaycastEngine* eng, uint32_t globId, int type, int tex, vf2d pos);
	bool operator == (const Thing& rhs) const { return (this->id == rhs.id && this->position == rhs.position && this->texture == rhs.texture); }
	bool operator != (Thing& rhs) const { return (this->id != rhs.id || this->position != rhs.position || this->texture != rhs.texture); }


	bool CheckRayCollision(vf2d start, vf2d direction, float length);
};






