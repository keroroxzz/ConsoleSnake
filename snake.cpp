/*
Windows Console Snake
by Brian Tu

Release : 2020/8/18
*/

#include "stdafx.h"
#include <conio.h>
#include <iostream>
#include <vector>
#include <random>
#include <Windows.h>
#include <thread>
#include <chrono>
#include <mmsystem.h>
#include <algorithm>

#pragma "winmm.lib"

#define width 20
#define height 20
#define item_life_rate 2
#define UP 72
#define DOWN 80
#define LEFT 75
#define RIGHT 77

#define die_sound PlaySound(TEXT("bgm/GO.wav"), GetModuleHandle(NULL), SND_FILENAME | SND_SYNC)
#define music PlaySound(TEXT("bgm/music.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC)
#define music_hard PlaySound(TEXT("bgm/bgm-hard.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC)
#define music_sick PlaySound(TEXT("bgm/sick.wav"), NULL, SND_FILENAME | SND_LOOP | SND_ASYNC)
#define text_sound PlaySound(TEXT("bgm/run.wav"), GetModuleHandle(NULL), SND_FILENAME | SND_ASYNC)
#define beep_sound PlaySound(TEXT("bgm/beep.wav"), GetModuleHandle(NULL), SND_FILENAME | SND_ASYNC)
#define mute PlaySound(NULL, GetModuleHandle(NULL), SND_FILENAME | SND_ASYNC)
#define wait(t) this_thread::sleep_for(chrono::milliseconds(t))

using namespace std;

struct pos
{
	int X, Y, t;
	pos(int x, int y);

	friend bool operator ==(const pos& p1, const pos& p2);
};

int random(int, int);
int menu(char text[][50], int, int, int);
void initialize();
void play_music();
void snake_run(int);
void cleanUp();
void display(int, int);
void screen_show(int, int);
void put(const char[]);
void life_of_item();
void gold_apple_effect();
void get_key();
void time_count(int);
void control();
void text_anime( char[], const char[], int, int, int);
void time_ticker(int);
bool is_dead();
void screen_update();
void exit_game();
//void ticker(int);
int eat_item();
vector<pos> find_space();

char
menu_text[][50] = { "Easy","Norm","Hard","Very Hard","Play this if you're sick" },
menu_set[][50] = { "Resume","Restart","Music off","Music on","Exit" },
menu_YN[][50] = { "Yep","No" },
menu_text2[][50] = { "Try again","I'm out" },
text[2][4][20] = { { "hit wall, lol","loser","lol","love the wall?" },{ "you hit yourself","lol","fail","idiot" } };
char key = 'x';
int screen[width][height];
int death_type = 0;
int food_rate = 16*item_life_rate;
int food_period = 0;
int bad_appple_count = -1;
int score = 0;
int LEVEL = 0;
int accelerate_count = 0;
int background_color = 18;
vector<pos> snake;
vector<pos> food;
vector<pos> gold_apple;
vector<pos> medicine;
vector<pos> bad_apple;
pos v(0,0);
bool gaming = false;
bool is_screen_updated = false;
bool music_switch = true;
bool menuing = false;
int game_speed = 200;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);



int main()
{
	//Hide the cursor
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &info);

	thread keyT(get_key);

	text_anime("%15%Select by Up and Down, ENTER to skip or select.", "text_m", 25, 40, 15);
	text_anime("Hi, there.", "text_m", 25, 40, 15);
	text_anime("You can control the snake with arrow key.", "text_m", 25, 40, 15);
	text_anime("The snake is white.~%255%            ", "text_m", 25, 40, 15);
	text_anime("Eating food %199%  %15% gives you one point and grows the snake up.", "text_m", 25, 40, 15);
	text_anime("When the food become %70%  %15%, it's disappearing.", "text_m", 25, 40, 15);
	text_anime("%238%  %15% is a golden apple, which gives you 3 points and 3 extra food.", "text_m", 25, 40, 15);
	text_anime("When it becomes %110%  %15%, it's going to be gone.", "text_m", 25, 40, 15);
	text_anime("%215%  %15% is a pill, which shrinks the snake by 5.", "text_m", 25, 40, 15);
	text_anime("When it becomes %185%  %15%, it'll disappear.", "text_m", 25, 40, 15);
	text_anime("%87%  %15%, is the bad apple, which speeds up the game.", "text_m", 25, 40, 15);
	text_anime("The special items would appear after a while.", "text_m", 25, 40, 15);
	text_anime("Select the game mode.", "text_m", 25, 40, 15);

