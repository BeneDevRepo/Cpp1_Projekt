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
	void configTerminal() {
		#if _WIN32
			// HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
			// DWORD mode = 0;
			// GetConsoleMode(hStdin, &mode);
			// SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
		#endif

		#if __linux__
			termios terminalState;
			tcgetattr(STDIN_FILENO, &terminalState); //get the terminal state

			terminalState.c_lflag &= ~ICANON; // turn off canonical (buffered i/o) mode (Input buffer accessible in real time; no enter required)
			terminalState.c_lflag &= ~ECHO; // Disable echo mode

			tcsetattr(STDIN_FILENO, TCSANOW, &terminalState);//set the terminal attributes.
		#endif
	}

	bool kbPressed() { // true wenn beliebige Taste gedrueckt wurde
		#if _WIN32
			return _kbhit();
		#endif

		#if __linux__
			timeval tv{};
			fd_set fds;
			FD_ZERO(&fds);
			FD_SET(STDIN_FILENO, &fds);
			select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
			return (FD_ISSET(0, &fds));
		#endif
	}

	char getPressedKey() { // Gedrueckte Taste aus input-buffer auslesen
		#if _WIN32
			return _getch();
		#endif

		#if __linux__
			return getchar();
		#endif
	}

	void sleepMS(uint32_t ms) { // Delay in Millisekunden
		#if _WIN32
			Sleep(ms);
		#endif

		#if __linux__
			usleep(ms * 1000);
		#endif
	}
};