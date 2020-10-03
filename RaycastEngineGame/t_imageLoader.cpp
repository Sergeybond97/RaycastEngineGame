#include "t_imageLoader.h"


static class GDIPlusStartup
{
public:
	GDIPlusStartup()
	{
		Gdiplus::GdiplusStartupInput startupInput;
		ULONG_PTR	token;
		Gdiplus::GdiplusStartup(&token, &startupInput, NULL);
	};
} gdistartup;


std::wstring ConvertS2W(std::string s)
{
	int count = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);
	wchar_t* buffer = new wchar_t[count];
	MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, buffer, count);
	std::wstring w(buffer);
	delete[] buffer;
	return w;
}


ImageLoader::ImageLoader() {}

ImageLoader::~ImageLoader() {}

olc::rcode ImageLoader::LoadImageResource(Sprite* spr, const std::string& sImageFile)
{
	// Check file exists
	if (!_gfs::exists(sImageFile)) return olc::rcode::NO_FILE;

	// It does, so clear out existing sprite
	if (spr->pColData != nullptr) delete[] spr->pColData;

	// Open file
	Gdiplus::Bitmap* bmp = nullptr;

	// Load sprite from file
	bmp = Gdiplus::Bitmap::FromFile(ConvertS2W(sImageFile).c_str());


	if (bmp->GetLastStatus() != Gdiplus::Ok) return olc::rcode::FAIL;
	spr->width = bmp->GetWidth();
	spr->height = bmp->GetHeight();
	spr->pColData = new Color[spr->width * spr->height];

	for (int y = 0; y < spr->height; y++)
		for (int x = 0; x < spr->width; x++)
		{
			Gdiplus::Color c;
			bmp->GetPixel(x, y, &c);
			spr->SetPixel(x, y, Color(c.GetRed(), c.GetGreen(), c.GetBlue(), c.GetAlpha()));
		}
	delete bmp;
	return olc::rcode::OK;
}



