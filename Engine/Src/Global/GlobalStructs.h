#pragma once
#include "../Core.h"

struct SetUpTexsParams final{
    str texPath;
    bool flipTex;
    int texTarget;
    int texWrapParam;
    int texFilterMin;
    int texFilterMag;
};