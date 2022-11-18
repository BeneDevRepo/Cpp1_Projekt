/* 
 * Datum: 7.1.2022
 * Autoren: Martin schmidt, Benedikt Binger, Hendrik Lange
 */


#include <cstdio>
#include <cmath>

#include <cstdlib>
#include <ctime>

#include <memory.h>

#include <vector>

#include "platform.h" // Plattform-spezifischer Code
#include "map.h" // Pacman-Level

// 2D - Vektor fuer Positionen u. Richtungen:
struct vec2 {
	int x, y;
};

// Geist:
struct Ghost {
	vec2 pos; // position
	vec2 dir; // richtung
};

// Globaler status des Programms:
enum class State { START_SCREEN, MENU, HELPPAGE, GAME, ENDSCREEN };
State state = State::START_SCREEN;


// === Spielstatus:
int level;
int leben;

int MAX_SCORE; // Summe aller muenzen, also maximaler erreichbarer Punktestand des Levels
int score;

int width, height; // Breite u. Hoehe des Spielfeldes
bool *walls; // true = Wand
bool *coins; // true = Muenze

vec2 pacPos; // Position pacmans
vec2 pacDir; // Richtung pacmans

const int NUM_GHOSTS = 4;
Ghost ghosts[NUM_GHOSTS]; // Geister
// === /Spielstatus


// Setzt die Positionen und Richtungen der Geister und Pacman auf ihre Startwerte:
void resetPositions() {
	pacPos.x = 13;
	pacPos.y = 23;

	pacDir.x = 0;
	pacDir.y = -1; // Pacman sieht zu beginn des spiels nach oben in die Wand

	ghosts[0].pos.x = 13;
	ghosts[0].pos.y = 11;

	ghosts[1].pos.x = 12;
	ghosts[1].pos.y = 13;

	ghosts[2].pos.x = 13;
	ghosts[2].pos.y = 13;

	ghosts[3].pos.x = 14;
	ghosts[3].pos.y = 13;

	for(int i = 0; i < NUM_GHOSTS; i++)
		ghosts[i].dir = {0, 0};
}

// initialisiert das Spielfeld und den sonstigen Spielzustand:
void loadMap() {
	level = 0;
	score = 0;
	leben = 3;

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

	resetPositions();
}

// gibt Speicher des Spielfeldes wieder frei:
void freeMap() {
	delete[] walls;
	delete[] coins;
}

// Druckt das Spiel aus:
void printMap() {
	static int frame = 0;
	frame++;

	printf("\x1B[?25l"); // Konsolen-Cursor deaktivieren
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>

	printf("Level %d\n\n", level + 1);

    printf("\x1B[40m"); // Hintergrund schwarz
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(x == pacPos.x && y == pacPos.y) { // Pac Man
				// printf("\x1B[33m" "C " "\x1B[0m");
				printf("\x1B[33m"); // Gelb
				if(frame%2 == 0) {
					printf("o ");
				} else {
					if(pacDir.x == 1)
						printf("< ");
					else if(pacDir.x == -1)
						printf("> ");
					else if(pacDir.y == 1)
						printf("^ ");
					else if(pacDir.y == -1)
						printf("V ");
				}
				// printf("\x1B[0m");
			}
			else if(x == ghosts[0].pos.x && y == ghosts[0].pos.y) {
				printf("\x1B[31m"); // Rot
				printf("U "); // Geist
				// printf("\x1B[0m");
			} 
			else if(x == ghosts[1].pos.x && y == ghosts[1].pos.y) {
				printf("\x1B[95m"); // Pink
				printf("U "); // Geist
				// printf("\x1B[0m");
			} 
			else if(x == ghosts[2].pos.x && y == ghosts[2].pos.y) {
				printf("\x1B[36m"); // Tuerkis / Cyan
				printf("U "); // Geist
				// printf("\x1B[0m");
			} 
			else if(x == ghosts[3].pos.x && y == ghosts[3].pos.y) {
				printf("\x1B[37m"); // Blau
				printf("U "); // Geist
				// printf("\x1B[0m");
			} 
			else if(walls[y * width + x]) {
				printf("\x1B[34m"); // Blau
                printf("WW"); // Wand
                // printf("\x1B[0m");
			} else if(coins[y * width + x]) {
				printf("\x1B[33m"); // Gelb
				printf(". "); // Muenze
				// printf("\x1B[0m");
			} else {
				printf("  "); // Leer
			}
		}
		printf("\n");
	}

    printf("\x1B[0m");

	printf("\x1B[35m" "\nScore: %d\n" "\x1B[0m", score);

	printf("\x1B[31m"); // Rot
	for(int i = 0; i < leben; i++)
		printf("%s ", Platform::HEART);
	printf("      \n"); // Konsole wird nicht jeden Frame geleert, daher muessen die Herzen manuell ueberschrieben werdens
	printf("\x1B[0m");
}

