#include "Game.h"


int main()
{
	Game game;
	if (game.Start())
	{
		system("cls");
		cout << "Victory!" << endl;
	}
	else
	{
		system("cls");
		cout << "Loss!" << endl;
	}
			
	return 0;
}
