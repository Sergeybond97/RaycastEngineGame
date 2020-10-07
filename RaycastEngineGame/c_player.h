#pragma once
#include "e_core.h"


class RaycastEngine;

class Player
{

public:

	RaycastEngine* engine;


	// Position
	vf2d position = vf2d{ 0, 0 };
	vf2d direction = vf2d{ -1, 0 };

	vf2d viewPlane = vf2d{ 0, -0.75 }; //the 2d raycaster version of camera plane
	float cameraVertical = 0;


	// Collision
	bool enableCollision = true;
	float collisionSize = 0.5f;


	// Movement
	float moveSpeed = 5.0f;
	float rotationSpeed = 3.0f;

	vf2d playerVelocity = { 0, 0 };

	bool isMoving = false;
	bool isShooting = false;
	vf2d controlMoveVector = { 0, 0 };
	vf2d controlRotationVector = { 0 ,0 };

	// Stats
	int health = 100;
	bool isAlive = true;
	float damageEffectTimer = 0;

	// HUD and weapons
	float bobValue;
	int activeWeapon = 0;
	float weaponDelay = 0;
	int weaponState; // for changing sprites ( 0 - idle, 1 - shoot, 2 - back to idle )

	float timeToChangeWeapon = 0.6f;
	float changeWeaponTimer = 0.0f;
	int weaponToChange = 0;


	Player();

	void Update(float fElapsedTime);



	void WeaponChanging(float fElapsedTime);

	void ChangeWeapon(int weapon);




	void Move(float fElapsedTime);

	void Shoot();

	void ShootRay(float angle);

	void Interact();

	void TakeDamage(int amount);

	void Ressurect();



};