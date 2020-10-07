#pragma once
#include "c_thing.h"


class Item : public Thing {

public:



	Item();
	Item(uint32_t globId, int tex, vf2d pos);


	Thing ToThing();


	bool isPickedup = false;


	void Update(float fElapsedTime);


	void CheckForPickup();




};
