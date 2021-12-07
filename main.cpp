#include <iostream>
#include <cstdint>

#include <vector>

#include "platform.h"

// void flushInput() {
// 	for(;;) { const int c = getchar(); if(c=='\n' || c==EOF) break; }
// }

struct Ghost {
	int x, y; // position des Geistes
	int dirX, dirY; // richtung
};


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

const int NUM_GHOSTS = 1;
Ghost ghosts[NUM_GHOSTS]; // blinky
// /Spielstatus

void loadMap() {
	pacPosX = 13;
	pacPosY = 23;

	pacDirX = 0;
	pacDirY = -1;

	ghosts[0].x = 13;
	ghosts[0].y = 11;
	ghosts[0].dirX = 0;
	ghosts[0].dirY = 0;


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
	// printf("\x1B[2J"); // Erase Entire Viewport

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(x == pacPosX && y == pacPosY) { // Pac Man
				// printf("\x1B[33m" "C " "\x1B[0m");
				printf("\x1B[33m"); // Gelb
				if(frame%2 == 0) {
					printf("o ");
				} else {
					if(pacDirX == 1)
						printf("< ");
					else if(pacDirX == -1)
						printf("> ");
					else if(pacDirY == 1)
						printf("^ ");
					else if(pacDirY == -1)
						printf("V ");
				}
				printf("\x1B[0m");
			} else if(x == ghosts[0].x && y == ghosts[0].y) {
				printf("\x1B[31m"); // Rot
				printf("U "); // Geist
				printf("\x1B[0m");
			} else if(walls[y * width + x]) {
				// printf("\x1B[34m" "WW" "\x1B[0m"); // Wand
				printf("\x1B[36m" "WW" "\x1B[0m"); // Wand
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

	printf("\x1B[35m" "\nScore: %d\n" "\x1B[0m", score);
}

void updateGhosts() {
	int *distMap = new int[width * height]{};

	struct Pos {
		int x, y;
	};

	int currentDist = 0; // Distanz von der Zierposition
	std::vector<Pos> lastSet; // zuletzt ausgefuellte Felder

	lastSet.push_back({pacPosX, pacPosY});

	for(;;) {
		std::vector<Pos> newLastSet;

		for(int i=0; i < lastSet.size(); i++) {
			if(walls[lastSet[i].y * width + lastSet[i].x-1] == false && distMap[lastSet[i].y * width + lastSet[i].x-1] == 0) { // Keine Wand links
				distMap[lastSet[i].y * width + lastSet[i].x-1] = currentDist;
				newLastSet.push_back({lastSet[i].x-1, lastSet[i].y});
			}
			if(walls[lastSet[i].y * width + lastSet[i].x+1] == false && distMap[lastSet[i].y * width + lastSet[i].x+1] == 0) { // Keine Wand rechts
				distMap[lastSet[i].y * width + lastSet[i].x+1] = currentDist;
				newLastSet.push_back({lastSet[i].x+1, lastSet[i].y});
			}
			if(walls[(lastSet[i].y-1) * width + lastSet[i].x] == false && distMap[(lastSet[i].y-1) * width + lastSet[i].x] == 0) { // Keine Wand oben
				distMap[(lastSet[i].y-1) * width + lastSet[i].x] = currentDist;
				newLastSet.push_back({lastSet[i].x, lastSet[i].y-1});
			}
			if(walls[(lastSet[i].y+1) * width + lastSet[i].x] == false && distMap[(lastSet[i].y+1) * width + lastSet[i].x] == 0) { // Keine Wand unten
				distMap[(lastSet[i].y+1) * width + lastSet[i].x] = currentDist;
				newLastSet.push_back({lastSet[i].x, lastSet[i].y+1});
			}
		}

		currentDist++;

		if(newLastSet.size() == 0)
			break;

		lastSet = newLastSet;
	}

	{
		int minDist = 1000000;

		if(distMap[ghosts[0].y * width + ghosts[0].x-1] < minDist && walls[ghosts[0].y * width + ghosts[0].x-1] == false) {
			minDist = distMap[ghosts[0].y * width + ghosts[0].x-1];
			ghosts[0].dirX = -1;
			ghosts[0].dirY = 0;
		}
		if(distMap[ghosts[0].y * width + ghosts[0].x+1] < minDist && walls[ghosts[0].y * width + ghosts[0].x+1] == false) {
			minDist = distMap[ghosts[0].y * width + ghosts[0].x+1];
			ghosts[0].dirX = 1;
			ghosts[0].dirY = 0;
		}
		if(distMap[(ghosts[0].y-1) * width + ghosts[0].x] < minDist && walls[(ghosts[0].y-1) * width + ghosts[0].x] == false) {
			minDist = distMap[(ghosts[0].y-1) * width + ghosts[0].x];
			ghosts[0].dirX = 0;
			ghosts[0].dirY = -1;
		}
		if(distMap[(ghosts[0].y+1) * width + ghosts[0].x] < minDist && walls[(ghosts[0].y+1) * width + ghosts[0].x] == false) {
			minDist = distMap[(ghosts[0].y+1) * width + ghosts[0].x];
			ghosts[0].dirX = 0;
			ghosts[0].dirY = 1;
		}
	}

	ghosts[0].x += ghosts[0].dirX;
	ghosts[0].y += ghosts[0].dirY;

	delete[] distMap;
}

void update() {
	if(Platform::kbPressed()) { // Wenn Taste gedrueckt
		char input = Platform::getPressedKey(); // Die gedrückte Taste
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

	static int tick = 0;
	if(tick++ % 8 != 0)
		updateGhosts();
}

int main(int argc, char** argv) {
	Platform::configTerminal();

	loadMap();

	for(;;) {
		update();
		printMap();
		// Sleep(100);
		Platform::sleepMS(100);
	}

	printf("\x1B[31m" "Hello World!\n" "\x1B[0m");
	return 0;
}