#pragma once
#include <thread>
#include <vector>
#include <windows.h>
#include <iostream>
#include <random>
#include <math.h>
#include "TankObject.h"
#include "TerrainObject.h"
#include "ProjectileObject.h"

using namespace std;


class Game
{
	GameObjectBase* playerObject_;
	vector<GameObjectBase*> projectilesObjects_;
	vector<GameObjectBase*> enemiesObjects_;
	vector<GameObjectBase*> terrainObjects_;
	CHAR_INFO*  charBuffer_;
	bool isGameActive_;
	int gameTime_;
	short m_keyOldState_[256] = { 0 };
	short m_keyNewState_[256] = { 0 };
	int fieldWidth_;
	int fieldHeight_;
	float tickTime_;
	bool isPCProjActive_;
	SMALL_RECT rectWindow_;
	float reloadTime_;
	HANDLE hConsole_;
	int tickCount_;
	int shotTickCount_;
	int playersHP_;
	int enemiesKilled_;
public:
	Game();
	~Game();
	bool CreateGameField();
	bool Start();
	bool OnCreate();
	int CheckCollision(float  posX, float  posY);
	bool OnUpdate(float elapsedTime);
	bool OnDestroy();
	void GameThread();
	void Draw(int x, int y, wchar_t c, short col);
	void Fill(int x1, int y1, int x2, int y2, wchar_t c, short col);
	void Clip(int & x, int & y);
	void UpdateKeyboardInput();
private:
	struct sKeyState
	{
		bool bPressed;
		bool bReleased;
		bool bHeld;
	} m_keys[256];
};