#include "GameObjectBase.h"



GameObjectBase::GameObjectBase(float x, float y, int v, char sp, direction dr,short cl) : xPos_(x), yPos_(y), velocity_(v), sprite_(sp), dir_(dr), color_(cl)
{
}


GameObjectBase::~GameObjectBase()
{
}


float GameObjectBase::getXPos()
{
	return xPos_;
}

float GameObjectBase::getYPos()
{
	return yPos_;
}

int GameObjectBase::getVelocity()
{
	return velocity_;
}

direction GameObjectBase::GetDirection()
{
	return dir_;
}

char GameObjectBase::getSprite()
{
	return sprite_;
}


void GameObjectBase::setDirection(direction dr)
{
	dir_ = dr;
}

void GameObjectBase::setXPos(float x)
{
	xPos_ = x;
}

void GameObjectBase::setYPos(float y)
{
	yPos_ = y;
}

void GameObjectBase::setSprite(char s)
{
	sprite_ = s;
}

short GameObjectBase::getColor()
{
	return color_;
}
