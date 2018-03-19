#include "Game.h"

Game::Game()
{	
	fieldWidth_ = 40; //60
	fieldHeight_ = 20; //40
	isGameActive_ = false;
	gameTime_ = 0;
	enemiesKilled_ = 0;
	tickTime_ = 0;
	direction dr = UP;
	hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
	int size = fieldWidth_*fieldHeight_;
	charBuffer_ = new CHAR_INFO[size];
	memset(charBuffer_, 0, sizeof(CHAR_INFO) * size);
	
}

Game::~Game()
{
	delete playerObject_;
	for (unsigned int i=0; i< projectilesObjects_.size(); ++i)
		delete projectilesObjects_[i];
	for (unsigned int i = 0; i< enemiesObjects_.size(); ++i)
		delete enemiesObjects_[i];
	for (unsigned int i = 0; i< terrainObjects_.size(); ++i)
		delete terrainObjects_[i];

	CloseHandle(hConsole_);
	delete[] charBuffer_;
}


bool Game::Start()
{
	isGameActive_ = true;

	// Start the thread
	thread t = thread(&Game::GameThread, this);

	// Wait for thread to be exited
	t.join();

	//game win/lose condition return
	if (playersHP_ == 0)
		return false;

	return true;
}


bool Game::OnCreate()
{	
	//random number generator init
	random_device rd;     
	mt19937 rng(rd());   
	tickCount_ = 3;
	shotTickCount_ = 10;
	// constructing game field
	CreateGameField();

	//create player  
	playerObject_ = new TankObject(UP, (fieldWidth_-2)/2, fieldHeight_-2, 200, UP, 2);
	reloadTime_ = 0.01f;
	isPCProjActive_ = false;
	playersHP_ = 3;

	//create walls
	int wallCount = 15; //half vertical, other half horizontal
	for (int i = 0; i < wallCount; ++i)
	{
		//generate length
		uniform_int_distribution<int> uniLength(2, 6); 
		int len = uniLength(rng);

		//generate startpos
		uniform_int_distribution<int> uniX(2, fieldWidth_ - 4); 
		uniform_int_distribution<int> uniY(2, fieldHeight_ - 4); 
		int xPos = uniX(rng);
		int yPos = uniY(rng);

		for (int j = 0; j < len; ++j)
		{				
			if ((xPos == (fieldWidth_ - 2) / 2) &&  (yPos == fieldHeight_ - 2))
			{
				continue;
			}
			GameObjectBase* wall = new TerrainObject(xPos, yPos, '#');
			terrainObjects_.push_back(wall);
			if (i<wallCount/2) //vertical
				++yPos;
			else //horizontal
				++xPos;
		}	
	}
	//draw walls (write to screen buffer, so while tank objets are created collision checks could be made
	short col = 0x000F;
	for (auto i : terrainObjects_)
		Draw(i->getXPos(), i->getYPos(), i->getSprite(), col);
	WriteConsoleOutput(hConsole_, charBuffer_, { (short)fieldWidth_, (short)fieldHeight_ + 3 }, { 0,0 }, &rectWindow_);

	//create enemies 
	unsigned int enemiesCount = 8;
	int spacing = 3;
	while(enemiesObjects_.size() < enemiesCount)
	{
		uniform_int_distribution<int> uniX(1, fieldWidth_-2); 
		uniform_int_distribution<int> uniY(1, fieldHeight_-2);

		float xPos = uniX(rng);
		float yPos = uniY(rng);

		bool hasEmptyArea = true;
		for (auto i : enemiesObjects_)
		{
			if (abs((i->getXPos() - xPos)) < spacing && abs((i->getYPos() - yPos)) < spacing)
			{
				hasEmptyArea = false;
				break;
			}
		}
		
		if (CheckCollision(xPos, yPos) == 0 && hasEmptyArea)
		{		
			//create enm tanks
			GameObjectBase* nme = new TankObject( UP, xPos, yPos, 150, UP, short(3 + enemiesObjects_.size()));
			enemiesObjects_.push_back(nme);
		}
	}
	return true;
}


