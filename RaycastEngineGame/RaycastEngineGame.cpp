

#include "olcPixelGameEngine.h"

#include "c_engine.h"





//===============================================================================================================================
// PIXEL GAME ENGINE FUNCTIONS
//===============================================================================================================================




class PGE : public olc::PixelGameEngine
{
public:

	RaycastEngine* gameEngine;
	RECore core = RECore(this);

	PGE() {
		sAppName = "RaycastEngine";

		gameEngine = RaycastEngine::getInstance();
		gameEngine->engineCore = core;
	}

	bool OnUserCreate() override {
		gameEngine->GameStart();

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override {
		gameEngine->GameUpdate(fElapsedTime);

		return true;
	}
};

PGE pixelGameEngine = PGE();








//===============================================================================================================================
// MAIN() 
//===============================================================================================================================



int main()
{


	ShowCursor(false);



	if (pixelGameEngine.Construct(320, 200, 2, 2, false, true))
		pixelGameEngine.Start();


	// After we are finished, we have to delete the irrKlang Device created earlier
	// with createIrrKlangDevice(). Use ::drop() to do that. In irrKlang, you should
	// delete all objects you created with a method or function that starts with 'create'.
	// (an exception is the play2D()- or play3D()-method, see the documentation or the
	// next example for an explanation)
	// The object is deleted simply by calling ->drop().
	//soundEngine->drop(); // delete engine

	return 0;
}