// berechnet die optimale Richtung um von Punkt 'from' nach 'to' zu gelangen:
vec2 dirToTarget(int fromX, int fromY, int toX, int toY) {
	int *const distMap = new int[width * height]{}; // Array mit Distanzen zur Zielposition

	std::vector<vec2> lastSet; // zuletzt ausgefuellte Felder

	// Startpositionen fuer A* - Ausbreitung finden:
	if(toY >= 0 && toY < height && toX >= 0 && toX < width && walls[toY * width + toX] == false) { // falls Zielposition betretbar ist
		lastSet.push_back({toX, toY}); // Zielposition als Startposition fuer A*-Algorithmus auswaehlen; der Eintrag in distMap wird nach der Propagationsphase vorgenommen.
	} else { // falls Zielposition in einer Wand oder ausserhalb des Spielfeldes liegt:
		
		// mit Manhattan-Distanz 1 starten und solange distanz erhoehen bis mindestens eine A*-Startposition gefunden wurde:
		for(int currentDist = 1; lastSet.size() == 0; currentDist++) {

			// Isolinie (alle Punkte mit gleichem Wert) aller Punkte mit manhattan-Distanz <currentDist> zu (toX|toY) von links nach rechts durchgehen: (Rautenform)
			for(int xRel = -currentDist; xRel <= currentDist; xRel++) {

				// Erst oberen, dann unteren Punkt der Raute an x = xRel testen:
				for(int yFac = -1; yFac <= 1; yFac += 2) {

					// Absolute Koordinaten im Spielfeld ausrechnen:
					const int x = toX + xRel;
					const int y = toY + yFac * (currentDist - abs(xRel));

					// Punkte ausserhalb des spielfeldes ueberspringen:
					if(x < 0 || x > width-1 || y < 0 || y > height-1) 
						continue;

					// Waende ueberspringen:
					if(walls[y * width + x]) 
						continue;

					distMap[y * width + x] = currentDist; // Distanz in distMap eintragen
					lastSet.push_back({x, y}); // Punkt zu Menge der Startpunkte fuer A* hinzufuegen

					// Wenn ein valider startpunkt erkannt wurde, werden danach noch alle weiteren Punkte gleicher Distanz getestet und ggf. ausgewaehlt,
					// danach wird die Suche nach Startpunkten dann abgebrochen.
				}
			}
		}
	}

	// Von den Markierten Startpunkten aus wellenartig ausbreiten, alle infrage kommenden benachbarten Punkte mit erhoehter Distanz fuellen
	// und die neu gefuellten Punkte zur Startmenge im naechsten Durchlauf machen:
	for(;;) {
		std::vector<vec2> newLastSet; // die Menge der in diesem Durchgang markierten Punkte; im naechsten Durchgang sind dies die Startpunkte.

		// Durch die Menge aller Startpunkte (Wellenfront) dieses Durchgangs iterieren:
		for(int i = 0; i < lastSet.size(); i++) {
			const vec2 p = lastSet[i];

			const int currentDist = distMap[p.y * width + p.x] + 1; // die Distanz der infrage kommenden Nachbarfelder um das aktuelle Feld herum ist 1 hoeher als die distanz des aktuellen Feldes zum Ziel

			// Fuer alle 4 Richtungen:
			// 1. Testen ob Feld in die Richtung innerhalb des Spielfeldes liegt
			// 2. Testen ob das Feld keine Wand ist
			// 3. Testen ob das Feld noch frei ist, um keine bereits beschriebenen Felder zu ueberschreiben
			// Falls alle Bedingungen zutreffen:
			//	1. Feld mit neuer Distanz fuellen
			//	2. Feld zur menge der in diesem Durchlauf gefuellten Felder hinzufuegen

			if(p.x-1 >= 0 && walls[p.y * width + p.x-1] == false && distMap[p.y * width + p.x-1] == 0) { // Keine Wand links
				distMap[p.y * width + p.x-1] = currentDist; // Feld mit Distanz fuellen
				newLastSet.push_back({p.x-1, p.y}); // Feld zur menge der in diesem Durchlauf gefuellten Felder hinzufuegen
			}

			if(p.x+1 <= width-1 && walls[p.y * width + p.x+1] == false && distMap[p.y * width + p.x+1] == 0) { // Keine Wand rechts
				distMap[p.y * width + p.x+1] = currentDist; // Feld mit Distanz fuellen
				newLastSet.push_back({p.x+1, p.y}); // Feld zur menge der in diesem Durchlauf gefuellten Felder hinzufuegen
			}

			if(p.y-1 >= 0 && walls[(p.y-1) * width + p.x] == false && distMap[(p.y-1) * width + p.x] == 0) { // Keine Wand oben
				distMap[(p.y-1) * width + p.x] = currentDist; // Feld mit Distanz fuellen
				newLastSet.push_back({p.x, p.y-1}); // Feld zur menge der in diesem Durchlauf gefuellten Felder hinzufuegen
			}

			if(p.y+1 <= height-1 && walls[(p.y+1) * width + p.x] == false && distMap[(p.y+1) * width + p.x] == 0) { // Keine Wand unten
				distMap[(p.y+1) * width + p.x] = currentDist; // Feld mit Distanz fuellen
				newLastSet.push_back({p.x, p.y+1}); // Feld zur menge der in diesem Durchlauf gefuellten Felder hinzufuegen
			}
		}

		if(newLastSet.size() == 0) // Wenn im gesamten Durchgang kein einziges Feld gefuellt ist, dann ist die Distanzkarte fertig.
			break;

		lastSet = newLastSet; // die Menge der neu gefuellten Punkte zur Startmenge des naechsten Durchlaufs machen
	}

	// falls moeglich Distanz an Zielposition auf 0 setzen:
	if(toY >= 0 && toY < height && toX >= 0 && toX < width)
		if(walls[toY * width + toX] == false)
			distMap[toY * width + toX] = 0;

	// Distanzen in allen 4 Richtungen der Startposition vergleichen und kleinste Distanz als optimale Richtung zum Ziel zurueckgeben:
	vec2 dir; // Resultat (optimale richtung zum Zielpunkt)
	int minDist = 1000000; // kleinste bisher gefundene Distanz (startwert hoeher alls alle realistisch zu erwartenden Distanzen in distMap)

	// fuer alle 4 Richtungen:
	// 1. testen ob Feld in die Richtung innerhalb des Spielfeldes liegt
	// 2. testen ob distanz zum ziel in die Richtung kleiner ist als alle bisherigen gefundenen Distanzen
	// 3. testen ob Feld in die Richtung keine Wand ist
	// Falls alle 3 Bedingungen erfuellt sind:
	//	1. minimale bisher gefundene distanz mit distanz des Feldes in die Richtung ueberschreiben
	//	2. Richtung der minimalen bisher gefundenen Distanz mit gerade ueberpruefter Richtung ueberschreiben
	if(fromX-1 >= 0 && distMap[fromY * width + fromX-1] < minDist && walls[fromY * width + fromX-1] == false) { // links
		minDist = distMap[fromY * width + fromX-1]; // kleinste bisher gefundene Distanz anpassen
		dir = {-1, 0}; // optimalste bisher gefundene Richtung ueberschreiben
	}
	if(fromX+1 <= width-1 && distMap[fromY * width + fromX+1] < minDist && walls[fromY * width + fromX+1] == false) { // rechts
		minDist = distMap[fromY * width + fromX+1]; // kleinste bisher gefundene Distanz anpassen
		dir = {1, 0}; // optimalste bisher gefundene Richtung ueberschreiben
	}
	if(fromY-1 >= 0 && distMap[(fromY-1) * width + fromX] < minDist && walls[(fromY-1) * width + fromX] == false) { // oben
		minDist = distMap[(fromY-1) * width + fromX]; // kleinste bisher gefundene Distanz anpassen
		dir = {0, -1}; // optimalste bisher gefundene Richtung ueberschreiben
	}
	if(fromY+1 <= height-1 && distMap[(fromY+1) * width + fromX] < minDist && walls[(fromY+1) * width + fromX] == false) { // unten
		minDist = distMap[(fromY+1) * width + fromX]; // kleinste bisher gefundene Distanz anpassen
		dir = {0, 1}; // optimalste bisher gefundene Richtung ueberschreiben
	}

	delete[] distMap; // Speicher von distMap freigeben 

	return dir; // optimale Richtung zum Ziel zurueckgeben
}