again:
	initialize();

	LEVEL = menu(menu_text, 5, 40, 10);

	if (LEVEL == 0) {
		game_speed = 320;
	}
	else if (LEVEL == 1) {
		game_speed = 160; 
	}
	else if (LEVEL == 2) {
		game_speed = 80;
	}
	else if (LEVEL == 3) {
		game_speed = 40;
	}
	else if (LEVEL == 4) {
		text_anime("Sick?", "text_m", 25, 40, 15);
		if (menu(menu_YN, 2, 40, 10) == 1)
		{
			text_anime("Go back and select again!", "text_m", 25, 40, 15);
			goto again;
		}
		game_speed = 150;
	}

	screen_show(20, 3);

	//wait for player's first control
	while (key != UP&&key != DOWN&&key != RIGHT&&key != LEFT) wait(20); 

	thread acc_T(time_count, 16);
	thread screen_updateT(display,20,3);
	play_music();

	while (gaming)
	{
		thread ticker(time_ticker,game_speed);

		if (key == 27)//esc menu
		{
			system("cls");
			int index = menu(menu_set, 5, 40, 10);
			if (index == 0)
			{
				play_music();
			}
			else if (index == 1)
			{
				gaming = false;
			}
			else if (index == 2)
			{
				music_switch = false;
				mute;
			}
			else if (index == 3)
			{
				music_switch = true;
				play_music();
			}
			else if (index == 4)
			{
				exit_game();
			}
		}
			
		control();

		snake_run(eat_item());

		if (is_dead())
		{
			gaming = false;
		}
		else
		{
			life_of_item();

			screen_update();
		}

		ticker.join();
	}

	gaming = false;
	die_sound;
	acc_T.join();
	screen_updateT.join();
	v.X = 0;
	v.Y = 0; 
	
	if (!is_dead())
	{
		goto again;
	}

	cout << "\n\n";

	text_anime(text[death_type - 1][random(0, 3)], "text_m", 25,40,12);
	if (LEVEL == 4 && accelerate_count <= 14) 
		text_anime("Too fast, dude.", "text_m", 25, 40, 12);

	if (menu(menu_text2, 2, 40, 10) == 0)goto again;

	exit_game();

    return 0;
}

//---------screen------------
void screen_show(int X, int Y)
{
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			wait(2);
			COORD cur = { X + y * 2, Y + x };
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
			SetConsoleTextAttribute(hConsole, screen[x][y]);
			cout << "  ";
		}
		cout << endl;
	}
	SetConsoleTextAttribute(hConsole, 15);
	COORD cur = { X , Y + width + 1 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
	cout << "Press any arrow key to start...";
}

void screen_update()
{
	cleanUp();

	pos *food_;
	int size = food.size();
	for (int i = 0; i < size; i++)
	{
		food_ = &food.at(i);
		if (food_->t>10)screen[food_->X][food_->Y] = 199;
		else screen[food_->X][food_->Y] = 70;
	}

	pos *item;
	size = gold_apple.size();
	for (int i = 0; i < size; i++)
	{
		item = &gold_apple.at(i);
		if (item->t>5)screen[item->X][item->Y] = 238;
		else screen[item->X][item->Y] = 110;
	}

	size = medicine.size();
	for (int i = 0; i < size; i++)
	{
		item = &medicine.at(i);
		if (item->t>4)screen[item->X][item->Y] = 215;
		else screen[item->X][item->Y] = 185;
	}

	size = bad_apple.size();
	for (int i = 0; i < size; i++)
	{
		item = &bad_apple.at(i);
		screen[item->X][item->Y] = 87;
	}

	pos *part_of_body;
	size = snake.size();
	for (int i = 0; i < size; i++)
	{
		part_of_body = &snake.at(i);
		screen[part_of_body->X][part_of_body->Y] = 255;
	}

	is_screen_updated = true;
}

