#include <stdio.h>
#include <Windows.h>
#include <conio.h>
#include <time.h>

#define U64Max 0xffffffffffffffffULL

typedef unsigned char u8;
typedef unsigned short int ui16;
typedef unsigned int ui32;
typedef unsigned long long int ui64;

typedef struct { int x; int y; } Point;
typedef char bool;

HANDLE STDOUT;
char Running;

int main() {
GameStart:
	//设置全局变量
	{
		STDOUT = GetStdHandle(STD_OUTPUT_HANDLE);
		Running = 1;
		srand(time(0));
	}

	//隐藏光标
	{
		CONSOLE_CURSOR_INFO info;
		info.bVisible = FALSE;
		info.dwSize = sizeof(info);
		SetConsoleCursorInfo(STDOUT, &info);
	}

	//设置玩家和敌人的图案
	const char *player_pattern = "  *  \n *** \n *** ";
	const char *enermy_pattern = " \n*\n ";

	//设置局部变量
	char player_move = 1;						//指示玩家是否在移动(1->右移, -1->左移, 0->不移动)
	bool player_shoot = 0;						//指示玩家是否在射击
	bool enermy_add = 0;						//指示敌人是否增援
	bool enermy_move = 0;						//指示敌人是否在向下移动
	bool player_powerful_attack = 0;			//玩家攻击是否可以穿透敌人(1->是, 0->否)

	Point player_position = { 0,20 };			//这里储存玩家的位置
	Point *enermy_positions = (Point*)malloc(64 * sizeof(int) * 2);			//这里储存敌人的位置
	ui64 free_positions = U64Max;											//这里动态分配敌人
	unsigned int counter = 512, score = 0;									//计数器（时间和分数）
	//准备好开始游戏
	{
		printf("Space Invader\n\t\t--by Sango\n\n按a,d键左右移动,按空格键开火(注意别开大写锁定)\n按任意键开始游戏");
		char c = '\0';
		while (c == '\0') {
			c = _getch();
		}

		//按下s键开始游戏后，玩家火力可以穿透敌人
		if (c == 's') {
			player_powerful_attack = 1;
		}
	}
	system("cls");

	COORD debug_position0 = { (SHORT)60, (SHORT)0 };
	SetConsoleCursorPosition(STDOUT, debug_position0);
	printf("按下l键退出游戏");

GameRunning:
	//游戏主体循环
	while (Running != 0) {
		//优先处理玩家的键盘输入
		if (_kbhit()) {
			char c = _getch();
			switch (c) {

				//处理玩家移动
			case 'a':
				player_move = -1;
				break;
			case 'd':
				player_move = 1;
				break;
				//处理玩家射击
			case ' ':
				player_shoot = 1;
				break;
				//处理玩家强制退出
			case 'l':
				Running = 0;
				break;
			}
		}
		
		//绘制敌人
		if (enermy_positions != 0) {
			for (ui16 i = 0; i < 64; i++) {
				//找到所有正在占用的位置
				if (((free_positions >> i) & 0b1ULL) == 0) {
					Point enermy_position = enermy_positions[i];

					//在侧栏显示一些信息
					if (i < 16) {
						COORD debug_position = { (SHORT)60, (SHORT)(i + 6) };
						SetConsoleCursorPosition(STDOUT, debug_position);
						printf("(%2d, %2d)", enermy_positions[i].x, enermy_positions[i].y);
					}
					else if (i < 32) {
						COORD debug_position = { (SHORT)80, (SHORT)(i - 10) };
						SetConsoleCursorPosition(STDOUT, debug_position);
						printf("(%2d, %2d)", enermy_positions[i].x, enermy_positions[i].y);
					}
					else if (i < 48) {
						COORD debug_position = { (SHORT)80, (SHORT)(i - 26) };
						SetConsoleCursorPosition(STDOUT, debug_position);
						printf("(%2d, %2d)", enermy_positions[i].x, enermy_positions[i].y);
					}
					else if (i < 64) {
						COORD debug_position = { (SHORT)80, (SHORT)(i - 42) };
						SetConsoleCursorPosition(STDOUT, debug_position);
						printf("(%2d, %2d)", enermy_positions[i].x, enermy_positions[i].y);
					}

					//将光标移动到敌人位置
					COORD position = { (SHORT)enermy_position.x, (SHORT)enermy_position.y };
					SetConsoleCursorPosition(STDOUT, position);

					//正式开始绘制
					for (ui16 i = 0; i < 0xff; i++) {
						const char c = enermy_pattern[i];
						char next_line = 0;
						switch (c) {
						case '\n':
							next_line = 1;
							break;
						case '\0':
							i = 0xff;
							break;
						default:
							printf("%c", c);
							break;
						}

						if (next_line) {
							COORD position = { (SHORT)enermy_position.x, (SHORT)enermy_position.y + i };
							SetConsoleCursorPosition(STDOUT, position);
						}
					}
				}
			}
		}

		//绘制玩家
		//Draw When Moved
		if (player_move != 0) {
			//Calculate Position
			player_position.x += player_move;
			if (player_position.x < 0) {
				player_position.x = 0;
			}
			else if (player_position.x > 40) {
				player_position.x = 40;
			}
			player_move = 0;

			//Move Cursor To Player's Position
			COORD position = { (SHORT)player_position.x, (SHORT)player_position.y };
			SetConsoleCursorPosition(STDOUT, position);

			//Draw
			for (ui16 i = 0; i < 0xff; i++) {
				char c = player_pattern[i];

				if (c == '\n') {
					COORD position = { (SHORT)player_position.x, (SHORT)(player_position.y + i / 5) };
					SetConsoleCursorPosition(STDOUT, position);
				}
				else if (c == '\0') {
					i = 0xff;
				}
				else {
					printf("%c", c);
				}
			}
		}

		//Draw When Shoot
		if (player_shoot != 0) {
			for (ui16 i = player_position.y - 1; i > 0; i--) {
				COORD position = { player_position.x + 2, i };
				SetConsoleCursorPosition(STDOUT, position);
				printf("^");
			}

			Sleep(20);

			for (ui16 i = player_position.y - 1; i > 0; i--) {
				COORD position = { (SHORT)(player_position.x + 2), (SHORT)i };
				SetConsoleCursorPosition(STDOUT, position);
				printf(" ");
			}
		}

		//检查Timing
		{

			if (--counter == 1)
				counter = 2048;

			if (counter % (512) == 0) {
				enermy_move = 1;
			}

			if (counter % (2000) == 0) {
				enermy_add = 1;
			}

			//在侧栏显示一些信息
			COORD debug_position = { (SHORT)60, (SHORT)1 };
			SetConsoleCursorPosition(STDOUT, debug_position);
			printf("counter : %4d", counter);
			COORD debug_position1 = { (SHORT)60, (SHORT)2 };
			SetConsoleCursorPosition(STDOUT, debug_position1);
			printf("enermy_move : %d", enermy_move);
			COORD debug_position2 = { (SHORT)60, (SHORT)3 };
			SetConsoleCursorPosition(STDOUT, debug_position2);
			printf("enermy_add : %d", enermy_add);
			COORD debug_position3 = { (SHORT)60, (SHORT)4 };
			SetConsoleCursorPosition(STDOUT, debug_position3);
			printf("score : %d", score);
			COORD debug_position4 = { (SHORT)60, (SHORT)5 };
			SetConsoleCursorPosition(STDOUT, debug_position4);
			printf("player_powerful_attack : %d", player_powerful_attack);
		}

		//处理敌人对于玩家行为的反应，或者敌人自己该做什么
		if (enermy_positions != 0) {
			//添加敌人
			if (enermy_add != 0) {
				//Add 8 Enermies
				int counter = 8;
				//Find Free Position;
				for (ui16 i = 0; counter > 0 && i < 64; i++) {
					if (((free_positions >> i) & 0b1ULL) == 1) {
						//Set Position
						free_positions &= ~(0b1ULL << i);
						
						//Set Location
						int r = rand() % 3;
						Point p = {counter * 4 + ((rand() % 2) ? -r:r) ,0};
						enermy_positions[i] = p;

						counter--;
					}
				}

				enermy_add = 0;
			}

			//击杀敌人
			if (player_shoot != 0) {
				//找到敌人正在使用的位置
				for (ui16 i = 0; i < 64; i++) {
					if (((free_positions >> i) & 0b1ULL) == 0) {
						//if it got shoot
						if (enermy_positions[i].x == player_position.x + 2) {
							//free its position to remove it
							free_positions |= 0b1ULL << i;
							score++;

							if (player_powerful_attack == 0) {
								break;
							}
						}
					}
				}

				player_shoot = 0;
			}

			//敌人前进
			if (enermy_move != 0) {
				//找到敌人正在使用的位置
				for (ui16 i = 0; i < 64; i++) {
					if (((free_positions >> i) & 0b1ULL) == 0) {
						//添加一个
						enermy_positions[i].y += 1;

						if (enermy_positions[i].y >= 20) {
							Running = 0;
							break;
						}
					}
				}

				enermy_move = 0;
			}
		}
	}

GameOver:
	{
		//清理屏幕
		system("cls");
		COORD position1 = { (SHORT)5, (SHORT)10 };
		SetConsoleCursorPosition(STDOUT, position1);
		printf("G a m e     O v e r\n\n\n\n");
		COORD position2 = { (SHORT)10, (SHORT)12 };
		SetConsoleCursorPosition(STDOUT, position2);
		printf("Your Score:%d\n\n\n\n", score);

		printf("按下r键重新开始，按其他键退出游戏\n");
		if (_getch() == 'r') {
			system("cls");
			goto GameStart;
		}
	}
}