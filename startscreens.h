#include <cmath>
#include <cstdlib>
#include <cstdint>

#include <algorithm>

#include "platform.h"

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
	constexpr int NUM_COLORS = 14;
	constexpr int COLORS[] = { 34, 94,  36, 96,  32, 92,  35, 95,  31, 91,  33, 93,  37, 97 };

	printf("\x1B[?25l"); // Cursor deaktivieren
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>

	// Top Frame line
	printf("+-");
	for(int x = 0; x < width; x++)
		printf("-");
	printf("-+\n");

	for(int y = 0; y < height; y++) {
		printf("\x1B[0m"); // Konsolenfarbe zuruecksetzen
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

		printf("\x1B[0m"); // Konsolenfarbe zuruecksetzen
		printf(" |\n"); // Right Frame line
	}

	// Bottom Frame line
	printf("\x1B[0m"); // Konsolenfarbe zuruecksetzen
	printf("+-");
	for(int x = 0; x < width; x++)
		printf("-");
	printf("-+\n\n");

	printf("    Press any key to continue...\n");
}




void printStartScreen2() {
	static float a = 0; a += .2;

	printf("\x1B[?25l"); // Konsolen-Cursor deaktivieren
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
	printf("\x1B[2J"); // Gesamte Konsole leeren

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