void display(int X,int Y)
{
	COORD cur = { X,0 };

	while (gaming)
	{
		if (!menuing)
		{
			for (int x = 0; x < width; x++)
			{
				cur.Y = Y + x;
				SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);

				for (int y = 0; y < height; y++)
				{
					SetConsoleTextAttribute(hConsole, screen[x][y]);
					cout << "  ";
				}
			}

			cur.Y = Y + width + 1;
			SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
			SetConsoleTextAttribute(hConsole, 15);

			cout << "score : " << score << " length : " << snake.size() << " LEVEL : ";
			if (LEVEL == 3)cout << "FUCKING HARD";
			else if (LEVEL == 4)cout << "YOU SICK";
			else if (LEVEL == 2)cout << "HARD";
			else if (LEVEL == 1)cout << "NORMAL";
			else if (LEVEL == 0)cout << "EASY";
		}
		wait(2);
	}
}

void cleanUp()
{
	if (accelerate_count==8 && LEVEL == 4)
	{
		game_speed = 100;
		background_color = 96;
	}

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			screen[x][y] = background_color;
		}
	}
	is_screen_updated = false;
}


//---------control------------
void control()
{
	if (key == DOWN && v.X != -1)
	{
		v.X = 1;
		v.Y = 0;
		key = 'x';
	}
	else if (key == UP&& v.X != 1)
	{
		v.X = -1;
		v.Y = 0;
		key = 'x';
	}
	else if (key == RIGHT&& v.Y != -1)
	{
		v.X = 0;
		v.Y = 1;
		key = 'x';
	}
	else if (key == LEFT&& v.Y != 1)
	{
		v.X = 0;
		v.Y = -1;
		key = 'x';
	}
}

void get_key()
{
	while (1)
	{
		char key_now = _getch();
		if (key_now == DOWN && v.X != -1 || key_now == UP&& v.X != 1 || key_now == RIGHT&& v.Y != -1 || key_now == LEFT&& v.Y != 1 || key_now == 13 || key_now == 27 || menuing)
		{
			key = key_now;
		}
		wait(2);
	}
}



//---------game checking------------
int eat_item()
{
	pos head = snake.back();
	pos *item_i;
	int size = food.size();

	for (int i = 0; i < size;i++)
	{
		item_i = &food.at(i);
		if (head == *item_i)
		{
			score++;
			food.erase(food.begin()+i);
			return 1;
		}
	}

	size = gold_apple.size();
	for (int i = 0; i < size; i++)
	{
		item_i = &gold_apple.at(i);
		if (head == *item_i)
		{
			gold_apple_effect();
			gold_apple.erase(gold_apple.begin() + i);
			return 2;
		}
	}

	size = medicine.size();
	for (int i = 0; i < size; i++)
	{
		item_i = &medicine.at(i);
		if (head == *item_i)
		{
			for (int i = 0; i < 5; i++)
			{
				if(snake.size()>1)snake.erase(snake.begin());
				else break;
			}
			medicine.erase(medicine.begin() + i);
			return 2;
		}
	}

	size = bad_apple.size();
	for (int i = 0; i < size; i++)
	{
		item_i = &bad_apple.at(i);
		if (head == *item_i)
		{
			if(bad_appple_count<0)game_speed /= 2;
			bad_appple_count = 87;
			bad_apple.erase(bad_apple.begin() + i);
			return 2;
		}
	}

	return 0;
}

bool is_dead()
{
	pos head = snake.back();

	if (head.X >= width || head.X < 0 || head.Y >= height || head.Y < 0)
	{
		death_type = 1;
		return true;
	}

	int size = snake.size();
	pos *part_of_body;

	for (int i = 0; i < size-1; i++)
	{
		part_of_body = &snake.at(i);
		if (*part_of_body == snake.back())
		{
			death_type = 2;
			return true;
		}
	}
	return false;
}



