#pragma once
enum direction { LEFT= 27, RIGHT = 26, UP = 24, DOWN = 25
};
class GameObjectBase
{
	float xPos_;
	float yPos_;
	int velocity_;
	char sprite_;
	short color_;
	direction dir_;
public:
	GameObjectBase(float x, float y, int v, char sp, direction dr, short color);
	~GameObjectBase();
	float getXPos();
	float getYPos();
	char getSprite();
	int getVelocity();
	direction GetDirection();

	void setDirection(direction dr);
	void setXPos(float x);
	void setYPos(float y);
	void setSprite(char s);
	short getColor();
};

