#include "c_item.h"
#include "c_engine.h"



Item::Item() : Thing() {
	//std::cout << "Created Item | ID : " << id << std::endl;
};
Item::Item(uint32_t globId, int tex, vf2d pos) : Thing(globId, 2, tex, pos) {
	//std::cout << "Created Item, param  | ID : " << id << std::endl;

	enableCollision = false;
}


Thing Item::ToThing() {
	Thing thing = Thing(id, thingType, texture, position);
	thing.spritePartIndex = spritePartIndex;
	thing.enableCollision = enableCollision;
	thing.enableRender = enableRender;
	thing.collisionSize = collisionSize;
	return thing;
}


void Item::Update(float fElapsedTime) {

	engine->UpdateThings();
	CheckForPickup();

}


void Item::CheckForPickup() {
	if (!isPickedup) {
		float distanceToItem = (engine->player.position - position).mag2();
		if (distanceToItem < 0.3) {


			if (texture == 0) {
				if (engine->player.health != 100) {
					engine->player.health += 20;
					engine->InfoLog("Picked up Medkit");
					enableRender = false;
					isPickedup = true;
					std::cout << "Item picked up | ID : " << id << std::endl;
					engine->soundEngine->play2D("sounds/pickup.wav", false);

				}
				if (engine->player.health > 100) {
					engine->player.health = 100;
				}
			}

			if (texture == 1) {
				if (engine->weapons[0].currentAmmo != engine->weapons[0].maxAmmo) {
					engine->weapons[0].currentAmmo += 20;
					engine->weapons[1].currentAmmo += 20;
					engine->InfoLog("Picked up 9mm ammo");
					enableRender = false;
					isPickedup = true;
					std::cout << "Item picked up | ID : " << id << std::endl;
					engine->soundEngine->play2D("sounds/pickup.wav", false);
				}
				if (engine->weapons[0].currentAmmo > engine->weapons[0].maxAmmo || engine->weapons[1].currentAmmo > engine->weapons[1].maxAmmo) {
					engine->weapons[0].currentAmmo = engine->weapons[0].maxAmmo;
					engine->weapons[1].currentAmmo = engine->weapons[1].maxAmmo;
				}
			}

			if (texture == 2) {
				if (engine->weapons[2].currentAmmo != engine->weapons[2].maxAmmo) {
					engine->weapons[2].currentAmmo += 8;
					engine->InfoLog("Picked up shells");
					enableRender = false;
					isPickedup = true;
					std::cout << "Item picked up | ID : " << id << std::endl;
					engine->soundEngine->play2D("sounds/pickup.wav", false);

				}
				if (engine->weapons[2].currentAmmo > engine->weapons[2].maxAmmo) {
					engine->weapons[2].currentAmmo = engine->weapons[2].maxAmmo;
				}
			}

			if (texture == 3) { // Uzi

				bool changeWeaponAfterPickup = false;
				if (engine->weapons[1].onPlayer == false) {
					changeWeaponAfterPickup = true;
				}

				engine->weapons[1].onPlayer = true;
				engine->InfoLog("Picked up Uzi!");
				enableRender = false;
				isPickedup = true;
				std::cout << "Item picked up | ID : " << id << std::endl;
				engine->soundEngine->play2D("sounds/pickup.wav", false);

				if (changeWeaponAfterPickup) {
					engine->player.ChangeWeapon(1);
				}

				if (engine->weapons[0].currentAmmo != engine->weapons[0].maxAmmo) {
					engine->weapons[0].currentAmmo += 20;
					engine->weapons[1].currentAmmo += 20;
				}
				if (engine->weapons[0].currentAmmo > engine->weapons[0].maxAmmo || engine->weapons[1].currentAmmo > engine->weapons[1].maxAmmo) {
					engine->weapons[0].currentAmmo = engine->weapons[0].maxAmmo;
					engine->weapons[1].currentAmmo = engine->weapons[1].maxAmmo;
				}
			}

			if (texture == 4) { // Shotgun

				bool changeWeaponAfterPickup = false;
				if (engine->weapons[2].onPlayer == false) {
					changeWeaponAfterPickup = true;
				}

				engine->weapons[2].onPlayer = true;
				engine->InfoLog("Picked up Shotgun!");
				enableRender = false;
				isPickedup = true;
				engine->soundEngine->play2D("sounds/pickup.wav", false);

				if (changeWeaponAfterPickup) {
					engine->player.ChangeWeapon(2);
				}

				if (engine->weapons[2].currentAmmo != engine->weapons[2].maxAmmo) {
					engine->weapons[2].currentAmmo += 8;
					std::cout << "Item picked up | ID : " << id << std::endl;
				}
				if (engine->weapons[2].currentAmmo > engine->weapons[2].maxAmmo) {
					engine->weapons[2].currentAmmo = engine->weapons[2].maxAmmo;
				}
			}

		}
	}
}