int Game::CheckCollision(float posX, float posY) 
{
	//compare char in new pos with char that is in this cell
	WCHAR cToComp = charBuffer_[(int)posY * fieldWidth_ + (int)posX].Char.UnicodeChar;
	if (cToComp == '#') //wall collision
	{
		return 1;
	}
	else if (cToComp == char(UP) || cToComp == char(DOWN) || cToComp == char(RIGHT) || cToComp == char(LEFT)) //tank collision
	{
		return 2;
	}
	return 0;
}


bool Game::OnUpdate(float elapsedTime)
{
	gameTime_ += ceil(elapsedTime);
	tickTime_ += elapsedTime;

	float fps = 35;//fps lock
	
	if (tickTime_ > 1/fps)
	{
		tickTime_ = 0;

		//update game info
		wchar_t s[256];
		swprintf_s(s, 256, L"Crazy tanks - Time: %d - HP: %d - Kills: %d", gameTime_ / 1000, playersHP_, enemiesKilled_);
		SetConsoleTitle(s);
		
	
		//reloading PC's gun 
		if (isPCProjActive_)
		{
			reloadTime_ -= elapsedTime;
			if (reloadTime_ <= 0)
			{
				reloadTime_ = 0.01f;
				isPCProjActive_ = false;
			}
		}
			 	
		//fill the game field with spaces
		short col = 0x000F;
		Fill(1, 1, fieldWidth_-1, fieldHeight_-1,L' ', 0);

		//draw walls
		for (auto i : terrainObjects_)
			Draw(i->getXPos(), i->getYPos(), i->getSprite(), col);

		//draw player
		Draw(playerObject_->getXPos(), playerObject_->getYPos(), playerObject_->getSprite(), playerObject_->getColor());

		//draw enemies
		for (auto i : enemiesObjects_)	
			Draw(i->getXPos(), i->getYPos(), i->getSprite(), i->getColor());

		//draw shells
		for (auto i : projectilesObjects_)
			Draw(i->getXPos(), i->getYPos(), i->getSprite(), 0x000F);

		// enemy tanks act only every 3 ticks
		if (tickCount_ > 0)
			tickCount_--;
		else
		{
			tickCount_ = 3;
			for (auto i : enemiesObjects_)
			{
				float newPosX = i->getXPos();
				float newPosY = i->getYPos();

				//generate random direction
				int drar[] = { 24, 25, 26, 27 };
				direction dr = static_cast<direction>(drar[rand() % 4]);
				i->setSprite(dr);
				i->setDirection(dr);

				switch (i->GetDirection())
				{
				case LEFT:
					newPosX = i->getXPos() - i->getVelocity()* elapsedTime;
					break;
				case RIGHT:
					newPosX = i->getXPos() + i->getVelocity()* elapsedTime;
					break;
				case UP:
					newPosY = i->getYPos() - i->getVelocity()* elapsedTime;
					break;
				case DOWN:
					newPosY = i->getYPos() + i->getVelocity()* elapsedTime;
					break;
				}
				if (CheckCollision(newPosX, newPosY) == 0
					|| (CheckCollision(newPosX, newPosY) == 2
					&& charBuffer_[(int)newPosY * fieldWidth_ + (int)newPosX].Attributes == i->getColor()
					&& charBuffer_[(int)newPosY * fieldWidth_ + (int)newPosX].Attributes != playerObject_->getColor()))
				{
					i->setXPos(newPosX);
					i->setYPos(newPosY);
				}
			}
		}
		
		//make a shot with random enemy tank every 5 ticks
		if (shotTickCount_> 0)
			shotTickCount_--;
		else
		{
			shotTickCount_ = 5;
			int randEnemyIdx = rand() % enemiesObjects_.size();

			float x = enemiesObjects_[randEnemyIdx]->getXPos();
			float y = enemiesObjects_[randEnemyIdx]->getYPos();
			GameObjectBase* sh = new ProjectileObject(x, y, '*', enemiesObjects_[randEnemyIdx]->GetDirection());
			projectilesObjects_.push_back(sh);
		}

		//process every projectile
		auto projIt = begin(projectilesObjects_);
		while (projIt != end(projectilesObjects_)) 
		{
			float newPosX = (*projIt)->getXPos();
			float newPosY = (*projIt)->getYPos();
			switch ((*projIt)->GetDirection())
			{
			case LEFT:
				newPosX = ((*projIt)->getXPos() - (*projIt)->getVelocity()* elapsedTime);
				break;
			case RIGHT:
				newPosX = ((*projIt)->getXPos() + (*projIt)->getVelocity()* elapsedTime);
				break;
			case UP:
				newPosY = ((*projIt)->getYPos() - (*projIt)->getVelocity()* elapsedTime);
				break;
			case DOWN:
				newPosY = ((*projIt)->getYPos() + (*projIt)->getVelocity()* elapsedTime);
				break;
			}
	
			//collision 
			int collisionCheck = CheckCollision(newPosX, newPosY);
			if (collisionCheck == 1)
			{
				//wall collision
				//remove projectile	
				projIt = projectilesObjects_.erase(projIt);
				continue;	
			}
			else if (collisionCheck == 2)
			{
				bool isPlayerHit = false;
				char projectSprite = (*projIt)->getSprite();
				if (((int)playerObject_->getXPos() == (int)newPosX && (int)playerObject_->getYPos() == (int)newPosY) && (projectSprite == '*'))
				{
					//player gets hit
					playersHP_--;
					if(playersHP_ == 0)
						isGameActive_ = false;
				}
					

				if (projectSprite == 'o')
				{
					auto nmeIt = begin(enemiesObjects_);
					while (nmeIt != end(enemiesObjects_))
					{
						if ((int)(*nmeIt)->getXPos() == (int)newPosX && (int)(*nmeIt)->getYPos() == (int)newPosY && (projectSprite == 'o'))
						{
							//destroy enemy
							projIt = projectilesObjects_.erase(projIt);
							nmeIt = enemiesObjects_.erase(nmeIt);
							enemiesKilled_++;
							if (enemiesObjects_.size() == 0)
								isGameActive_ = false;
							continue;
						}
						if (nmeIt != end(enemiesObjects_))
							nmeIt++;
					}
				}
				else
				{
					projIt = projectilesObjects_.erase(projIt);
				}
			}
			else
			{
				(*projIt)->setXPos(newPosX);
				(*projIt)->setYPos(newPosY);
			}
			if(projIt != end(projectilesObjects_))
				++projIt;
		}
		
		// Process player's input
		UpdateKeyboardInput();

		float newPosPCX = playerObject_->getXPos();
		float newPosPCY = playerObject_->getYPos();
		if (m_keys[VK_LEFT].bHeld) //bHeld
		{
			playerObject_->setDirection(LEFT);
			playerObject_->setSprite(LEFT);
			newPosPCX = (playerObject_->getXPos() - playerObject_->getVelocity()* elapsedTime) ;
		}
		if (m_keys[VK_RIGHT].bHeld)
		{
			playerObject_->setDirection(RIGHT);
			playerObject_->setSprite(RIGHT);
			newPosPCX = (playerObject_->getXPos() + playerObject_->getVelocity()* elapsedTime);
		}
		if (m_keys[VK_UP].bHeld)
		{	
			playerObject_->setDirection(UP); 
			playerObject_->setSprite(UP);
			newPosPCY = (playerObject_->getYPos() - playerObject_->getVelocity()* elapsedTime) ;
		}
		if (m_keys[VK_DOWN].bHeld)
		{
			playerObject_->setDirection(DOWN);
			playerObject_->setSprite(DOWN);
			newPosPCY = (playerObject_->getYPos() + playerObject_->getVelocity()* elapsedTime);
		}
		//check player's collision
		if (CheckCollision(newPosPCX, newPosPCY) == 0 || (CheckCollision(newPosPCX, newPosPCY) == 2
			&& charBuffer_[(int)newPosPCY * fieldWidth_ + (int)newPosPCX].Attributes == 2))
		{
			playerObject_->setXPos(newPosPCX);
			playerObject_->setYPos(newPosPCY);
		}

		//fire players projectile
		if (m_keys[VK_SPACE].bReleased && !isPCProjActive_)
		{
			float x = playerObject_->getXPos();
			float y = playerObject_->getYPos();
			GameObjectBase* sh = new ProjectileObject(x, y, 'o', playerObject_->GetDirection());
			projectilesObjects_.push_back(sh);
			isPCProjActive_ = true;
		}
		WriteConsoleOutput(hConsole_, charBuffer_, { (short)fieldWidth_, (short)fieldHeight_ + 3 }, { 0,0 }, &rectWindow_);
	}

	return true;
}

