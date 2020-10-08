#pragma once
#include "e_core.h"
#include "c_thing.h"

class Decoration : public Thing {

public:

	Decoration(RaycastEngine* eng);
	Decoration(RaycastEngine* eng, uint32_t globId, int tex, vf2d pos);

	Thing ToThing();

};


