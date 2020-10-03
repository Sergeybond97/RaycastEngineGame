#pragma once
#include <iostream>
#include "c_thing.h"



class Enemy : public Thing {

public:
	Enemy();
	Enemy(uint32_t globId, int tex, vf2d pos);


	Thing ToThing();


	int enemyState = 0; // 0 - idle0,    1 - idle1,    2 - atack,    3 - dying0,    4 - dying1,    5 - dying3
	float spriteTimer = 0;

	int enemyAIstate = 0; // 0 - calm,   1 - folowing,    2 - Running back
	float AItimer = 0;


	int health = 100;
	bool isAlive = true;
	bool isUpdating = true;

	int damage = 5;


	vf2d velocity{ 0,0 };
	vf2d moveVector{ 1, 1 };
	vf2d randMoveVector{ 1, 1 };
	float moveSpeed = 5;




	void Update(float fElapsedTime);

	void TakeDamage(int amount);

	bool ShootRay(vf2d rayDir);


};