//---------game operation------------
void gold_apple_effect()
{
	put("food");
	put("food");
	put("food");
	score += 3;
}

void snake_run(int eat)
{
	pos head = snake.back();
	pos new_head(head.X + v.X, head.Y + v.Y);
	snake.push_back(new_head);
	if (eat!=1)snake.erase(snake.begin());
}

void life_of_item()
{
	food_period++;
	if (food_period >= food_rate)
	{
		put("food");
		food_period = 0;

	}

	if (bad_appple_count >= 0)
	{
		bad_appple_count--;
		if (bad_appple_count < 0)
		{
			game_speed *= 2;
		}
	}

	if (snake.size() > 30)
	{
		if (random(0, 100) > 80)
		{
			put("medicine");
		}
	}

	if (score > 20)
	{
		if (random(0, 100) > 95)
		{
			put("gold apple");
		}

		if (score > 30 && LEVEL >1)
		{
			if (random(0, 100) > 103 - min(score/10, 12))
			{
				put("bad apple");
			}
		}
	}
	pos *food_i;
	int size = food.size();
	for (int i = 0; i < size; i++)
	{
		food_i = &food.at(i);
		food_i->t--;
		if (food_i->t < 0)
		{
			food.erase(food.begin() + i);
			i--;
			size--;
		}
	}

	size = gold_apple.size();
	pos *item;
	for (int i = 0; i < size; i++)
	{
		item = &gold_apple.at(i);
		item->t--;
		if (item->t < 0)
		{
			gold_apple.erase(gold_apple.begin() + i);
			i--;
			size--;
		}
	}

	size = medicine.size();
	for (int i = 0; i < size; i++)
	{
		item = &medicine.at(i);
		item->t--;
		if (item->t < 0)
		{
			medicine.erase(medicine.begin() + i);
			i--;
			size--;
		}
	}

	size = bad_apple.size();
	for (int i = 0; i < size; i++)
	{
		item = &bad_apple.at(i);
		item->t--;
		if (item->t < 0)
		{
			bad_apple.erase(bad_apple.begin() + i);
			i--;
			size--;
		}
	}
}

void put(const char item[])
{
	if (is_screen_updated)
	{
		vector<pos> vacuum=find_space();

		if (vacuum.size() > 0)
		{
			pos new_item = vacuum.at(random(0, vacuum.size() - 1));

			if (item == "food")
			{
				new_item.t = 20 * item_life_rate;
				food.push_back(new_item);
			}
			else if (item == "gold apple")
			{
				new_item.t = 10 * item_life_rate;
				gold_apple.push_back(new_item);
			}
			else if (item == "medicine")
			{
				new_item.t = 8 * item_life_rate;
				medicine.push_back(new_item);
			}
			else if (item == "bad apple")
			{
				new_item.t = 35 * item_life_rate;
				bad_apple.push_back(new_item);
			}
		}
	}
}

void time_count(int t)
{
	while (gaming)
	{
		wait(1000);
		accelerate_count++;

		if (accelerate_count >= t)break;
	}
	if (LEVEL == 4)
	{
		game_speed = 25;
		background_color = 55;
	}
}

void time_ticker(int time)
{
	wait(time);
}

void play_music()
{
	if (music_switch)
	{
		if (LEVEL == 0 || LEVEL == 1) music;
		else if (LEVEL == 2) music;
		else if (LEVEL == 3) music_hard;
		else if (LEVEL == 4) music_sick;
	}
}

void initialize()
{
	srand((unsigned)time(NULL));
	snake.clear();
	food.clear();
	gold_apple.clear();
	medicine.clear();
	bad_apple.clear();
	food_period = 0;
	score = 0;
	death_type = 0;
	bad_appple_count = -1;
	background_color = 18;
	key = 'x';
	gaming = true;
	accelerate_count = 0;
	pos first_pos(random(0, width - 1), random(0, height - 1));
	snake.push_back(first_pos);
	screen_update();
	put("food");
}

