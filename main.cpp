#include <iostream>
#include <cstdint>

// Windows
#include <windows.h>
#include <conio.h>

// void flushInput() {
// 	for(;;) { const int c = getchar(); if(c=='\n' || c==EOF) break; }
// }

uint16_t WIDTH = 28;
uint16_t HEIGHT = 31;

const char *const LEVEL_INIT[] = {
	"WWWWWWWWWWWWWWWWWWWWWWWWWWWW",
	"W............WW............W",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"WoWWWW.WWWWW.WW.WWWWW.WWWWoW",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"W..........................W",
	"W.WWWW.WW.WWWWWWWW.WW.WWWW.W",
	"W.WWWW.WW.WWWWWWWW.WW.WWWW.W",
	"W......WW....WW....WW......W",
	"WWWWWW.WWWWW WW WWWWW.WWWWWW",
	"WWWWWW.WWWWW WW WWWWW.WWWWWW",
	"WWWWWW.WW          WW.WWWWWW",
	"WWWWWW.WW WWW  WWW WW.WWWWWW",
	"WWWWWW.WW W      W WW.WWWWWW",
	"      .   W      W   .      ",
	"WWWWWW.WW W      W WW.WWWWWW",
	"WWWWWW.WW WWWWWWWW WW.WWWWWW",
	"WWWWWW.WW          WW.WWWWWW",
	"WWWWWW.WW WWWWWWWW WW.WWWWWW",
	"WWWWWW.WW WWWWWWWW WW.WWWWWW",
	"W............WW............W",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"W.WWWW.WWWWW.WW.WWWWW.WWWW.W",
	"Wo..WW................WW..oW",
	"WWW.WW.WW.WWWWWWWW.WW.WW.WWW",
	"WWW.WW.WW.WWWWWWWW.WW.WW.WWW",
	"W......WW....WW....WW......W",
	"W.WWWWWWWWWW.WW.WWWWWWWWWW.W",
	"W.WWWWWWWWWW.WW.WWWWWWWWWW.W",
	"W..........................W",
	"WWWWWWWWWWWWWWWWWWWWWWWWWWWW"};

// Spielstatus:
int score = 0;

int width = 0, height = 0;
bool *walls = nullptr; // true = Wand
bool *coins = nullptr; // true = Muenze

int pacPosX, pacPosY;
int pacDirX, pacDirY;
// /Spielstatus

void loadMap() {
	pacPosX = 13;
	pacPosY = 23;

	pacDirX = 0;
	pacDirY = 0;

	width = WIDTH;
	height = HEIGHT;
	walls = new bool[width * height]{};
	coins = new bool[width * height]{};

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			char stelle = LEVEL_INIT[y][x];
			if (stelle=='W')
				walls[y * width + x] = true;
			if (stelle=='.')
				coins[y * width + x] = true;
		}
	}
}

int frame = 0;

void printMap() {
	frame++;
	printf("\x1B[?25l"); // Disable Cursor
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
	printf("\x1B[2J"); // Erase Entire Viewport

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(x == pacPosX && y == pacPosY) { // Pac Man
				// printf("\x1B[33m" "C " "\x1B[0m");
				printf("\x1B[33m"); // Gelb
				if(frame%2 == 0) {
					printf("O ");
				} else {
					if(pacDirX == 1)
						printf("< ");
					if(pacDirX == -1)
						printf("> ");
					if(pacDirY == 1)
						printf("^ ");
					if(pacDirY == -1)
						printf("V ");
				}
				printf("\x1B[0m");
			} else if(walls[y * width + x]) {
				printf("\x1B[32m" "WW" "\x1B[0m"); // Wand
			} else if(coins[y * width + x]) {
				printf("\x1B[33m"); // Gelb
				printf(". "); // Coin
				printf("\x1B[0m");
			} else {
				printf("  "); // Leer
			}
		}
		printf("\n");
	}

	printf("\nScore: %d\n", score);
}

void update() {
	if(_kbhit()) { // Wenn Taste gedrueckt
		char input = _getch(); // Die gedrückte Taste
		if(input == 'w') {
			pacDirX = 0;
			pacDirY = -1;
		} else if(input == 's') {
			pacDirX = 0;
			pacDirY = 1;
		} else if(input == 'a') {
			pacDirX = -1;
			pacDirY = 0;
		} else if(input == 'd') {
			pacDirX = 1;
			pacDirY = 0;
		}
	}

	int nextX = pacPosX + pacDirX;
	int nextY = pacPosY + pacDirY;

	if(nextX == -1)
		nextX += width;
	if(nextX == width)
		nextX -= width;

	if(walls[nextY * width + nextX] == false) { // Wenn an Zielposition keine Wand
		pacPosX = nextX;
		pacPosY = nextY;

		if(coins[pacPosY * width + pacPosX]) {
			coins[pacPosY * width + pacPosX] = false; // Muenze einsammeln
			score++;
		}
	}
}

int main(int argc, char** argv) {
	loadMap();

	for(;;) {
		update();
		printMap();
		Sleep(100);
	}

	printf("\x1B[31m" "Hello World!\n" "\x1B[0m");
	return 0;
}