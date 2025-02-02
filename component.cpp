#include "component.h"
#include <iostream>
#include <vector>

using namespace std;

Component::Component()
{
    // ulOrig and ulNew use Location's default constructor to
    //  already initialize to -1,-1

    label = -1;
    height = 0;
    width = 0;
}

Component::Component(Location origUL, int h, int w, int mylabel)
{
    ulOrig = origUL;
    ulNew = origUL;
    label = mylabel;
    height = h;
    width = w;
}

Component::~Component()
{
}


