

#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


// MIDI =============================================================

#include "minisdl_audio.h"

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define TML_IMPLEMENTATION
#include "tml.h"

// Holds the global instance pointer
static tsf* g_TinySoundFont;

// Holds global MIDI playback state
static double g_Msec;               //current playback time
static tml_message* g_MidiMessage;  //next message to be played



// Callback function called by the audio thread
static void AudioCallback(void* data, Uint8 *stream, int len)
{
	//Number of samples to process
	int SampleBlock, SampleCount = (len / (2 * sizeof(float))); //2 output channels
	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float))))
	{
		//We progress the MIDI playback and then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		//Loop through all MIDI messages which need to be played up until the current playback time
		for (g_Msec += SampleBlock * (1000.0 / 44100.0); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
		{
			switch (g_MidiMessage->type)
			{
			case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
				tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
				break;
			case TML_NOTE_ON: //play a note
				tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
				break;
			case TML_NOTE_OFF: //stop a note
				tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
				break;
			case TML_PITCH_BEND: //pitch wheel modification
				tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
				break;
			case TML_CONTROL_CHANGE: //MIDI controller messages
				tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
				break;
			}
		}

		// Render the block of audio samples in float format
		tsf_render_float(g_TinySoundFont, (float*)stream, SampleBlock, 0);
	}
}



tml_message* TinyMidiLoader = NULL;


void StartMIDI() {

	g_Msec = 0;

	// Define the desired audio output format we request
	SDL_AudioSpec OutputAudioSpec;
	OutputAudioSpec.freq = 44100;
	OutputAudioSpec.format = AUDIO_F32;
	OutputAudioSpec.channels = 2;
	OutputAudioSpec.samples = 4096;
	OutputAudioSpec.callback = AudioCallback;

	// Initialize the audio system
	if (SDL_AudioInit(TSF_NULL) < 0)
	{
		fprintf(stderr, "Could not initialize audio hardware or driver\n");
	}

	TinyMidiLoader = tml_load_filename("sw_underground.mid");
	if (!TinyMidiLoader)
	{
		fprintf(stderr, "Could not load MIDI file\n");
	}

	//Set up the global MidiMessage pointer to the first MIDI message
	g_MidiMessage = TinyMidiLoader;

	// Load the SoundFont from a file
	g_TinySoundFont = tsf_load_filename("gzdoom.sf2");
	if (!g_TinySoundFont)
	{
		fprintf(stderr, "Could not load SoundFont\n");
	}

	//Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);

	// Set the SoundFont rendering output mode
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, 0.0f);

	// Request the desired audio output format
	if (SDL_OpenAudio(&OutputAudioSpec, TSF_NULL) < 0)
	{
		fprintf(stderr, "Could not open the audio hardware or the desired audio output format\n");
	}

	// Start the actual audio playback here
	// The audio thread will begin to call our AudioCallback function
	SDL_PauseAudio(0);


	//Wait until the entire MIDI file has been played back (until the end of the linked message list is reached)
	//while (g_MidiMessage != NULL) SDL_Delay(100);

	// We could call tsf_close(g_TinySoundFont) and tml_free(TinyMidiLoader)
	// here to free the memory and resources but we just let the OS clean up
	// because the process ends here.

}







// =====================================================================



//sort algorithm
//sort the sprites based on distance
void sortSprites(int* order, double* dist, int amount)
{
	std::vector<std::pair<double, int>> sprites(amount);
	for (int i = 0; i < amount; i++) {
		sprites[i].first = dist[i];
		sprites[i].second = order[i];
	}
	std::sort(sprites.begin(), sprites.end());
	// restore in reverse order to go from farthest to nearest
	for (int i = 0; i < amount; i++) {
		dist[i] = sprites[amount - i - 1].first;
		order[i] = sprites[amount - i - 1].second;
	}
}


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

struct Sprite
{
	double x;
	double y;
	int texture;
	int spriteIndex;

};

#define numSprites 20

