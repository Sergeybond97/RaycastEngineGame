#pragma once
#include "e_core.h"

class RaycastEngine;

class InputField {

public:

	RaycastEngine* engine;


	vi2d position;

	uint32_t width;
	uint32_t height;

	bool enabled;

	bool isPressed;
	bool isHovered;
	bool isFocused;

	bool showBorder;
	bool showBackground;

	Color colorBorder;
	Color colorBackground;
	Color colorText;
	Color colorHovered;
	Color colorPressed;


	std::string text;
	std::string hoverText;


	int charMaximum;
	bool allowOnlyNumbers;




	InputField();

	InputField(vi2d pos, uint32_t w, uint32_t h, std::string txt);

	InputField(vi2d pos, uint32_t w, uint32_t h, std::string txt, int maxChars);





	void Update();

};