void exit_game()
{
	die_sound;
	text_anime("lol", "text_m", 25, 40, 15);
	wait(10);
	exit(0);
}



//---------text------------
void text_anime(char text[], const char mode[], int update_time,int x,int y)
{
	int length = strlen(text);
	x -= length / 2;

	COORD cur = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
	
	key = 'x';
	for (int i = 0; i<length; i++)
	{
		if (key == 13 && (mode == "text" || mode == "text_m"))
		{
			length = i;
			break;
		}
		else if (text[i] == '%')
		{
			int color = 0;
			vector<int> array;
			for (int a = i + 1; a < length; a++)
			{
				int num = text[a];
				if (num >= 48 && num <= 57)
				{
					array.push_back(num-48);
				}
				else if (text[a] == '%') 
				{
					i = a+1;
					break;
				}
			}
			reverse(array.begin(),array.end());
			for (int a = 0; a < array.size(); a++)
			{
				color += array.at(a)*pow(10,a);
			}
			SetConsoleTextAttribute(hConsole, color);
		}
		if (i % 5 == 0 && (mode == "text" || mode == "text_m") && mode != "list_m")text_sound;
		wait(update_time+5);
		cout << text[i];
	}

	SetConsoleTextAttribute(hConsole, 15);

	if (mode == "text" || mode == "text_m")
	{
		while (key != 13) { wait(20); }
		key = 'x';

		for (int i = 0; i < length; i++)
		{
			cout << "\b" << " \b";

			if (key != 13)
			{
				if (i % 5 == 0)text_sound;
				wait(update_time);
			}
			
		}
	}
	if(mode != "list_m")mute;
}

int menu(char text[][50], int num,int x,int y)
{
	int chose = 0;

	menuing = true;
	system("cls");

	for (int i = 0; i < num; i++)
	{
		text_anime(text[i], "list_m", 10, x, y + i);
		cout << endl; 
		beep_sound;
	}

	wait(100);

	while (1)
	{
		for (int i = 0; i < num; i++)
		{
			if (chose == i)
				SetConsoleTextAttribute(hConsole, 241);
			else
				SetConsoleTextAttribute(hConsole, 15);

			text_anime(text[i], "list_m", 0, x, y + i);
		}

		key = 'x';
		while (key != UP && key != DOWN && key != 13) { wait(20); }

		if (key == UP || key == DOWN)beep_sound;

		if (key == UP)chose = (chose - 1 >= 0) ? chose - 1 : 0;
		if (key == DOWN)chose = (chose + 1 < num) ? chose + 1 : num - 1;
		if (key == 13)
		{
			text_anime(text[chose], "list_m", 0, x, y + chose);
			text_sound;
			break;
		}
		key = 'x';

		wait(50);
	}

	SetConsoleTextAttribute(hConsole, 15);

	int length = 0;
	for (int y_ = 0; y_ < num; y_++)
	{
		if (y_ == chose)continue;
		length = strlen(text[y_]);
		COORD cur = { x  - length / 2, y + y_ };
		SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
		for (int x_ = 0; x_ < length; x_++)
		{
			cout << " ";
			wait(10);
		}
	}

	wait(150);
	length = strlen(text[chose]);
	COORD cur = { x + length / 2, y + chose };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cur);
	for (int x_ = 0; x_ < length; x_++)
	{
		cout << "\b \b";
		wait(10);
	}

	menuing = false;
	return chose;
}



//---------algorithm------------
int random(int min, int max)
{
	return min + (max - min + 1)*rand() / (RAND_MAX + 1);
}

vector<pos> find_space()
{
	vector<pos> space;

	if (is_screen_updated)
		for (int x = 0; x < width; x++)
			for (int y = 0; y < width; y++)
				if (screen[x][y] == background_color)
				{
					pos spaace_pos(x, y);
					space.push_back(spaace_pos);
				}

	return space;
}

pos::pos(int x,int y)
{
	X = x;
	Y = y;
}

bool operator ==(const pos& p1, const pos& p2)
{
	return p1.X == p2.X&&p1.Y == p2.Y;
}
