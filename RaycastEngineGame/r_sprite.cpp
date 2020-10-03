
#include "r_sprite.h"


Sprite::Sprite() {
	pColData = nullptr; width = 0; height = 0;
}

//Sprite::Sprite(const std::string& sImageFile) {
//	LoadFromFile(sImageFile);
//}

Sprite::Sprite(int32_t w, int32_t h) {
	if (pColData) delete[] pColData;
	width = w;		height = h;
	pColData = new Color[width * height];
	for (int32_t i = 0; i < width * height; i++)
		pColData[i] = Color();
}

Sprite::~Sprite() {
	if (pColData) delete[] pColData;
}


//olc::rcode Sprite::LoadFromFile(const std::string& sImageFile) {
//	return loader->LoadImageResource(this, sImageFile);
//}


Color Sprite::GetPixel(int32_t x, int32_t y) {
	return pColData[abs(y % height) * width + abs(x % width)];
}

bool Sprite::SetPixel(int32_t x, int32_t y, Color p) {
	if (x >= 0 && x < width && y >= 0 && y < height)
	{
		pColData[y * width + x] = p;
		return true;
	}
	else
		return false;
}

Color Sprite::GetPixel(const olc::vi2d& a) {
	return GetPixel(a.x, a.y);
}

bool  Sprite::SetPixel(const olc::vi2d& a, Color p) {
	return SetPixel(a.x, a.y, p);
}

Color Sprite::Sample(float x, float y) {
	int32_t sx = std::min((int32_t)((x * (float)width)), width - 1);
	int32_t sy = std::min((int32_t)((y * (float)height)), height - 1);
	return GetPixel(sx, sy);
}

Color Sprite::SampleBL(float u, float v) {
	u = u * width - 0.5f;
	v = v * height - 0.5f;
	int x = (int)floor(u); // cast to int rounds toward zero, not downward
	int y = (int)floor(v); // Thanks @joshinils
	float u_ratio = u - x;
	float v_ratio = v - y;
	float u_opposite = 1 - u_ratio;
	float v_opposite = 1 - v_ratio;

	Color p1 = GetPixel(std::max(x, 0), std::max(y, 0));
	Color p2 = GetPixel(std::min(x + 1, (int)width - 1), std::max(y, 0));
	Color p3 = GetPixel(std::max(x, 0), std::min(y + 1, (int)height - 1));
	Color p4 = GetPixel(std::min(x + 1, (int)width - 1), std::min(y + 1, (int)height - 1));

	return Color(
		(uint8_t)((p1.r * u_opposite + p2.r * u_ratio) * v_opposite + (p3.r * u_opposite + p4.r * u_ratio) * v_ratio),
		(uint8_t)((p1.g * u_opposite + p2.g * u_ratio) * v_opposite + (p3.g * u_opposite + p4.g * u_ratio) * v_ratio),
		(uint8_t)((p1.b * u_opposite + p2.b * u_ratio) * v_opposite + (p3.b * u_opposite + p4.b * u_ratio) * v_ratio));
}