Sprite sprite[numSprites] =
{
  //row of bats
  {12.5, 10.5, 3, 0},
 // {12.5, 11.5, 3, 0},
 // {12.5, 12.5, 3, 0},


  //green lights in every room
{20.5, 11.5, 2, 0},
  {18.5,4.5, 2, 0},
  {10.0,4.5, 2, 0},
  {10.0,12.5,2, 0},
  {3.5, 6.5, 2, 0},
  {3.5, 20.5,2, 0},
  {3.5, 14.5,2, 0},
  {14.5,20.5,2, 0},

  //row of pillars in front of wall: fisheye test
  {18.5, 10.5, 1, 0},
  {18.5, 11.5, 1, 0},
  {18.5, 12.5, 1, 0},

  //some barrels around the map
  {21.5,  1.5, 0, 0},
  {15.5,  1.5, 0, 0},
  {16.0,  1.8, 0, 0},
  {16.2,  1.2, 0, 0},
  { 3.5,  2.5, 0, 0},
  { 9.5, 15.5, 0, 0},
  {10.0, 15.1, 0, 0},
  {10.5, 15.8, 0, 0},
};


//arrays used to sort the sprites
int spriteOrder[numSprites];
double spriteDistance[numSprites];

//function used to sort the sprites
void sortSprites(int* order, double* dist, int amount);




// Override base class with your custom functionality
class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		// Name you application
		sAppName = "Example";
	}


	olc::Pixel DarkColor(olc::Pixel color , float amount = 0.5){
		olc::Pixel darkColor = olc::Pixel(color.r * amount, color.g * amount, color.b * amount);
		return darkColor;
	}


	olc::Sprite* GetWallTexture(int idx) {
		switch(idx){
			case 0: return &spritesWall1;
			case 1: return &spritesWall2;
			case 2: return &spritesWall3;
			case 3: return &spritesWall4;
			case 4: return &spritesWall5;
			default: return &spritesWall5;
		}
	}


	olc::Sprite* GetSpriteTexture(int idx) {
		switch (idx) {
		case 0: return &spriteBarell;
		case 1: return &spritePillar;
		case 2: return &spriteLamp;
		case 3: return &spriteBat;
		default: return &spriteLamp;
		}
	}


	void DrawSpriteColorTransparent(int32_t x, int32_t y, olc::Sprite* sprite, olc::Pixel transparancyColor) {

		if (sprite == nullptr)
			return;

		for (int32_t i = 0; i < sprite->width; i++)
			for (int32_t j = 0; j < sprite->height; j++)
				if (sprite->GetPixel(i, j) != transparancyColor)
					Draw(x + i, y + j, sprite->GetPixel(i, j));
	}






	bool LineIntersection(olc::vf2d ln1pnt1, olc::vf2d ln1pnt2, olc::vf2d ln2pnt1, olc::vf2d ln2pnt2){
		
		olc::vf2d line1 = ln1pnt2 - ln1pnt1;
		olc::vf2d line2 = ln2pnt2 - ln2pnt1;

		float cross1 = line1.cross(ln2pnt1 - ln1pnt1);
		float cross2 = line1.cross(ln2pnt2 - ln1pnt1);

		if ( copysign(1, cross1) == copysign(1, cross2) || cross1 == 0 || cross2 == 0 )
			return false;


			
		cross1 = line2.cross(ln1pnt1 - ln2pnt1);
		cross2 = line2.cross(ln1pnt2 - ln2pnt1);

		if ( copysign(1, cross1) == copysign(1, cross2) || cross1 == 0 || cross2 == 0 )
			return false;

		return true;
	}




