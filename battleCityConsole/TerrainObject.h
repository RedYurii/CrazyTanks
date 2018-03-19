#pragma once
#include "GameObjectBase.h"
class TerrainObject :
	public GameObjectBase
{
public:
	TerrainObject(float x, float y, char sp);
	~TerrainObject();
};

