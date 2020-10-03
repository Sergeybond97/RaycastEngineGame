#pragma once
#include <iostream>

// 32-Bit RGBA colour
struct Color
{
	union
	{
		uint32_t n = 0;
		struct { uint8_t r; uint8_t g; uint8_t b; uint8_t a; };
	};

	Color() {
		r = 0; g = 0; b = 0; a = 255;
	}

	Color(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255) {
		n = red | (green << 8) | (blue << 16) | (alpha << 24);
	}

	Color(uint32_t p) {
		n = p;
	}

	bool operator==(const Color& p) {
		return n == p.n;
	}

	bool operator!=(const Color& p) {
		return n != p.n;
	}

};


static const Color
GREY(192, 192, 192), DARK_GREY(128, 128, 128), VERY_DARK_GREY(64, 64, 64),
RED(255, 0, 0), DARK_RED(128, 0, 0), VERY_DARK_RED(64, 0, 0),
YELLOW(255, 255, 0), DARK_YELLOW(128, 128, 0), VERY_DARK_YELLOW(64, 64, 0),
GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
CYAN(0, 255, 255), DARK_CYAN(0, 128, 128), VERY_DARK_CYAN(0, 64, 64),
BLUE(0, 0, 255), DARK_BLUE(0, 0, 128), VERY_DARK_BLUE(0, 0, 64),
MAGENTA(255, 0, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64),
WHITE(255, 255, 255), BLACK(0, 0, 0), BLANK(0, 0, 0, 0);