public:

	double posX = 22, posY = 12;  //x and y start position
  	double dirX = -1, dirY = 0; //initial direction vector
  	double planeX = 0, planeY = 0.75; //the 2d raycaster version of camera plane


	float fDistanceToWallInCenter = 0;



	olc::Sprite spritesWall1;
	olc::Sprite spritesWall2;
	olc::Sprite spritesWall3;
	olc::Sprite spritesWall4;
	olc::Sprite spritesWall5;

	olc::Sprite spriteBarell;
	olc::Sprite spritePillar;
	olc::Sprite spriteLamp;
	olc::Sprite spriteBat;



	std::unique_ptr<olc::Sprite> weaponSprite = std::make_unique<olc::Sprite>("gfx/pistol.png");
	std::unique_ptr<olc::Sprite> weaponSpriteShoot = std::make_unique<olc::Sprite>("gfx/pistolShoot.png");


	void LoadAssets(){
		
		// Load the sprite
		spritesWall1.LoadFromFile("gfx/wall_stone1.png");
		spritesWall2.LoadFromFile("gfx/wall_brick1.png");
		spritesWall3.LoadFromFile("gfx/wall_metal1.png");
		spritesWall4.LoadFromFile("gfx/wall_stoneBlue1.png");
		spritesWall5.LoadFromFile("gfx/wall_wood1.png");

		spriteBarell.LoadFromFile("gfx/barrel.png");
		spritePillar.LoadFromFile("gfx/pillar.png");
		spriteLamp.LoadFromFile("gfx/greenlight.png");
		spriteBat.LoadFromFile("gfx/bat.png");
	}






	bool *drawnPixels;


	//1D Zbuffer
	double *ZBuffer;


	int32_t prevMouseX = 0;
	float deltaMouseX = 0;


	olc::vf2d dirVector = { 0, 0 };
	olc::vf2d playerVelocity = { 0 ,0 };
	
	float playerMaxVelocity = 1.0f;
	bool bPlayerMoving = false;

	olc::vf2d rotVector = { 0 ,0 };


	//speed modifiers
	double moveSpeed = 5.0; //the constant value is in squares/second
	double rotSpeed = 3.0; //the constant value is in radians/second


	olc::vf2d playerBob = {0, 0};
	float bobValue = 0.0f;



	bool bPlayerShooting = false;










	void PlayerMovement(float fElapsedTime){

		// Apply motion ----------------------------------

		playerVelocity += moveSpeed * 4 * 0.001 * dirVector * fElapsedTime;
		// Clamp speed
		if (playerVelocity.mag() > 0.006) {
			playerVelocity -= moveSpeed * 4 * 0.001 * dirVector * fElapsedTime;
		}


		if (worldMap[int(posX + (playerVelocity.x * fElapsedTime * 600) + (playerVelocity.norm().x * 0.5) )][int(posY)] == false)
			posX += playerVelocity.x * fElapsedTime * 600;

		if(worldMap[int(posX)][int(posY + (playerVelocity.y * fElapsedTime * 600) + (playerVelocity.norm().y * 0.5) )] == false)
			posY += playerVelocity.y * fElapsedTime * 600;




		// Apply friction ----------------------------------

		playerVelocity = playerVelocity * (1.0 - 0.1 * (fElapsedTime * 1000 / 33));



		// Apply rotation ----------------------------------

		//both camera direction and camera plane must be rotated
		float rotation = fElapsedTime * rotVector.x * rotSpeed;

		double oldDirX = dirX;
		dirX = dirX * cos(rotation) - dirY * sin(rotation);
		dirY = oldDirX * sin(rotation) + dirY * cos(rotation);
		double oldPlaneX = planeX;
		planeX = planeX * cos(rotation) - planeY * sin(rotation);
		planeY = oldPlaneX * sin(rotation) + planeY * cos(rotation);
		
	}



	void UserControls(){
		

		// Clear inputs

		bPlayerMoving = false;
		dirVector = olc::vf2d(0, 0);
		rotVector = olc::vf2d(0, 0);

		bPlayerShooting = false;


		// Main controls ---------------------------------------------------------


		// Move forward
		if (GetKey(olc::Key::UP).bHeld || GetKey(olc::Key::W).bHeld)
		{
			bPlayerMoving = true;
			dirVector += olc::vf2d(dirX, dirY);
		}

		// Move backwards
		if (GetKey(olc::Key::DOWN).bHeld || GetKey(olc::Key::S).bHeld)
		{
			bPlayerMoving = true;
			dirVector += olc::vf2d(-dirX, -dirY);
		}

		// Strafe right
		if (GetKey(olc::Key::D).bHeld)
		{
			bPlayerMoving = true;
			dirVector += olc::vf2d(planeX, planeY);
		}

		// Strafe left
		if (GetKey(olc::Key::A).bHeld)
		{
			bPlayerMoving = true;
			dirVector += olc::vf2d(-planeX, -planeY);
		}

		// Rotate to the right
		if (GetKey(olc::Key::RIGHT).bHeld)
		{
			rotVector = olc::vf2d(-1, 0);
		}

		// Rotate to the left
		if (GetKey(olc::Key::LEFT).bHeld)
		{
			rotVector = olc::vf2d(1, 0);
		}



		if (GetKey(olc::Key::CTRL).bHeld)
		{
			bPlayerShooting = true;
		}
		

		
		// Mouse controls ------------------------------------------------------
		
		if (IsFocused()) {

			ShowCursor(false);

			//deltaMouseX = GetMouseX() - ScreenWidth() / 2;
			deltaMouseX = (-1) * (GetMouseX() - prevMouseX);

			rotVector += olc::vf2d(deltaMouseX, 0);

			//SetCursorPos(GetWindowPosition().x + GetWindowSize().x / 2, GetWindowPosition().y + GetWindowSize().y / 2);
			prevMouseX = GetMouseX();
		}
		else {
			//ShowCursor(true);
			LockMouse(false);
		}



	}






	bool OnUserCreate() override
	{

		LoadAssets();


		prevMouseX = GetMouseX();

		//LockMouse(true);

		// Called once at the start, so create things here
		drawnPixels = new bool[ScreenHeight() * ScreenWidth()];
		for (int i = 0; i < ScreenHeight() * ScreenWidth(); i++) drawnPixels[i] = false;
		//memcpy(&drawnPixels, 0, ScreenHeight() * ScreenWidth() * sizeof(bool));


		// Clear Z buffer
		ZBuffer = new double[ScreenWidth()];

		return true;
	}






	bool OnUserUpdate(float fElapsedTime) override
	{















		// WALL CASTING
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

			if (x == ScreenWidth() / 2) fDistanceToWallInCenter = perpWallDist;

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
				
				//std::cout << perpWallDist << std::endl;
				float shading = perpWallDist;
				if (shading > 15) shading = 15;
				shading = 1 - shading / 15;

				color = GetWallTexture(textureNumber)->Sample(wallX, wallY);
				color = DarkColor(color, shading);

				if (side == 1) color = DarkColor(color);

				Draw(x, y, color);
				wallY += wallYstep;

				drawnPixels[y * ScreenWidth() + x] = true;
			}

			//SET THE ZBUFFER FOR THE SPRITE CASTING
			ZBuffer[x] = perpWallDist; //perpendicular distance is used
		}




		//FLOOR CASTING
		for (int y = ScreenHeight() / 2; y < ScreenHeight(); y++)
		{
			// rayDir for leftmost ray (x = 0) and rightmost ray (x = w)
			float rayDirX0 = dirX - planeX;
			float rayDirY0 = dirY - planeY;
			float rayDirX1 = dirX + planeX;
			float rayDirY1 = dirY + planeY;

			// Current y position compared to the center of the screen (the horizon)
			int p = y - ScreenHeight() / 2;

			// Vertical position of the camera.
			float posZ = 0.5 * ScreenHeight();

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance = posZ / p;

			// calculate the real world step vector we have to add for each x (parallel to camera plane)
			// adding step by step avoids multiplications with a weight in the inner loop
			float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / ScreenWidth();
			float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / ScreenWidth();

			// real world coordinates of the leftmost column. This will be updated as we step to the right.
			float floorX = posX + rowDistance * rayDirX0;
			float floorY = posY + rowDistance * rayDirY0;

			for (int x = 0; x < ScreenWidth(); ++x)
			{			
				//std::cout << "floorX = " << floorX << "   floorY = " << floorY << std::endl;

				// the cell coord is simply got from the integer parts of floorX and floorY
				int cellX = (int)(floorX);
				int cellY = (int)(floorY);

				// get the texture coordinate
				floorX += floorStepX;
				floorY += floorStepY;


				float shading = 1 - (0.8 * (ScreenHeight() - y) / (ScreenHeight() / 2));
				//std::cout << a << "  " << y << std::endl;

				if (!drawnPixels[y * ScreenWidth() + x]) {
					// floor
					olc::Pixel color = GetWallTexture(0)->Sample(floorX - cellX, floorY - cellY);
					color = DarkColor(color, shading);
					Draw(x, y, color);

					//ceiling (symmetrical, at screenHeight - y - 1 instead of y)
					color = GetWallTexture(4)->Sample(floorX - cellX, floorY - cellY);
					color = DarkColor(color, shading);
					Draw(x, ScreenHeight() - y - 1, color);
				}
			}
		}




		//SPRITE CASTING
		//sort sprites from far to close
		for (int i = 0; i < numSprites; i++)
		{
			spriteOrder[i] = i;
			spriteDistance[i] = ((posX - sprite[i].x) * (posX - sprite[i].x) + (posY - sprite[i].y) * (posY - sprite[i].y)); //sqrt not taken, unneeded
		}
		sortSprites(spriteOrder, spriteDistance, numSprites);

		//after sorting the sprites, do the projection and draw them
		for (int i = 0; i < numSprites; i++)
		{
			//translate sprite position to relative to camera
			double spriteX = sprite[spriteOrder[i]].x - posX;
			double spriteY = sprite[spriteOrder[i]].y - posY;

			//std::cout << i << "  " << spriteX << "  " << spriteY << "  " << sprite[spriteOrder[i]].texture <<  std::endl;


			if (sprite[i].texture == 3){
				int spriteAngle = (int)(atan2(spriteX, spriteY) * 180 / 3.1415);
				spriteAngle = (spriteAngle + 360) % 360;
				sprite[i].spriteIndex = (int)(spriteAngle / 45);
				//std::cout << sprite[i].spriteIndex << std::endl;
			}



			//transform sprite with the inverse camera matrix
			// [ planeX   dirX ] -1                                       [ dirY      -dirX ]
			// [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
			// [ planeY   dirY ]                                          [ -planeY  planeX ]

			double invDet = 1.0 / (planeX * dirY - dirX * planeY); //required for correct matrix multiplication

			double transformX = invDet * (dirY * spriteX - dirX * spriteY);
			double transformY = invDet * (-planeY * spriteX + planeX * spriteY); //this is actually the depth inside the screen, that what Z is in 3D

			int spriteScreenX = int((ScreenWidth() / 2) * (1 + transformX / transformY));

			//calculate height of the sprite on screen
			int spriteHeight = abs(int(ScreenHeight() / (transformY))); //using 'transformY' instead of the real distance prevents fisheye
			//calculate lowest and highest pixel to fill in current stripe
			int drawStartY = -spriteHeight / 2 + ScreenHeight() / 2;
			if (drawStartY < 0) drawStartY = 0;
			int drawEndY = spriteHeight / 2 + ScreenHeight() / 2;
			if (drawEndY >= ScreenHeight()) drawEndY = ScreenHeight() - 1;

			//calculate width of the sprite
			int spriteWidth = abs(int(ScreenHeight() / (transformY)));
			int drawStartX = -spriteWidth / 2 + spriteScreenX;
			if (drawStartX < 0) drawStartX = 0;
			int drawEndX = spriteWidth / 2 + spriteScreenX;
			if (drawEndX >= ScreenWidth()) drawEndX = ScreenWidth() - 1;

			//loop through every vertical stripe of the sprite on screen
			for (int stripe = drawStartX; stripe < drawEndX; stripe++)
			{
				int texX = int(256 * (stripe - (-spriteWidth / 2 + spriteScreenX)) * 64 / spriteWidth) / 256;
				//the conditions in the if are:
				//1) it's in front of camera plane so you don't see things behind you
				//2) it's on the screen (left)
				//3) it's on the screen (right)
				//4) ZBuffer, with perpendicular distance
				if (transformY > 0 && stripe > 0 && stripe < ScreenWidth() && transformY < ZBuffer[stripe])
					for (int y = drawStartY; y < drawEndY; y++) //for every pixel of the current stripe
					{
						int d = (y) * 256 - ScreenHeight() * 128 + spriteHeight * 128; //256 and 128 factors to avoid floats
						int texY = ((d * 64) / spriteHeight) / 256;

						olc::Pixel color = GetSpriteTexture(sprite[spriteOrder[i]].texture)->GetPixel(texX + (sprite[spriteOrder[i]].spriteIndex * 64), texY);

						float shading = (spriteX * spriteX) + (spriteY * spriteY);
						if (shading > 60) shading = 60;
						shading = 1 - shading / 80;

						//Uint32 color = texture[sprite[spriteOrder[i]].texture][64 * texY + texX]; //get current color from the texture
						if (color != olc::BLACK)
						{
							Draw(stripe, y, DarkColor(color, shading)); //paint pixel if it isn't black, black is the invisible color
						}
					}
			}
		}
		//std::cout << "=================" << std::endl;







		// Draw weapon ==============================================================================

		bobValue += 1000.0f * playerVelocity.mag() * fElapsedTime;


		if (bPlayerShooting) {
			DrawSpriteColorTransparent
			(
				ScreenWidth() / 2 + 16 - sin(bobValue + 1.55) * 8,
				ScreenHeight() - 96 - (abs(sin(bobValue)) * 8),
				weaponSpriteShoot.get(),
				olc::CYAN
			);
		}
		else {
			DrawSpriteColorTransparent
			(
				ScreenWidth() / 2 + 16 - sin(bobValue + 1.55) * 8,
				ScreenHeight() - 96 - (abs(sin(bobValue)) * 8),
				weaponSprite.get(),
				olc::CYAN
			);
		}



		Draw(ScreenWidth() / 2, ScreenHeight() / 2, olc::WHITE);


		// Debug info ==============================================================================

		std::string text = "VEL : " + std::to_string(playerVelocity.x) + ", " + std::to_string(playerVelocity.y);
		DrawString(10, 10, text, olc::WHITE);

		text = "POS : " + std::to_string(posX) + ", " + std::to_string(posY);
		DrawString(10, 20, text, olc::WHITE);

		text = "DIR : " + std::to_string(dirVector.x) + ", " + std::to_string(dirVector.y);
		DrawString(10, 30, text, olc::WHITE);

		text = "AIM DIST : " + std::to_string(fDistanceToWallInCenter);
		DrawString(10, 40, text, olc::WHITE);

		
		text = "VIEW : " + std::to_string(dirX) + ", " + std::to_string(dirY);
		DrawString(10, 50, text, olc::WHITE);


		// Game logic ==============================================================================
	

		UserControls();

		PlayerMovement(fElapsedTime);



		bool interc = LineIntersection(olc::vf2d(posX, posY), olc::vf2d(posX + dirX * fDistanceToWallInCenter, posY + dirY * fDistanceToWallInCenter), olc::vf2d(0, 5), olc::vf2d(20, 5));
		
		text = "INT : " + std::to_string(interc);
		DrawString(10, 60, text, olc::WHITE);


		// MIDI ==========================================================================================

		// MIDI Looping
		if (g_MidiMessage == NULL) {
			//fprintf(stderr, "End of MIDI\n");
			StartMIDI();
		}


		// Cleaning ===================================================================================

		// Clear drawn pixels info array
		for (int i = 0; i < ScreenHeight() * ScreenWidth(); i++) drawnPixels[i] = false;




		//std::cout << "=== END FRAME ===" << std::endl;
		return true;
	}




};

int main()
{

	StartMIDI();


	Example demo;
	if (demo.Construct(320, 200, 2, 2))
		demo.Start();
	return 0;
}
