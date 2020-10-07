#include "c_player.h"
#include "c_engine.h"


Player::Player() {
	engine = RaycastEngine::getInstance();
}


void Player::Update(float fElapsedTime) {

	if (isAlive) {

		Move(fElapsedTime);
		WeaponChanging(fElapsedTime);

		// Bobbing
		if (changeWeaponTimer == 0) {
			bobValue += 1000.0f * playerVelocity.mag() * fElapsedTime;
		}

		// Weapon states
		if (weaponDelay > 0) {
			weaponDelay -= fElapsedTime;

			float weaponStateTime = engine->weapons[activeWeapon].fireRate / 3;

			if (weaponDelay < weaponStateTime * 2) {
				weaponState = 2;
			}

			if (weaponState == 2 && weaponDelay < weaponStateTime) {
				weaponState = 0;
			}

			if (weaponDelay <= 0) {
				weaponDelay = 0;
			}
		}

	}
	else {

		if (cameraVertical > -20) {
			cameraVertical -= 20 * fElapsedTime;
		}


	}

}



void Player::WeaponChanging(float fElapsedTime) {
	if (changeWeaponTimer > 0) {
		changeWeaponTimer -= fElapsedTime;
	}

	if (changeWeaponTimer < timeToChangeWeapon / 2) {
		activeWeapon = weaponToChange;
	}

	if (changeWeaponTimer < 0) {
		changeWeaponTimer = 0;
	}
}

void Player::ChangeWeapon(int weapon) {
	if (changeWeaponTimer == 0 && activeWeapon != weapon && engine->weapons[weapon].onPlayer == true) {
		changeWeaponTimer = timeToChangeWeapon;
		weaponState = 0;
		weaponDelay = 0;
		weaponToChange = weapon;
	}
}




void Player::Move(float fElapsedTime) {

	// Apply motion ----------------------------------

	playerVelocity += moveSpeed * 4 * 0.001 * controlMoveVector * fElapsedTime;
	// Clamp speed
	if (playerVelocity.mag() > 0.006) {
		playerVelocity -= moveSpeed * 4 * 0.001 * controlMoveVector * fElapsedTime;
	}


	if (engine->worldMap[int(position.y) * engine->worldMapWidth + int(position.x + (playerVelocity.x * fElapsedTime * 600) + (playerVelocity.norm().x * 0.1))] == false)
		position.x += playerVelocity.x * fElapsedTime * 600;

	if (engine->worldMap[int(position.y + (playerVelocity.y * fElapsedTime * 600) + (playerVelocity.norm().y * 0.1)) * engine->worldMapWidth + int(position.x)] == false)
		position.y += playerVelocity.y * fElapsedTime * 600;


	// Apply friction ----------------------------------

	playerVelocity = playerVelocity * (1.0 - 0.1 * (fElapsedTime * 1000 / 33));


	// Apply rotation ----------------------------------

	//both camera direction and camera plane must be rotated
	float rotation = fElapsedTime * controlRotationVector.x * rotationSpeed;

	float oldDirX = direction.x;
	direction.x = direction.x * cos(rotation) - direction.y * sin(rotation);
	direction.y = oldDirX * sin(rotation) + direction.y * cos(rotation);
	float oldPlaneX = viewPlane.x;
	viewPlane.x = viewPlane.x * cos(rotation) - viewPlane.y * sin(rotation);
	viewPlane.y = oldPlaneX * sin(rotation) + viewPlane.y * cos(rotation);
}

void Player::Shoot() {

	if (weaponState == 0 && weaponDelay == 0 && changeWeaponTimer == 0) { // If weapon is idle

		if (engine->weapons[activeWeapon].currentAmmo > 0) { // If player have ammo


			// Change weapon state
			weaponState = 1; // Shooting
			weaponDelay = engine->weapons[activeWeapon].fireRate;

			// Shooting

			engine->weapons[activeWeapon].currentAmmo--;

			// Weapon 0 and 1 (pistol and uzi) have same ammo, so i equal them
			if (activeWeapon == 0) engine->weapons[1].currentAmmo = engine->weapons[activeWeapon].currentAmmo;
			if (activeWeapon == 1) engine->weapons[0].currentAmmo = engine->weapons[activeWeapon].currentAmmo;


			if (activeWeapon == 0) { // Pistol and Uzi
				engine->soundEngine->play2D("sounds/pistol.wav", false);
				ShootRay(0);
			}
			if (activeWeapon == 1) { // Shotgun
				engine->soundEngine->play2D("sounds/uzi.wav", false);
				ShootRay(0);
			}
			if (activeWeapon == 2) { // Shotgun
				engine->soundEngine->play2D("sounds/shotgun.wav", false);
				for (int s = 0; s < 20; s++)
					ShootRay(-0.75 + (s / 19.0) * 1.5); // from -0.5 to  0.5
			}

		}


	}
}

void Player::ShootRay(float angle) {

	//std::cout << "Shoot vector : " << direction + viewPlane * angle << std::endl;

	float rayDistance = engine->ShootRaycastRay(position, direction + viewPlane * angle);
	//aimDist = std::to_string(rayDistance);

	for (int i = 0; i < engine->thingsArray.size(); i++) {

		Thing* spriteToTest = &engine->thingsArray[engine->spriteOrder[engine->thingsArray.size() - 1 - i]];

		bool collide = spriteToTest->CheckRayCollision(position, direction, rayDistance);
		if (collide)
		{
			//std::cout << "Hit : " << spriteToTest->position.x << "  " << spriteToTest->position.y << "   " << spriteToTest->texture << std::endl;
			Enemy* hittedEnemy = engine->GetEnemyByID(spriteToTest->id);
			if (hittedEnemy != nullptr) {
				//std::cout << "Hited enemy found | ID:  " << hittedEnemy->id << std::endl;
				hittedEnemy->TakeDamage(engine->weapons[activeWeapon].damage);
				hittedEnemy = nullptr;
			}
			break;
		}
	}
}

void Player::Interact() {

	for (int i = 0; i < engine->interactbleWallsArray.size(); i++) {

		vi2d interactionVector = position + direction * 0.5;
		std::cout << "Interaction : " << interactionVector << std::endl;

		if (interactionVector == engine->interactbleWallsArray[i].wallPosition) {
			std::cout << "Activated wall : " << engine->interactbleWallsArray[i].wallPosition << std::endl;
			engine->interactbleWallsArray[i].isActive = true;
			engine->interactbleWallsArray[i].Interact();
		}

	}
}

void Player::TakeDamage(int amount) {
	damageEffectTimer += 0.1f;

	health -= amount;

	if (health <= 0) {
		health = 0;
		isAlive = false;
	}
}

void Player::Ressurect() {
	cameraVertical = 0;
	health = 100;
	isAlive = true;

	engine->weapons[0].currentAmmo = 30;
	engine->weapons[1].currentAmmo = 30;
	engine->weapons[2].currentAmmo = 0;

	engine->weapons[1].onPlayer = false;
	engine->weapons[2].onPlayer = false;

	activeWeapon = 0;
	weaponState = 0;
	weaponDelay = 0;
}
