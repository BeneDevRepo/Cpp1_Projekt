#include <iostream>
#include <cstdint>

#include <cstdlib>
#include <ctime>

#include <cmath>
#include <algorithm>

#include <vector>

#include "platform.h"

// void flushInput() {
// 	for(;;) { const int c = getchar(); if(c=='\n' || c==EOF) break; }
// }

struct vec2 {
	int x, y;
};

struct Ghost {
	int x, y; // position des Geistes
	vec2 dir; // richtung
};

enum class State { START_SCREEN, GAME, ENDSCREEN };
State state = State::START_SCREEN;

namespace testMap {
	const uint16_t WIDTH = 28;
	const uint16_t HEIGHT = 31;

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
};

// Spielstatus:
int MAX_SCORE = 0;
int score = 0;

int leben;

int width = 0, height = 0;
bool *walls = nullptr; // true = Wand
bool *coins = nullptr; // true = Muenze

int pacPosX, pacPosY;
int pacDirX, pacDirY;

const int NUM_GHOSTS = 4;
Ghost ghosts[NUM_GHOSTS]; // blinky
// /Spielstatus

void loadStartPositions() {
	pacPosX = 13;
	pacPosY = 23;

	pacDirX = 0;
	pacDirY = -1;

	ghosts[0].x = 13;
	ghosts[0].y = 11;

	ghosts[1].x = 12;
	ghosts[1].y = 13;

	ghosts[2].x = 13;
	ghosts[2].y = 13;

	// ghosts[3].x = 13;
	// ghosts[3].y = 11;

	for(int i = 0; i < NUM_GHOSTS; i++)
		ghosts[i].dir = {0};
}

void loadMap() {
	score = 0;
	leben = 3;

	loadStartPositions();

	width = testMap::WIDTH;
	height = testMap::HEIGHT;
	walls = new bool[width * height]{};
	coins = new bool[width * height]{};

	MAX_SCORE = 0;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			char stelle = testMap::LEVEL_INIT[y][x];
			if (stelle=='W')
				walls[y * width + x] = true;
			if (stelle=='.') {
				coins[y * width + x] = true;
				MAX_SCORE++;
			}
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
			}
			else if(x == ghosts[0].x && y == ghosts[0].y) {
				printf("\x1B[31m"); // Rot
				printf("U "); // Geist
				printf("\x1B[0m");
			} 
			else if(x == ghosts[1].x && y == ghosts[1].y) {
				printf("\x1B[95m"); // Pink
				printf("U "); // Geist
				printf("\x1B[0m");
			} 
			else if(x == ghosts[2].x && y == ghosts[2].y) {
				printf("\x1B[36m"); // Tuerkis / Cyan
				printf("U "); // Geist
				printf("\x1B[0m");
			} 
			else if(x == ghosts[3].x && y == ghosts[3].y) {
				printf("\x1B[37m"); // Rot
				printf("U "); // Geist
				printf("\x1B[0m");
			} 
			else if(walls[y * width + x]) {
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

	printf("\x1B[31m"); // Rot
	for(int i = 0; i < leben; i++)
		// printf("♥ ");
		printf("%s ", Platform::HEART);
		// printf("%c ", 3);
	printf("      ");
	printf("\x1B[0m");
}

