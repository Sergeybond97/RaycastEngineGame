
#include "c_thing.h"
#include "c_engine.h"


Thing::Thing(RaycastEngine* eng) {
	engine = eng;
	id = 0;
	thingType = 0;
	texture = 0;
	spritePartIndex = 0;
	position = vf2d{ 10.0, 10.0 };
	enableCollision = true;
	collisionSize = 0.5;
}
Thing::Thing(RaycastEngine* eng, uint32_t globId, int type, int tex, vf2d pos) {
	engine = eng;
	id = globId;
	thingType = type;
	texture = tex;
	spritePartIndex = 0;
	position = pos;
	enableCollision = true;
	collisionSize = 0.5;
}


bool Thing::CheckRayCollision(vf2d start, vf2d direction, float length) {
	if (enableCollision) {
		vf2d spriteColPnt1 = vf2d(position.x, position.y) + direction.perp() * 0.5 * collisionSize;
		vf2d spriteColPnt2 = vf2d(position.x, position.y) - direction.perp() * 0.5 * collisionSize;

		bool interc = engine->LineIntersection
		(
			start,
			start + direction * length,
			spriteColPnt1,
			spriteColPnt2
		);
		return interc;
	}
	else {
		return false;
	}
}





