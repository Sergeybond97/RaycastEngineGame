
#include "olcPixelGameEngine.h"


class Item : public Thing {

public:

    Item(RaycastEngine* engine) : Thing(engine);
    Item(RaycastEngine* engine, uint32_t globId, int tex, olc::vf2d pos) : Thing(engine, globId, 2 ,tex, pos);

    Thing ToThing();

    bool isPickedup;

    void Update(float fElapsedTime);
    void CheckForPickup();

};