vec2 dirToTarget(int fromX, int fromY, int toX, int toY) {
	int *distMap = new int[width * height]{};

	int currentDist = 1; // Distanz zur Zierposition
	std::vector<vec2> lastSet; // zuletzt ausgefuellte Felder

	if(toY >= 0 && toY < height && toX >= 0 && toX < width && walls[toY * width + toX] == false) {
		lastSet.push_back({toX, toY});
	} else {
		// TODO Fuer waende sichern
		for(; lastSet.size() == 0; currentDist++) { // if Target pos inside Wall
			for(int16_t xRel = -currentDist; xRel <= currentDist; xRel++) {
				for(int8_t yFac = -1; yFac <= 1; yFac += 2) {
					const int16_t x = toX + xRel;
					const int16_t y = toY + yFac * (currentDist - abs(xRel));

					if(x < 0 || x > width-1 || y < 0 || y > height-1) // out of map; skip
						continue;

					if(walls[y * width + x]) // inside Wall; skip
						continue;

					distMap[y * width + x] = currentDist;
					lastSet.push_back({x, y});
				}
			}
		}
	}

	for(;;) {
		std::vector<vec2> newLastSet;

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

	if(toY >= 0 && toY < height && toX >= 0 && toX < width)
		if(walls[toY * width + toX] == false) // Distanz unter Pacman setzen
			distMap[toY * width + toX] = 0;


	vec2 dir;
	int minDist = 1000000;

	if(distMap[fromY * width + fromX-1] < minDist && walls[fromY * width + fromX-1] == false) {
		minDist = distMap[fromY * width + fromX-1];
		dir.x = -1;
		dir.y = 0;
	}
	if(distMap[fromY * width + fromX+1] < minDist && walls[fromY * width + fromX+1] == false) {
		minDist = distMap[fromY * width + fromX+1];
		dir.x = 1;
		dir.y = 0;
	}
	if(distMap[(fromY-1) * width + fromX] < minDist && walls[(fromY-1) * width + fromX] == false) {
		minDist = distMap[(fromY-1) * width + fromX];
		dir.x = 0;
		dir.y = -1;
	}
	if(distMap[(fromY+1) * width + fromX] < minDist && walls[(fromY+1) * width + fromX] == false) {
		minDist = distMap[(fromY+1) * width + fromX];
		dir.x = 0;
		dir.y = 1;
	}

	delete[] distMap;

	return dir;
}

void updateGhosts() {
	// vec2 dirToPacman = 
	ghosts[0].dir = dirToTarget(ghosts[0].x, ghosts[0].y, pacPosX, pacPosY);
	ghosts[1].dir = dirToTarget(ghosts[1].x, ghosts[1].y, pacPosX+pacDirX*4, pacPosY+pacDirY*4);

	vec2 pacPlus2 = { pacPosX+pacDirX * 2, pacPosY+pacDirY * 2 }; // Punkt 2 Felder vor Pacman
	vec2 fromBlinkyToPacFront = { pacPlus2.x - ghosts[0].x, pacPlus2.y - ghosts[0].y }; // Richtung von Blinky zum Feld 2 Felder vor Pacman
	vec2 inkyTarget = { ghosts[0].x + fromBlinkyToPacFront.x * 2, ghosts[0].y + fromBlinkyToPacFront.y * 2 };
	ghosts[2].dir = dirToTarget(ghosts[2].x, ghosts[2].y, inkyTarget.x, inkyTarget.y);

	// ghosts[0].dirX = dirToPacman.x;
	// ghosts[0].dirY = dirToPacman.y;
	// ghosts[0].x += ghosts[0].dir.x;
	// ghosts[0].y += ghosts[0].dir.y;
	for(int i = 0; i < NUM_GHOSTS; i++) {
		ghosts[i].x += ghosts[i].dir.x;
		ghosts[i].y += ghosts[i].dir.y;
	}
}

void updateGame() {
	if(Platform::kbPressed()) { // Wenn Taste gedrueckt
		char input = Platform::getPressedKey(); // Die gedrückte Taste
		switch(input) {
			case 'w':
			case 'W':
				pacDirX = 0;
				pacDirY = -1;
				break;

			case 's':
			case 'S':
				pacDirX = 0;
				pacDirY = 1;
				break;

			case 'a':
			case 'A':
				pacDirX = -1;
				pacDirY = 0;
				break;

			case 'd':
			case 'D':
				pacDirX = 1;
				pacDirY = 0;
				break;
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
			if(score == MAX_SCORE) {
				printf("\x1B[2J"); // Erase Entire Viewport
				state = State::ENDSCREEN;
				// GEWONNEN
			}
		}
	}

	static int tick = 0;
	if(tick++ % 8 != 0)
		updateGhosts();

	for(int i = 0; i < NUM_GHOSTS; i++) { // Leben abziehen bei Kollision mit Geist
		if(ghosts[i].x == pacPosX && ghosts[i].y == pacPosY) {
			leben--;
			if(leben == 0) { // Tot
				// printf("\x1B[2J"); // Erase Entire Viewport
				state = State::ENDSCREEN;
				break;
			}
			printMap();
			loadStartPositions();
			Platform::sleepMS(500);
			printMap();
			Platform::sleepMS(750);
			break;
		}
	}
}

void printStartScreen1() {
	const int width = 65;
	const int height = 25;
	static int tick = 0; tick++;

	constexpr int NUM_BALLS = 7;
	static struct Ball {
		float x, y, dirX, dirY, r;
		Ball() {
			x = rand() * .5 * width / RAND_MAX;
			y = rand() * .5 * height / RAND_MAX;
			dirX = rand() * 2. / RAND_MAX - 1;
			dirY = rand() * 2. / RAND_MAX - 1;
			r = rand() * 1.5 / RAND_MAX + 1.;
			// Richtung normalisieren:
			const float dirMag = sqrt(dirX*dirX + dirY*dirY);
			dirX /= dirMag;
			dirY /= dirMag;

			// Gschwindigkeit:
			dirX *= .5;
			dirY *= .5;
		}
	} balls[NUM_BALLS]{};

	for(int i = 0; i < NUM_BALLS; i++) {
		balls[i].x += balls[i].dirX;
		balls[i].y += balls[i].dirY;
		if(balls[i].x-1 < 0 || balls[i].x+1 > width/2)
			balls[i].dirX *= -1;
		if(balls[i].y-1 < 0 || balls[i].y+1 > height)
			balls[i].dirY *= -1;
	}

	const auto map = [](const float val, const float vMin, const float vMax, const float outMin, const float outMax){
			return outMin + (val - vMin) / (vMax - vMin) * (outMax - outMin);
		};

	const auto constrain = [](const int val, const int min, const int max) {
			return std::min<int>(std::max<int>(val, min), max);
		};

	const auto setColor = [](const uint8_t r, const uint8_t g, const uint8_t b) {
			printf("\x1B[38;2;%hhu;%hhu;%hhum", r, g, b);
		};

	constexpr int NUM_CHARS = 8;
	constexpr const char CHARS[] = ".-=+*#%@";

	// constexpr int NUM_COLORS = 7;
	// constexpr int COLORS[] = {31, 32, 33, 34, 35, 36, 37};
	constexpr int NUM_COLORS = 14;
	constexpr int COLORS[] = {34, 94,  36, 96,  32, 92,  35, 95,  31, 91,  33, 93,  37, 97};



	printf("\x1B[?25l"); // Disable Cursor
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>

	// Top Frame line
	printf("+-");
	for(int x = 0; x < width; x++)
		printf("-");
	printf("-+\n");

	for(int y = 0; y < height; y++) {
		printf("\x1B[0m"); // Reset Console Colors
		printf("| "); // Left Frame line

		for(int x = 0; x < width; x++) {
			float dist = 0;
			for(int i = 0; i < NUM_BALLS; i++)
				dist += balls[i].r / sqrtf((x/2. - balls[i].x)*(x/2. - balls[i].x) + (y - balls[i].y)*(y - balls[i].y));

			// Umskalierung
			// dist -= -.05;
			// dist *= dist;
			// dist += -.05;

			printf("\x1B[%hhum", COLORS[constrain(map(dist, .03, 1.9, 0, NUM_COLORS-1), 0, NUM_COLORS-1)]);
			// printf("\x1B[%hhum", COLORS[x % NUM_COLORS]);
			printf("%c", CHARS[constrain(map(dist, .03, 1.9, 0, NUM_CHARS-1), 0, NUM_CHARS-1)]);

			// if(x % NUM_COLORS == NUM_COLORS-1) // Debugging
			// 	printf(" ");

			// printf("%c", "@%#*+=-:."[constrain(map(dist, 3, .1, 0, 8), 0, 8)]);
		}

		printf("\x1B[0m"); // Reset Console Colors
		printf(" |\n"); // Right Frame line
	}

	// Bottom Frame line
	printf("\x1B[0m"); // Reset Console Colors
	printf("+-");
	for(int x = 0; x < width; x++)
		printf("-");
	printf("-+\n\n");

	printf("    Press any key to continue...\n");

	// Platform::sleepMS(100);
}

void printStartScreen2() {
	static float a = 0;
	a += .2;
	printf("\x1B[?25l"); // Disable Cursor
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
	printf("\x1B[2J"); // Erase Entire Viewport

	static constexpr auto mvPrintC = [](const uint16_t x, const uint16_t y, const char c) {
		printf("\x1B[%hu;%huH%c", y, x, c); // Cursor pos <y; x> + print char
	};

	static constexpr auto line = [](int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
		if(std::abs(x2 - x1) >= std::abs(y2 - y1)) {
			if(x1 > x2) {
				std::swap(x1, x2);
				std::swap(y1, y2);
			}
			const char lineChar = y1 == y2 ? '-' : (y1 > y2 ? '/' : '\\');
			for(int16_t x = std::max<int16_t>(x1, 0); x <= x2; x++) {
				mvPrintC(x, y1 + (x-x1) * (y2-y1) / (x2-x1), lineChar);
			}
		} else {
			if(y1 > y2) {
				std::swap(x1, x2);
				std::swap(y1, y2);
			}
			const char lineChar = x1 == x2 ? '|' : (x1 > x2 ? '/' : '\\');
			for(int16_t y = std::max<int16_t>(y1, 0); y <= y2; y++) {
				mvPrintC(x1 + (y-y1) * (x2-x1) / (y2-y1), y, lineChar);
			}
		}
	};
	// line(2, 4, 8, 10);
	line(15, 7, 10 + cos(a) * 15, 7 + sin(a) * 5);

	Platform::sleepMS(100);
}

void printStartScreen() {
	printStartScreen1();
	// printStartScreen2();
}

int main(int argc, char** argv) {
	srand(time(0));

	Platform::configTerminal();

	printf("\x1B[2J"); // Erase Entire Viewport

	loadMap();

	for(;;) {
		switch(state) {
			case State::ENDSCREEN:
				// End startscreen on Keypress:
				// if(Platform::kbPressed()) {
				// 	Platform::getPressedKey(); // Consume key event
				// 	state = State::GAME;
				// 	printf("\x1B[2J"); // Erase Entire Viewport
				// 	break;
				// }
				printf("\x1B[2J"); // Erase Entire Viewport
				printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
				printf("Das spiel ist vorbei. Ihr score war: %d\n", score);
			break;

			case State::START_SCREEN:
				// End startscreen on Keypress:
				if(Platform::kbPressed()) {
					Platform::getPressedKey(); // Consume key event
					state = State::GAME;
					printf("\x1B[2J"); // Erase Entire Viewport
					break;
				}

				// Logic + Draw + Delay
				printStartScreen();
			break;


			case State::GAME:
				// Logic
				updateGame();

				// Draw
				printMap();

				// Delay
				Platform::sleepMS(100);
			break;
		}
	}

	printf("\x1B[31m" "Hello World!\n" "\x1B[0m");
	return 0;
}