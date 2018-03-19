#pragma once
#include "GameObjectBase.h"
class TankObject :
	public GameObjectBase
{
public:
	TankObject(char spr, float x, float y, int v, direction dr, short cl);
	~TankObject();
};

