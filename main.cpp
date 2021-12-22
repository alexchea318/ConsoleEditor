#include "editor.h"

void run_editor(const char* file_name) {
	Editor e{file_name};	

	int character;
	while (true) {
		character = getch();
		e.process_keypress(character);
	}
}

int main(int argc, char** argv) {
	if (argc > 1) {
		run_editor(argv[1]);
	}
	else run_editor("hw.txt");
}
