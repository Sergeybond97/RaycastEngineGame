#include "c_decoration.h"



Decoration::Decoration() : Thing() {
	//std::cout << "Created Decoration | ID : " << id << std::endl;
};
Decoration::Decoration(uint32_t globId, int tex, vf2d pos) : Thing(globId, 0, tex, pos) {
	//std::cout << "Created Decoration, param | ID : " << id << std::endl;

	// Disable collision for lamps
	if (tex == 2) {
		enableCollision = false;
	}

	// Disable collision for exit sign
	if (tex == 3) {
		enableCollision = false;
	}
}

Thing Decoration::ToThing() {
	Thing thing = Thing(id, thingType, texture, position);
	thing.spritePartIndex = spritePartIndex;
	thing.enableCollision = enableCollision;
	thing.enableRender = enableRender;
	thing.collisionSize = collisionSize;
	return thing;
}
