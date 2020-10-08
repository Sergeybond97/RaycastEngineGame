
#include "c_enemy.h"
#include "c_engine.h"



Enemy::Enemy(RaycastEngine* eng) : Thing(eng) {

	//std::cout << "Created Enemy | ID : " << id << std::endl;
};
Enemy::Enemy(RaycastEngine* eng, uint32_t globId, int tex, vf2d pos) : Thing(eng, globId, 1, tex, pos) {
	//std::cout << "Created Enemy, param | ID : " << id << std::endl;

	if (tex == 0) {
		health = 70;
		damage = 5;
		moveSpeed = 5;
		collisionSize = 0.8f;
	}
	else if (tex == 1) {
		health = 200;
		damage = 10;
		moveSpeed = 3;
		collisionSize = 0.8f;
	}
	else if (tex == 2) {
		health = 35;
		damage = 2;
		moveSpeed = 12;
		collisionSize = 0.8f;
	}
	else if (tex == 3) {
		health = 50;
		damage = 0;
		moveSpeed = 0;
		collisionSize = 1.0f;
	}

}


Thing Enemy::ToThing() {
	Thing thing = Thing(engine, id, thingType, texture, position);
	thing.spritePartIndex = spritePartIndex;
	thing.enableCollision = enableCollision;
	thing.enableRender = enableRender;
	thing.collisionSize = collisionSize;
	return thing;
}



void Enemy::Update(float fElapsedTime) {

	// Update things info ----------------------------------

	if (isUpdating) {
		engine->UpdateThings();
	}


	// Sprite states -------------------------------------

	spriteTimer -= fElapsedTime;

	// Change idle states

	if (spriteTimer <= 0 && enemyState == 0) {
		enemyState = 1;
		spritePartIndex = enemyState; // Change sprite part to render
		spriteTimer = 0.5f;
	}

	if (spriteTimer <= 0 && enemyState == 1) {
		enemyState = 0;
		spritePartIndex = enemyState; // Change sprite part to render
		spriteTimer = 0.5f;
	}

	// Shooting state

	if (spriteTimer <= 0 && enemyState == 2) {
		enemyState = 0;
		spritePartIndex = enemyState; // Change sprite part to render
		spriteTimer = 0.2f;
	}


	// Change dying states

	if (spriteTimer <= 0 && enemyState == 3) {
		enemyState = 4;
		spritePartIndex = enemyState; // Change sprite part to render
		spriteTimer = 0.3f;
	}

	if (spriteTimer <= 0 && enemyState == 4) {
		enemyState = 5;
		spritePartIndex = enemyState; // Change sprite part to render
		engine->UpdateThings();
		isUpdating = false;
	}



	// AI -------------------------------------------

	if (isAlive) {

		AItimer -= fElapsedTime;
		//std::cout << AItimer << std::endl;

		vf2d playerRelativePos = vf2d(position - engine->player.position) * (-1);
		bool playerInSight = false;

		// Calm state
		if (enemyAIstate == 0) {
			moveVector = vf2d(0, 0);

			// Search for player
			if (AItimer <= 0) {
				playerInSight = ShootRay(playerRelativePos.norm());
				AItimer = 0.5;
			}

			if (playerInSight && engine->player.isAlive) {
				//std::cout << "Player is found" << std::endl;
				enemyAIstate = 1;
				AItimer = 1.0;
			}
		}

		// Angry state
		if (enemyAIstate == 1) {
			moveVector = playerRelativePos.norm();


			if (AItimer <= 0) {

				//std::cout << "Try to shoot" << std::endl;

				int randAction = rand() % 1000;
				// Shoot
				if (randAction > 500) {
					playerInSight = ShootRay(playerRelativePos.norm());
					AItimer = 1.0f;
				}
				if (randAction > 800) {
					enemyAIstate = 2;
					randMoveVector = vf2d(1.0 - rand() % 2000 / 1000.0, 1.0 - rand() % 2000 / 1000.0);
					AItimer = 1.0f;
				}


				if (playerInSight) {
					//std::cout << "Shoot" << std::endl;
					enemyState = 2;
					spritePartIndex = enemyState; // Change sprite part to render
					spriteTimer = 0.2f;
					if (texture != 3) { // Boss dont make damege
						engine->player.TakeDamage(damage + (rand() % 4 - 2));
					}
				}

			}


		}

		// Running state
		if (enemyAIstate == 2) {
			moveVector = randMoveVector;

			if (AItimer <= 0) {
				enemyAIstate = 0;
				AItimer = 1.0;
			}
		}




		if (!engine->player.isAlive) {
			enemyAIstate = 0;
		}





		// Apply motion ----------------------------------

		velocity += moveSpeed * 0.001 * moveVector * fElapsedTime;

		// Clamp speed
		if (velocity.mag() > 0.006) {
			velocity -= moveSpeed * 0.001 * moveVector * fElapsedTime;
		}


		if (engine->worldMap[int(position.y) * engine->worldMapWidth + int(position.x + (velocity.x * fElapsedTime * 600) + (velocity.norm().x * 0.5))] == false)
			position.x += velocity.x * fElapsedTime * 600;

		if (engine->worldMap[int(position.y + (velocity.y * fElapsedTime * 600) + (velocity.norm().y * 0.5)) * engine->worldMapWidth + int(position.x)] == false)
			position.y += velocity.y * fElapsedTime * 600;


		// Apply friction ----------------------------------

		velocity = velocity * (1.0 - 0.1 * (fElapsedTime * 1000 / 33));

	}



}


void Enemy::TakeDamage(int amount) {
	health -= amount;
	//std::cout << "Damage, health remains : " << health << std::endl;

	spriteTimer = 0.1f;
	if (rand() % 2 == 1) {
		spritePartIndex = 6;
	}
	else {
		spritePartIndex = 7;
	}


	if (health <= 0) {
		isAlive = false;
		health = 0;
		enableCollision = false;

		enemyState = 3;
		spriteTimer = 0.3f;
		spritePartIndex = enemyState;

		//isNewGameStarted
		if (texture == 3) { // Boss
			engine->bossDefeated++;
		}

		engine->UpdateThings();
	}
}


bool Enemy::ShootRay(vf2d rayDir) {

	float rayDistance = engine->ShootRaycastRay(position, rayDir);
	//std::cout << "RD : " << rayDistance << std::endl;

	bool collide = false;
	for (int i = 0; i < engine->thingsArray.size(); i++) {

		Thing* spriteToTest = &engine->thingsArray[engine->spriteOrder[engine->thingsArray.size() - 1 - i]];

		collide = spriteToTest->CheckRayCollision(position, rayDir, rayDistance);
		if (collide)
		{
			//std::cout << "Hit : " << spriteToTest->position.x << "  " << spriteToTest->position.y << "   " << spriteToTest->texture << std::endl;
			break;
		}
	}

	if (!collide && (position - engine->player.position).mag() < rayDistance) {
		//std::cout << "HIT PLAYER : " << (position - player.position).mag() << std::endl;
		return true;
	}
	else {
		return false;
	}

}



