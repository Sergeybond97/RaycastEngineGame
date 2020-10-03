
#include "o_item.h"


class Item : public Thing {

	public:

		Item(RaycastEngine* engine) : Thing(engine) {
			//std::cout << "Created Item | ID : " << id << std::endl;
		};
		Item(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 2 ,tex, pos) {
			//std::cout << "Created Item, param  | ID : " << id << std::endl;

			enableCollision = false;
		}


		Thing ToThing() {
			Thing thing = Thing(engineReference, id, thingType, texture, position);
			thing.spritePartIndex = spritePartIndex;
			thing.enableCollision = enableCollision;
			thing.enableRender = enableRender;
			thing.collisionSize = collisionSize;
			return thing;
		}


		bool isPickedup = false;


		void Update(float fElapsedTime) {

			engineReference -> UpdateThings();
			CheckForPickup();

		}


		void CheckForPickup() {
			if (!isPickedup) {
				float distanceToItem = (engineReference->player.position - position).mag2();
				if (distanceToItem < 0.3) {


					if (texture == 0) {
						if (engineReference->player.health != 100) {
							engineReference->player.health += 20;
							engineReference->InfoLog("Picked up Medkit");
							enableRender = false;
							isPickedup = true;
							std::cout << "Item picked up | ID : " << id << std::endl;
							soundEngine->play2D("sounds/pickup.wav", false);

						}
						if (engineReference->player.health > 100) {
							engineReference->player.health = 100;
						}
					}

					if (texture == 1) {
						if (engineReference->weapons[0].currentAmmo != engineReference->weapons[0].maxAmmo) {
							engineReference->weapons[0].currentAmmo += 20;
							engineReference->weapons[1].currentAmmo += 20;
							engineReference->InfoLog("Picked up 9mm ammo");
							enableRender = false;
							isPickedup = true;
							std::cout << "Item picked up | ID : " << id << std::endl;
							soundEngine->play2D("sounds/pickup.wav", false);
						}
						if (engineReference->weapons[0].currentAmmo > engineReference->weapons[0].maxAmmo || engineReference->weapons[1].currentAmmo > engineReference->weapons[1].maxAmmo) {
							engineReference->weapons[0].currentAmmo = engineReference->weapons[0].maxAmmo;
							engineReference->weapons[1].currentAmmo = engineReference->weapons[1].maxAmmo;
						}
					}

					if (texture == 2) {
						if (engineReference->weapons[2].currentAmmo != engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo += 8;
							engineReference->InfoLog("Picked up shells");
							enableRender = false;
							isPickedup = true;
							std::cout << "Item picked up | ID : " << id << std::endl;
							soundEngine->play2D("sounds/pickup.wav", false);

						}
						if (engineReference->weapons[2].currentAmmo > engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo = engineReference->weapons[2].maxAmmo;
						}
					}

					if (texture == 3) { // Uzi

						bool changeWeaponAfterPickup = false;
						if (engineReference->weapons[1].onPlayer == false) {
							changeWeaponAfterPickup = true;
						}

						engineReference->weapons[1].onPlayer = true;
						engineReference->InfoLog("Picked up Uzi!");
						enableRender = false;
						isPickedup = true;
						std::cout << "Item picked up | ID : " << id << std::endl;
						soundEngine->play2D("sounds/pickup.wav", false);

						if (changeWeaponAfterPickup) {
							engineReference->player.ChangeWeapon(1);
						}

						if (engineReference->weapons[0].currentAmmo != engineReference->weapons[0].maxAmmo) {
							engineReference->weapons[0].currentAmmo += 20;
							engineReference->weapons[1].currentAmmo += 20;
						}
						if (engineReference->weapons[0].currentAmmo > engineReference->weapons[0].maxAmmo || engineReference->weapons[1].currentAmmo > engineReference->weapons[1].maxAmmo) {
							engineReference->weapons[0].currentAmmo = engineReference->weapons[0].maxAmmo;
							engineReference->weapons[1].currentAmmo = engineReference->weapons[1].maxAmmo;
						}
					}

					if (texture == 4) { // Shotgun

						bool changeWeaponAfterPickup = false;
						if (engineReference->weapons[2].onPlayer == false) {
							changeWeaponAfterPickup = true;
						}

						engineReference->weapons[2].onPlayer = true;
						engineReference->InfoLog("Picked up Shotgun!");
						enableRender = false;
						isPickedup = true;
						soundEngine->play2D("sounds/pickup.wav", false);

						if (changeWeaponAfterPickup) {
							engineReference->player.ChangeWeapon(2);
						}

						if (engineReference->weapons[2].currentAmmo != engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo += 8;				
							std::cout << "Item picked up | ID : " << id << std::endl;
						}
						if (engineReference->weapons[2].currentAmmo > engineReference->weapons[2].maxAmmo) {
							engineReference->weapons[2].currentAmmo = engineReference->weapons[2].maxAmmo;
						}
					}

				}
			}
		}

	};