// Kartesische Distanz nach Satz des Pythagoras:
int dist (int x1, int y1, int x2, int y2) {
	return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

void updateGhosts() {
	// Blinky:
	ghosts[0].dir = dirToTarget(ghosts[0].pos.x, ghosts[0].pos.y, pacPos.x, pacPos.y);

	// Pinky:
	ghosts[1].dir = dirToTarget(ghosts[1].pos.x, ghosts[1].pos.y, pacPos.x + pacDir.x*4, pacPos.y + pacDir.y*4);

	// Inky:
	if(level == 0 && score >= 30 || level > 0) {
		vec2 pacPlus2 = { pacPos.x+pacDir.x * 2, pacPos.y+pacDir.y * 2 }; // Punkt 2 Felder vor Pacman
		vec2 fromBlinkyToPacFront = { pacPlus2.x - ghosts[0].pos.x, pacPlus2.y - ghosts[0].pos.y }; // Richtung von Blinky zum Feld 2 Felder vor Pacman
		vec2 inkyTarget = { ghosts[0].pos.x + fromBlinkyToPacFront.x * 2, ghosts[0].pos.y + fromBlinkyToPacFront.y * 2 }; // Zielposition fuer Inky
		ghosts[2].dir = dirToTarget(ghosts[2].pos.x, ghosts[2].pos.y, inkyTarget.x, inkyTarget.y);
	}

	// Clyde:
	if(level==0 && score >= 60 || level==1 && score >= 50 || level > 1) {
		int clydeDist = dist(ghosts[3].pos.x, ghosts[3].pos.y, pacPos.x, pacPos.y);
		if(clydeDist > 8) {
			ghosts[3].dir = dirToTarget(ghosts[3].pos.x, ghosts[3].pos.y, pacPos.x, pacPos.y);
		} else {
			ghosts[3].dir = dirToTarget(ghosts[3].pos.x, ghosts[3].pos.y, 1, height-2); // untere linke Ecke
		}
	}

	// Geister bewegen:
	for(int i = 0; i < NUM_GHOSTS; i++) {
		ghosts[i].pos.x += ghosts[i].dir.x;
		ghosts[i].pos.y += ghosts[i].dir.y;
	}
}

// Testet ob pacman einen Geist beruehrt hat und handelt dementsprechend:
bool manageGhostCollision() { // gibt true zurueck falls pacman getroffen wurde
	for(int i = 0; i < NUM_GHOSTS; i++) { // Leben abziehen bei Kollision mit Geist
		if(ghosts[i].pos.x == pacPos.x && ghosts[i].pos.y == pacPos.y) {
			leben--; // 1 Leben abziehen

			printMap();
			Platform::sleepMS(500); // Todesursache 0.5 Sekunden lang anzeigen

			if(leben == 0) { // Tot / Verloren
				printf("\x1B[2J"); // Gesamte Konsole leeren
				state = State::ENDSCREEN;
				return true;
			}

			resetPositions();
			printMap();
			Platform::sleepMS(750); // Wiedereinstiegspunkt 0.75 Sekunden lang anzeigen

			return true;
		}
	}
	return false;
}

void updateGame() {
	if(Platform::kbPressed()) { // Wenn Taste gedrueckt
		char input = Platform::getPressedKey(); // Die gedrückte Taste
		for(; Platform::kbPressed(); input = Platform::getPressedKey()); // Letztes Zeichen aus input Puffer auswaehlen, alle anderen entfernen
		switch(input) {
			case 'w':
			case 'W':
				pacDir.x = 0;
				pacDir.y = -1;
				break;

			case 's':
			case 'S':
				pacDir.x = 0;
				pacDir.y = 1;
				break;

			case 'a':
			case 'A':
				pacDir.x = -1;
				pacDir.y = 0;
				break;

			case 'd':
			case 'D':
				pacDir.x = 1;
				pacDir.y = 0;
				break;
		}
	}

	// Pacmans naechste Position berechnen:
	int nextX = pacPos.x + pacDir.x;
	int nextY = pacPos.y + pacDir.y;

	// Pacman an Portalen auf andere Seite bewegen:
	if(nextX == -1)
		nextX += width;
	if(nextX == width)
		nextX -= width;

	if(walls[nextY * width + nextX] == false) { // Wenn an Zielposition keine Wand
		// Pacman bewegen:
		pacPos.x = nextX;
		pacPos.y = nextY;

		if(coins[pacPos.y * width + pacPos.x]) {
			coins[pacPos.y * width + pacPos.x] = false; // Muenze einsammeln
			score++;

			if(score >= MAX_SCORE) { // Level gewonnen
				// printf("\x1B[2J"); // Gesamte Konsole leeren

				level++;
				resetPositions();
				freeMap();
				loadMap();

				printf("\x1B[20;%dH", width - 2); // Cursor pos <y=0; x=0>
				printf("\x1B[33m"); // Gelb
				printf("Ready!");
				printf("\x1B[0m"); // Farbe zuruecksetzen
				Platform::sleepMS(1000);
				// state = State::ENDSCREEN;
				// GEWONNEN
			}
		}
	}

	if(manageGhostCollision()) // Falls pacman getroffen wurde: Frame abbrechen da positionen ohnehin zurueckgesetzt werden
		return; // Verhindert dass pacman in 1 Frame 2 Leben verlieren kann

	static int tick = 0;
	if(tick++ % 8 != 0) // Geister ueberspringen jeden 8. Frame
		updateGhosts();

	manageGhostCollision(); // Kollision mit Geistern ein 2. Mal pruefen, damit Pacman nicht durch Geister hindurchlaufen kann
}

// Druckt startbildschirm:
void printStartScreen() {
	printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
	printf("+----+    *-------*   *------     *         *   *-------*   |      |        \n");
	printf("|     |   |       |   |           | \\     / |   |       |   | \\    |        \n");
	printf("|     |   |       |   |           |  \\   /  |   |       |   |  \\   |        \n");
	printf("+----*    *-------*   |           |    *    |   *-------*   |   \\  |        \n");
	printf("|         |       |   |           |         |   |       |   |    \\ |        \n");
	printf("|         |       |   *------     |         |   |       |   |      |        \n");

	printf("\n    Press any key to continue...\n");
	// printStartScreen1();
	// printStartScreen2();
}

void storeScore() {
	constexpr int NAME_BUFFER_SIZE = 128; // maximal 127 Zeichen + 1 Nullterminator
	char name[NAME_BUFFER_SIZE];
	int nameLength = 0;

	for(;;) {
		while(!Platform::kbPressed());
		const char cur = Platform::getPressedKey();

		if(cur == ',') continue; // Aufgrund des Dateiformats sind Kommas im Namen unzulaessig

		// Name wird durch 'Enter' bestaetigt:
		if(cur == '\r' || cur == '\n' && nameLength > 0) {
			name[nameLength] = 0; // Null-Terminator an name anfuegen
			FILE *const fp = fopen("Highscore.txt", "a");
			fprintf(fp,"%s, %d, ", name, score);
			fclose(fp);
			return;
		}

		if((cur == '\b' || cur == 127) && nameLength > 0) {
			printf("\b \b"); // Letzten Buchstaben aus Konsole loeschen
            fflush(stdout); // Weil linux
			nameLength--;
		}

		if((cur != '\b' && cur != 127) && nameLength < NAME_BUFFER_SIZE-1) {
			printf("%c", cur); // Buchstaben ausdrucken
            fflush(stdout); // Weil linux
			name[nameLength++] = cur;
		}
	}
}

int main(int argc, char** argv) {
	srand(time(0));

	Platform::configTerminal();

	printf("\x1B[2J"); // Gesamte konsole leeren


	for(;;) {
		switch(state) {
			case State::ENDSCREEN:
				printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
				printf("Das spiel ist vorbei. Ihr score war: %d\n\n", score);
                printf("Bitte geben sie ihren Namen ein: ");
                fflush(stdout); // Weil linux

				storeScore(); // Nutzernamen abfragen u. Score + Namen in Datei speichern

				freeMap(); // Speicher des Spielfeldes freigeben

                state = State::MENU;
			break;

			case State::START_SCREEN:
				// Startbildschirm nach Tastatureingabe beenden:
				if(Platform::kbPressed()) {
					char tmp = Platform::getPressedKey(); // Taste aus Inputpuffer konsumieren
                    // printf("%d", tmp);
					printf("\x1B[2J"); // Gesamte Konsole leeren
					state = State::MENU;
					break;
				}

				printStartScreen(); // Startbildschirm ausdrucken
			break;

			case State::HELPPAGE:
				printf("\x1B[2J"); // Gesamte konsole leeren
				printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>
				printf("Die Regeln:\n\n");

				printf("Sie steuern den Pac-Man mit WASD.\n");
				printf("Ihr Ziel ist es alle Muenzen auf dem Spielfeld \n");
				printf("einzusammeln ohne von den Geistern gefressen zu werden.\n");
				printf("Bei Kontakt mit einem der 4 Geister verlieren sie eines von 3 \n");
				printf("Leben und die Positionen aller Charaktere werden zurueckgesetzt, \n");
				printf("ihre gesammelten Muezen bleiben aber erhalten.\n");
				printf("Verlassen sie einen der beiden Ausgaenge, werden \n");
				printf("sie auf die gegenueberliegende Seite teleportiert.\n");
				printf("Verlieren sie alle 3 Leben ist das Spiel vorbei.\n\n");

				printf("Blinky, der rote Geist, verfolgt sie, sobald das Spiel beginnt.\n");
				printf("Pinky, der pinke Geist, versucht ihnen den Weg abzuschneiden.\n");
				printf("Inky, der blaue Geist, ist fast unberechnbar, greift aber oft von hinten an.\n");
				printf("Clyde, der orangene Geist, laeuft vor ihnen weg, sobald sie ihm zu nahe kommen.\n");
				printf("Alle Geister bewegen sich mit 80%% der Geschwindigkeit des Pac-Man.\n\n");

				printf("Druecken sie eine beliebige Taste um zurueckzukehren...\n");

				while(!Platform::kbPressed()); // Auf Eingabe warten
				Platform::getPressedKey(); // Eingabe konsumieren

				state = State::MENU; // zum Menü zurueckkehren
				break;


			case State::MENU:
				{
					FILE *fp = fopen("Highscore.txt", "r");
	
					if(!fp) {
                        fp = fopen("Highscore.txt", "w"); // Highscore datei erstellen falls sie nicht existiert
                        fclose(fp);

						fp = fopen("Highscore.txt", "r");
					}

					char highName[128];
					int highScore = -1;

					for(;;) {
						char name[128];
						int score;

						if(fscanf(fp, "%[^,], %d, ", name, &score) != 2)
							break;

						if(score > highScore || highScore == -1) {
							memcpy(highName, name, 128);
							highScore = score;
						}
					}
					fclose(fp);

					printf("\x1B[2J"); // Gesamte Konsole leeren
					printf("\x1B[0;0H"); // Cursor pos <y=0; x=0>

                    if(highScore != -1)
					    printf("Highscore: \"%s\" %d\n", highName, highScore);

					printf("Pac-Man\n");
					printf("1 = Spiel starten\n");
					printf("2 = Regeln und Steuerung\n");
					printf("3 = Beenden\n");

					while(!Platform::kbPressed());
					int option = Platform::getPressedKey() - '0';

					switch(option) {
						case 1:
							printf("\x1B[2J"); // Gesamte Konsole leeren
							printf("\x1B[15;23H"); // Cursor pos <y=15; x=23>
							printf("Das Spiel beginnt!");
							Platform::sleepMS(500);
							printf("\x1B[2J"); // Gesamte Konsole leeren
							state = State::GAME;
							loadMap(); // Spielfeld erstellen + startwerte setzen
							level = 0;
							break;

						case 2:
							state = State::HELPPAGE;
							break;

						case 3:
							return 0;

						default:
							printf("1, 2 oder 3\n\n");
							break;
					}
				}
				break;

			case State::GAME:
				// Logic
				updateGame();

				if(state != State::GAME) // Spielfeld nicht mehr zeichnen falls Spiel bereits verloren
					break;

				// Draw
				printMap();

				// Delay
				Platform::sleepMS(300);
			break;
		}
	}

	return 0;
}