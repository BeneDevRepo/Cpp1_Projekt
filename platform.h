/* 
 * Datum: 7.1.2022
 * Autor: Benedikt Binger
 */

#pragma once

#include <cstdio>
#include <cstdint>

#if _WIN32
	#include <windows.h>
	#include <conio.h>
#endif

#if __linux__
	#include <unistd.h>
	#include <termios.h>
	#include <sys/select.h>
#endif

namespace Platform {
	#if _WIN32
		constexpr const char *const HEART = "\x03"; // Herz fuer Windows konsole (ANSI)
	#endif

	#if __linux__
		constexpr const char *const HEART = "♥"; // Herz fuer linux konsole (UTF-8)
	#endif

	void configTerminal() { // Wird 1 mal in main aufgerufen um die Konsole vorzubereiten
		#if _WIN32
			CONSOLE_CURSOR_INFO cursorInfo;
			cursorInfo.dwSize = 1; // curser-hoehe auf Minimum setzen
			cursorInfo.bVisible = FALSE; // cursor deaktivieren
			SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo); // Cursor-Einstellungen speichern
		#endif

		#if __linux__
			termios terminalState;
			tcgetattr(STDIN_FILENO, &terminalState); // Terminal status laden

			terminalState.c_lflag &= ~ICANON; // Buffered I/O deaktivieren (Input buffer jederzeit ohne Enter verfuegbar)
			terminalState.c_lflag &= ~ECHO; // Echo modus deaktivieren

			tcsetattr(STDIN_FILENO, TCSANOW, &terminalState);// Terminal status speichern
		#endif
	}

	bool kbPressed() { // true wenn beliebige Taste gedrueckt wurde (bzw. mindestens 1 neues Zeichen im input-puffer)
		#if _WIN32
			return _kbhit();
		#endif

		#if __linux__
			timeval tv{}; // Timeout nach 0 Sekunden fuer select()
			fd_set fds; // FileDescriptor-set um select() auf stdin anzuwenden
			FD_ZERO(&fds); // fds initialisieren
			FD_SET(STDIN_FILENO, &fds); // stdin zu fds hinzufuegen
			select(STDIN_FILENO+1, &fds, NULL, NULL, &tv); // select() mit 0 sekunden Timeout nutzen um zu pruefen ob neue Zeichen im input puffer sind; Ergebnis wird in fds gespeichert
			return (FD_ISSET(0, &fds)); // Ergebnis von select() aus fds auslesen und zurueckgeben
		#endif
	}

	int getPressedKey() { // Gedrueckte Taste aus input-buffer auslesen
		#if _WIN32
			return _getch(); // Windows-interne Funktion um 1 Zeichen aus dem Input-Puffer zu lesen ohne auf 'Enter' Zu warten
		#endif

		#if __linux__
			return getchar(); // C-Funktion um 1 Zeichen aus Input-Puffer auszulesen; Funktioniert unter linux ohne 'Enter', wenn die Konsole zuvor korrekt konfiguriert wurde.
		#endif
	}

	void sleepMS(const uint32_t ms) { // Delay in Millisekunden
		#if _WIN32
			Sleep(ms); // Windows-interne sleep funktion in millisekunden
		#endif

		#if __linux__
			usleep(ms * 1000); // Linux-interne Sleep funktion in mikrosekunden
		#endif
	}
};