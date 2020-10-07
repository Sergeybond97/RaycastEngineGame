#pragma once
#include "e_core.h"

class RaycastEngine;

class Slider {

public:

	RaycastEngine* engine;


	vi2d position;

	uint32_t width;
	uint32_t height;


	bool isPressed;
	bool isHeld;
	bool isHovered;


	bool showBorder;
	bool showBackground;

	Color colorBorder;
	Color colorBackground;
	Color colorHovered;
	Color colorPressed;

	Color colorSlider;
	Color colorSliderLine;

	std::string hoverText;

	float value;


	Slider();

	Slider(vi2d pos, uint32_t w, uint32_t h);






	void Update();
};