bool Game::OnDestroy()
{
	return false;
}

void Game::UpdateKeyboardInput()
{
	// Handle Keyboard Input
	for (int i = 0; i < 256; i++)
	{
		m_keyNewState_[i] = GetAsyncKeyState(i);

		m_keys[i].bPressed = false;
		m_keys[i].bReleased = false;

		if (m_keyNewState_[i] != m_keyOldState_[i])
		{
			if (m_keyNewState_[i] & 0x8000)
			{
				m_keys[i].bPressed = !m_keys[i].bHeld;
				m_keys[i].bHeld = true;
			}
			else
			{
				m_keys[i].bReleased = true;
				m_keys[i].bHeld = false;
			}
		}
		m_keyOldState_[i] = m_keyNewState_[i];
	}
	
}

void Game::GameThread()
{
	if (!OnCreate())
		isGameActive_ = false;

	auto tp1 = chrono::system_clock::now();
	auto tp2 = chrono::system_clock::now();

	while (isGameActive_)
	{
		// Handle Timing
		tp2 = chrono::system_clock::now();
		chrono::duration<float> elapsedTime = tp2 - tp1;
		tp1 = tp2;
		float fElapsedTime = elapsedTime.count();
				
		if(!OnUpdate(fElapsedTime))
			isGameActive_ = false;
	}
}


