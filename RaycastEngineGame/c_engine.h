#pragma once

#include "c_vector.h"



class RaycastEngine {


private:
    static RaycastEngine* p_instance;

    RaycastEngine() {}
    RaycastEngine(const RaycastEngine&);
    RaycastEngine& operator=(RaycastEngine&);
public:
    static RaycastEngine* getInstance() {
        if (!p_instance)
            p_instance = new RaycastEngine();
        return p_instance;
    }


public:

	bool LineIntersection(vf2d ln1pnt1, vf2d ln1pnt2, vf2d ln2pnt1, vf2d ln2pnt2);


};