#include "ui_inputField.h"
#include "c_engine.h"



InputField::InputField(RaycastEngine* eng) {
	engine = eng;
	position = vi2d(0, 0);
	width = 20;
	height = 40;
	enabled = true;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorText = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	text = "BTN";
	hoverText = "";
	charMaximum = 20;
	allowOnlyNumbers = false;
}

InputField::InputField(RaycastEngine* eng, vi2d pos, uint32_t w, uint32_t h, std::string txt) {
	engine = eng;
	position = pos;
	width = w;
	height = h;
	enabled = true;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorText = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	text = txt;
	hoverText = "";
	charMaximum = 20;
	allowOnlyNumbers = false;
}

InputField::InputField(RaycastEngine* eng, vi2d pos, uint32_t w, uint32_t h, std::string txt, int maxChars) {
	engine = eng;
	position = pos;
	width = w;
	height = h;
	enabled = true;
	isPressed = false;
	isHovered = false;
	showBorder = true;
	showBackground = true;
	colorBorder = GREY;
	colorBackground = BLACK;
	colorText = WHITE;
	colorHovered = YELLOW;
	colorPressed = DARK_YELLOW;
	text = txt;
	hoverText = "";
	charMaximum = maxChars;
	allowOnlyNumbers = false;
}





void InputField::Update() {


	// Check mouse input

	isPressed = false;
	isHovered = false;

	vi2d mousePos = vi2d(engine->GetMouseX(), engine->GetMouseY());
	if (mousePos.x > position.x && mousePos.x < position.x + width && mousePos.y > position.y && mousePos.y < position.y + height)
	{
		isHovered = true;
	}
	else if (isFocused && engine->GetMouse(0).bPressed) {
		isFocused = false;
	}

	if (isHovered && engine->GetMouse(0).bPressed) {
		isPressed = true;
	}



	// Draw field

	if (showBorder) {
		engine->DrawRect(position.x, position.y, width, height, colorBorder); // Border
		if (showBackground) {
			engine->FillRect(position.x + 1, position.y + 1, width - 1, height - 1, colorBackground); // Background
		}
	}
	else {
		if (showBackground) {
			engine->FillRect(position.x, position.y, width, height, colorBackground); // Background
		}
	}

	// Draw selection
	if (isHovered) {
		engine->DrawRect(position.x, position.y, width, height, colorHovered);
	}
	if (isFocused) {
		engine->DrawRect(position.x, position.y, width, height, RED);
	}

	// Draw content

	if (text != "") {
		engine->DrawString(position.x + 6, position.y + (height / 2 - 3), text, colorText);
	}


	// Handle input from keyboard

	if (isFocused) {

		if (engine->GetKey(Key::BACK).bPressed) text = text.substr(0, text.length() - 1);

		if (text.length() < charMaximum) {

			if (!allowOnlyNumbers) {
				if (engine->GetKey(Key::Q).bPressed) text += "Q";
				if (engine->GetKey(Key::W).bPressed) text += "W";
				if (engine->GetKey(Key::E).bPressed) text += "E";
				if (engine->GetKey(Key::R).bPressed) text += "R";
				if (engine->GetKey(Key::T).bPressed) text += "T";
				if (engine->GetKey(Key::Y).bPressed) text += "Y";
				if (engine->GetKey(Key::U).bPressed) text += "U";
				if (engine->GetKey(Key::I).bPressed) text += "I";
				if (engine->GetKey(Key::O).bPressed) text += "O";
				if (engine->GetKey(Key::P).bPressed) text += "P";
				if (engine->GetKey(Key::A).bPressed) text += "A";
				if (engine->GetKey(Key::S).bPressed) text += "S";
				if (engine->GetKey(Key::D).bPressed) text += "D";
				if (engine->GetKey(Key::F).bPressed) text += "F";
				if (engine->GetKey(Key::G).bPressed) text += "G";
				if (engine->GetKey(Key::H).bPressed) text += "H";
				if (engine->GetKey(Key::J).bPressed) text += "J";
				if (engine->GetKey(Key::K).bPressed) text += "K";
				if (engine->GetKey(Key::L).bPressed) text += "L";
				if (engine->GetKey(Key::Z).bPressed) text += "Z";
				if (engine->GetKey(Key::X).bPressed) text += "X";
				if (engine->GetKey(Key::C).bPressed) text += "C";
				if (engine->GetKey(Key::V).bPressed) text += "V";
				if (engine->GetKey(Key::B).bPressed) text += "B";
				if (engine->GetKey(Key::N).bPressed) text += "N";
				if (engine->GetKey(Key::M).bPressed) text += "M";
			}

			if (engine->GetKey(Key::K0).bPressed) text += "0";
			if (engine->GetKey(Key::K1).bPressed) text += "1";
			if (engine->GetKey(Key::K2).bPressed) text += "2";
			if (engine->GetKey(Key::K3).bPressed) text += "3";
			if (engine->GetKey(Key::K4).bPressed) text += "4";
			if (engine->GetKey(Key::K5).bPressed) text += "5";
			if (engine->GetKey(Key::K6).bPressed) text += "6";
			if (engine->GetKey(Key::K7).bPressed) text += "7";
			if (engine->GetKey(Key::K8).bPressed) text += "8";
			if (engine->GetKey(Key::K9).bPressed) text += "9";
		}
	}
}

