#pragma once
#include "GameObjectBase.h"
class ProjectileObject :
	public GameObjectBase
{
public:
	ProjectileObject(float x, float y, char sp, direction dr);
	~ProjectileObject();
};

