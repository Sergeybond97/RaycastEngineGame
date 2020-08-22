

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"



#define mapWidth 24
#define mapHeight 24

int worldMap[mapWidth][mapHeight]=
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};





// Override base class with your custom functionality
class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		// Name you application
		sAppName = "Example";
	}


	olc::Pixel DarkColor(olc::Pixel color){
		olc::Pixel darkColor = olc::Pixel(color.r/2, color.g/2, color.b/2);
		return darkColor;
	}


	olc::Sprite GetWallTexture(int idx) {
		switch(idx){
			case 0: return spritesWall1;
			case 1: return spritesWall2;
			case 2: return spritesWall3;
			case 3: return spritesWall4;
			case 4: return spritesWall5;
			default: return spritesWall5;
		}
	}

public:

	double posX = 22, posY = 12;  //x and y start position
  	double dirX = -1, dirY = 0; //initial direction vector
  	double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

	olc::Sprite spritesWall1;
	olc::Sprite spritesWall2;
	olc::Sprite spritesWall3;
	olc::Sprite spritesWall4;
	olc::Sprite spritesWall5;


	std::unique_ptr<olc::Sprite> spritesWall1ptr;
	std::unique_ptr<olc::Sprite> spritesWall2ptr;
	std::unique_ptr<olc::Sprite> spritesWall3ptr;
	std::unique_ptr<olc::Sprite> spritesWall4ptr;
	std::unique_ptr<olc::Sprite> spritesWall5ptr;

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		// Load the sprite
		spritesWall1 = olc::Sprite("./gfx/wall_stone1.png");
		spritesWall2 = olc::Sprite("./gfx/wall_brick1.png");
		spritesWall3 = olc::Sprite("./gfx/wall_metal1.png");
		spritesWall4 = olc::Sprite("./gfx/wall_stoneBlue1.png");
		spritesWall5 = olc::Sprite("./gfx/wall_wood1.png");

		std::cout << "Texture : " << spritesWall1.width << "x" << spritesWall1.height << std::endl;
		std::cout << "Texture : " << spritesWall2.width << "x" << spritesWall2.height << std::endl;
		std::cout << "Texture : " << spritesWall3.width << "x" << spritesWall3.height << std::endl;
		std::cout << "Texture : " << spritesWall4.width << "x" << spritesWall4.height << std::endl;
		std::cout << "Texture : " << spritesWall5.width << "x" << spritesWall5.height << std::endl;

		spritesWall1ptr = std::make_unique<olc::Sprite>("./gfx/wall_stone1.png");
		spritesWall2ptr = std::make_unique<olc::Sprite>("./gfx/wall_brick1.png");
		spritesWall3ptr = std::make_unique<olc::Sprite>("./gfx/wall_metal1.png");
		spritesWall4ptr = std::make_unique<olc::Sprite>("./gfx/wall_stoneBlue1.png");
		spritesWall5ptr = std::make_unique<olc::Sprite>("./gfx/wall_wood1.png");

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		//std::cout << "FillRect" << std::endl;
		// Draw background
		FillRect(0, 0, ScreenWidth(), ScreenHeight(), olc::DARK_GREY);


		DrawSprite(0,0, spritesWall1ptr.get());
		DrawSprite(20,0, spritesWall2ptr.get());
		DrawSprite(40,0, spritesWall3ptr.get());
		DrawSprite(60,0, spritesWall4ptr.get());
		DrawSprite(80,0, spritesWall5ptr.get());


		//std::cout << "Walls drawing" << std::endl;
		// Draw walls
		for (int x = 0; x < ScreenWidth(); x++) {

			//std::cout << "\tRay" << std::endl;
			//calculate ray position and direction
      		double cameraX = 2 * x / double(ScreenWidth()) - 1; //x-coordinate in camera space,  [0 - 319] pixels -> [-1 - 1]
      		double rayDirX = dirX + planeX * cameraX;
      		double rayDirY = dirY + planeY * cameraX;
			
			//which box of the map we're in
     		int mapX = int(posX);
      		int mapY = int(posY);

     		 //length of ray from current position to next x or y-side
   		    double sideDistX;
 		    double sideDistY;

  		     //length of ray from one x or y-side to next x or y-side
  		    double deltaDistX = std::abs(1 / rayDirX);
 		    double deltaDistY = std::abs(1 / rayDirY);
  		    double perpWallDist;

      		//what direction to step in x or y-direction (either +1 or -1)
      		int stepX;
      		int stepY;

		    int hit = 0; //was there a wall hit?
      		int side; //was a NS(0) or a EW(1) wall hit?


			//calculate step and initial sideDist
			if (rayDirX < 0)
			{
				stepX = -1;
				sideDistX = (posX - mapX) * deltaDistX;
			}
			else
			{
				stepX = 1;
				sideDistX = (mapX + 1.0 - posX) * deltaDistX;
			}
			if (rayDirY < 0)
			{
				stepY = -1;
				sideDistY = (posY - mapY) * deltaDistY;
			}
			else
			{
				stepY = 1;
				sideDistY = (mapY + 1.0 - posY) * deltaDistY;
			}

			int stepsDone = 0;

			//std::cout << "\tDDA" << std::endl;
			//perform DDA
			while (hit == 0 && stepsDone < 50)
			{
				//jump to next map square, OR in x-direction, OR in y-direction
				if (sideDistX < sideDistY)
				{
					sideDistX += deltaDistX;
					mapX += stepX;
					side = 0;
				}
				else
				{
					sideDistY += deltaDistY;
					mapY += stepY;
					side = 1;
				}
				stepsDone++;
				//Check if ray has hit a wall
				if (worldMap[mapX][mapY] > 0) hit = 1;
			} 


			//std::cout << "\tDistance" << std::endl;
			//Calculate distance projected on camera direction (Euclidean distance will give fisheye effect!)
			if (side == 0) perpWallDist = (mapX - posX + (1 - stepX) / 2) / rayDirX;
			else           perpWallDist = (mapY - posY + (1 - stepY) / 2) / rayDirY;

			//std::cout << "\tLine to draw" << std::endl;
			//Calculate height of line to draw on screen
			int lineHeight = (int)(ScreenHeight() / perpWallDist);

			//calculate lowest and highest pixel to fill in current stripe
			int drawStart = -lineHeight / 2 + ScreenHeight() / 2;
			if(drawStart < 0)drawStart = 0;
			int drawEnd = lineHeight / 2 + ScreenHeight() / 2;
			if(drawEnd >= ScreenHeight())drawEnd = ScreenHeight() - 1;
					

			//std::cout << "\tTexture X" << std::endl;
			// calculate X coordinate of texture
			double wallX;
			if (side == 0) wallX = posY + perpWallDist * rayDirY;
			else wallX = posX + perpWallDist * rayDirX;
			wallX -= floor(wallX); // Get floating point part	


			//choose wall texture
			olc::Pixel color;
			int textureNumber = worldMap[mapX][mapY] - 1;
			
			//std::cout << "\tTexture Y" << std::endl;
			float wallYstep = 1.0 / lineHeight;
			float wallY = wallYstep;
			if (lineHeight > ScreenHeight())
				wallY = (lineHeight - ScreenHeight()) / 2 * wallYstep;				

			//std::cout << "\tDrawing line loop..." << std::endl;
			for (int y = drawStart; y < drawEnd; y++){
				//std::cout << "\t\tSample color" << std::endl;


				switch(textureNumber){
					case 0: color = spritesWall1.Sample(wallX, wallY); break;
					case 1: color = spritesWall2.Sample(wallX, wallY); break;
					case 2: color = spritesWall3.Sample(wallX, wallY); break;
					case 3: color = spritesWall4.Sample(wallX, wallY); break;
					case 4: color = spritesWall5.Sample(wallX, wallY); break;
				}


				//color = GetWallTexture(textureNumber).Sample(wallX, wallY);
				//std::cout << "\t\tShading" << std::endl;
				if (side == 1) color = DarkColor(color);
				//std::cout << "\t\tDraw" << std::endl;
				Draw(x, y, color);
				wallY += wallYstep;
			}
		}


		//std::cout << "Movement" << std::endl;
		//speed modifiers
		double moveSpeed = fElapsedTime * 5; //the constant value is in squares/second
		double rotSpeed = fElapsedTime * 3; //the constant value is in radians/second


		//move forward if no wall in front of you
		if (GetKey(olc::Key::UP).bHeld || GetKey(olc::Key::W).bHeld)
		{
			if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
			if(worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
		}
		//move backwards if no wall behind you
		if (GetKey(olc::Key::DOWN).bHeld || GetKey(olc::Key::S).bHeld)
		{
			if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= dirX * moveSpeed;
			if(worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
		}
		//move forward if no wall in front of you
		if (GetKey(olc::Key::D).bHeld)
		{
			if(worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += planeX * moveSpeed;
			if(worldMap[int(posX)][int(posY + planeY * moveSpeed)] == false) posY += planeY * moveSpeed;
		}
		//move backwards if no wall behind you
		if (GetKey(olc::Key::A).bHeld)
		{
			if(worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= planeX * moveSpeed;
			if(worldMap[int(posX)][int(posY - planeY * moveSpeed)] == false) posY -= planeY * moveSpeed;
		}
		//rotate to the right
		if (GetKey(olc::Key::RIGHT).bHeld)
		{
			//both camera direction and camera plane must be rotated
			double oldDirX = dirX;
			dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
			dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
			planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
		}
		//rotate to the left
		if (GetKey(olc::Key::LEFT).bHeld)
		{
			//both camera direction and camera plane must be rotated
			double oldDirX = dirX;
			dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
			dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
			double oldPlaneX = planeX;
			planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
			planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
		}
		

		//std::cout << "=== END FRAME ===" << std::endl;
		return true;
	}
};

int main()
{
	Example demo;
	if (demo.Construct(320, 200, 3, 3))
		demo.Start();
	return 0;
}