void Game::Draw(int x, int y, wchar_t c = 0x2588, short col = 0x000F)
{
	if (x >= 0 && x < fieldWidth_-1 && y >= 0 && y < fieldHeight_-1)
	{
		charBuffer_[y * fieldWidth_ + x].Char.UnicodeChar = c;
		charBuffer_[y * fieldWidth_ + x].Attributes = col;
	}
}

void Game::Fill(int x1, int y1, int x2, int y2, wchar_t c = 0x2588, short col = 0x000F)
{
	Clip(x1, y1);
	Clip(x2, y2);
	for (int x = x1; x < x2; x++)
		for (int y = y1; y < y2; y++)
			Draw(x, y, c, col);
}

void Game::Clip(int &x, int &y)
{
	if (x < 0) x = 0;
	if (x >= fieldWidth_) x = fieldWidth_;
	if (y < 0) y = 0;
	if (y >= fieldHeight_) y = fieldHeight_;
}


bool Game::CreateGameField()
{
	rectWindow_ = { 0,0,  (SHORT)fieldWidth_ - 1  , (SHORT)fieldHeight_ -1};
	SetConsoleWindowInfo(hConsole_, TRUE, &rectWindow_);
	SetConsoleActiveScreenBuffer(hConsole_);

	//create border ( # - walls)
	wchar_t c = '#';
	short col = 0x000F;

	for (int i = 0; i < fieldHeight_; ++i)
	{
		for (int j = 0; j < fieldWidth_; ++j)
		{
			if (i == 0 || i == fieldHeight_ - 1 || j == 0 || j == fieldWidth_ - 1)
				charBuffer_[i*fieldWidth_ + j].Char.UnicodeChar = c;
			charBuffer_[i*fieldWidth_ + j].Attributes = col;
		}
	}
	return true;
}