#pragma once
#include "e_core.h"


class RaycastEngine;


class Button {

public:

	RaycastEngine* engine;

	vi2d position;

	uint32_t width;
	uint32_t height;

	bool enabled;

	bool isPressed;
	bool isHovered;

	bool showBorder;
	bool showBackground;

	Color colorBorder;
	Color colorBackground;
	Color colorText;
	Color colorHovered;
	Color colorPressed;


	std::string text;
	std::string hoverText;
	Sprite* sprite;



	Button(RaycastEngine* eng);

	Button(RaycastEngine* eng, vi2d pos, uint32_t w, uint32_t h, std::string txt);

	Button(RaycastEngine* eng, vi2d pos, uint32_t w, uint32_t h, Sprite* img);



	void Update();

};




