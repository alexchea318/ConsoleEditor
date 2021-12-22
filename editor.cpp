#include "editor.h"

std::vector<int> lens;

Editor::Editor(const char* file_name) : screen{ file_name } {
	open_file = file_name;
	create_file_contents();

	screen.display(line, cursor, lens);
}

void Editor::create_file_contents() {
	line = "";
	int i = 0;
	//open_file = "hw.txt";

	std::fstream file;
	file.open(open_file, std::ios_base::in);
	if (!file.is_open()) {
		screen.log = "No File";
		line += "Hello!";
		lens.push_back(line.length());
		return;
	}

	while (1) {
		char c = file.get();
		if (file.eof()) break;

		line += c;
		++i;
		if (i == WIDTH || c == '\n') {
			lens.push_back(i);
			i = 0;
		}
	}
	lens.push_back(i);
	file.close();
}

void Editor::process_keypress(int character) {
	switch (current_mode) {
	case Mode::NORMAL:
		normal_mode_action(character);
		break;
	case Mode::INSERT:
		insert_mode_action(character);
		break;
	case Mode::COMMAND:
		command_mode_action(character);
		break;
	case Mode::FIND:
		find_mode_action(character);
		break;
	case Mode::REPLACE:
		replace_char(character);
		break;
	}
	screen.display(line, cursor, lens);

	screen.log.clear();
	screen.log_pos = 100;
}

void Editor::normal_mode_action(int character) {
	if (!screen.command.length()) {
		screen.text_mode = "NAV";

		switch (character) {
			//Extra Exit
		case 'q':
			exit_editor();
			break;

			//Mods
		case 'i':
			current_mode = Mode::INSERT;
			screen.text_mode = "WRT";
			break;
		case 'I':
			current_mode = Mode::INSERT;
			screen.text_mode = "WRT";
			move_cursor_start_line();
			break;
		case 'S':
			current_mode = Mode::INSERT;
			screen.text_mode = "WRT";
			move_cursor_end_line();
			break;
		case 'A':
			current_mode = Mode::INSERT;
			screen.text_mode = "WRT";
			move_cursor_start_line();
			clear_and_print();
			break;
		case 'r':
			current_mode = Mode::REPLACE;
			screen.text_mode = "REP";
			break;
		case ':':
			screen.text_mode = "COM";
			current_mode = Mode::COMMAND;
			break;
		case '/':
			screen.text_mode = "F->";
			current_mode = Mode::FIND;
			break;
		case '?':
			screen.text_mode = "<-F";
			current_mode = Mode::FIND;
			break;

			//Nav simple
		case KEY_RIGHT:
			move_cursor_right();
			break;
		case KEY_UP:
			move_cursor_up();
			break;
		case KEY_DOWN:
			move_cursor_down();
			break;
		case KEY_LEFT:
			move_cursor_left();
			break;
		case KEY_PPAGE:
			page_scroll('u');
			break;
		case KEY_NPAGE:
			page_scroll('d');
			break;
		case 'g':
			go_line(0);
			break;
		case 'G':
			if((lens.size() - HEIGHT)>0)
				go_line(lens.size() - HEIGHT);
			else
				go_line(lens.size() - 1);
			break;

			//Nav special
		case '^':
			move_cursor_start_line();
			break;
		case '$':
			move_cursor_end_line();
			break;
		case 'w':
			do_w_motion('r');
			break;
		case 'b':
			do_w_motion('l');
			break;
		case 'x':
			delete_char_after();
			break;
		case 'p':
			insert_line();
			break;

			//Default: do command
		default:
			screen.text_mode = "NAV_COM";
			screen.command += character;
			break;
		}
	}
	else {
		screen.text_mode = "NAV_COM";
		switch (character) {
			//Comands
		case '\r':
			screen.text_mode = "NAV";
			do_nav_command(screen.command);
			screen.command = "";
			break;
		case '\b':
			screen.command.erase(screen.command.length() - 1, 1);
			break;
		default:
			screen.command += character;
			break;
		}
	}
}

void Editor::do_nav_command(std::string command) {
	if (command == "y") {
		move_cursor_start_line();
		copy_line();
	}
	else if (command == "yw")
		copy_word();
	else if (command == "diw") {
		delete_word();
	}
	else if (command == "dd") {
		move_cursor_start_line();
		copy_line();
		delete_line();
	}
	else if (command.find('G') != -1) {
		std::string number;
		for (int i = 0; command[i] != 'G'; ++i) {
			if (!isdigit(command[i]))
				return;
			else
				number += command[i];
		}
		int line_num = atoi(number.c_str());
		go_line(line_num);
	}
	else {
		screen.log = "Error!";
	}
}

int Editor::calc_lens(int to) {
	int sum = 0;
	for (int i = 0; i <= to; ++i) {
		sum += lens[i];
	}
	return sum;
}

int Editor::clen() {
	return lens[cursor.y];
}

//Cursor nav start
void Editor::move_cursor_start_line() {
	cursor.row_offset -= cursor.x;
	cursor.x = 0;
}

void Editor::move_cursor_end_line() {
	cursor.row_offset += ((clen() - 1) - cursor.x);
	cursor.x = clen() - 1;
}

void Editor::move_cursor_right() {
	if (cursor.y == lens.size() - 1 && cursor.x == lens[lens.size() - 1] - 1) {
		return;
	}

	if (line[cursor.row_offset] != '\n' && cursor.x < WIDTH - 1)
	{
		++cursor.x;
		++cursor.row_offset;
	}
	else {
		cursor.x = 0;
		++cursor.row_offset;

		//Short_down
		if (cursor.y < lens.size() - 1) {
			cursor.y++;
		}
	}
}

void Editor::move_cursor_up() {
	if (cursor.y > 0) {
		cursor.y--;

		//Scroll
		if (cursor.y > HEIGHT) {
			screen.scroll_offset = calc_lens(cursor.y - HEIGHT);
		}
		else {
			screen.scroll_offset = 0;
		}

		//Calc offset
		if (lens[cursor.y] == WIDTH) {
			cursor.row_offset -= WIDTH;
		}
		else {
			cursor.row_offset -= lens[cursor.y];
		}

		//If <
		if (lens[cursor.y] < cursor.x + 1) {
			while (lens[cursor.y] != cursor.x + 1) {
				--cursor.x;
				--cursor.row_offset;
			}
		}
	}
}

void Editor::move_cursor_down() {
	if (cursor.y < lens.size() - 1) {
		cursor.y++;

		//Scroll
		if (cursor.y > HEIGHT) {
			screen.scroll_offset = calc_lens(cursor.y- HEIGHT);
		}
		else {
			screen.scroll_offset = 0;
		}

		//Offset calc
		if (lens[cursor.y - 1] == WIDTH) {
			cursor.row_offset += WIDTH;
		}
		else {
			cursor.row_offset += lens[cursor.y - 1];
		}

		//If <
		if (lens[cursor.y] < cursor.x + 1) {
			while (lens[cursor.y] != cursor.x + 1) {
				--cursor.x;
				--cursor.row_offset;
			}
		}
	}
}

void Editor::move_cursor_left() {
	if (cursor.x > 0) {
		--cursor.x;
		--cursor.row_offset;
	}
	else {
		//Short_up
		if (cursor.y > 0) {
			cursor.y--;
			cursor.x = lens[cursor.y] - 1;
			cursor.row_offset--;
		}
	}
}
//Cursor nav end


void Editor::insert_mode_action(int character) {
	screen.is_file_modified = true;
	switch (character) {
	case ESCAPE_KEY:
		current_mode = Mode::NORMAL;
		screen.text_mode = "NAV";
		break;
	case '\b':
		delete_char();
		break;
	case '\r':
		//insert_char('\n');
		break;
		//Nav simple
	case KEY_RIGHT:
		move_cursor_right();
		break;
	case KEY_UP:
		move_cursor_up();
		break;
	case KEY_DOWN:
		move_cursor_down();
		break;
	case KEY_LEFT:
		move_cursor_left();
		break;

	default:
		if (character > 255) return;
		if (std::isprint(character) || character == '\t') {
			insert_char(character);
		}
		break;
	}
}

//Main part of navigation
std::vector<int> Editor::get_start_indexes() {
	std::vector<int> indexes = { 0 };
	int sum = 0;
	for (int i = 0; i < lens.size() - 1; ++i) {
		sum += lens[i];
		indexes.push_back(sum);
	}
	return indexes;
}

void Editor::len_inc(int y, int character) {
	while (lens[y] == WIDTH) {
		++y;
	}
	++lens[y];
	if (lens[y] == WIDTH) {
		lens.insert(lens.begin() + (y + 1), 0);
	}
}

void Editor::len_dec(int y, int character) {
	while (lens[y] == WIDTH) {
		++y;
	}
	if (character != '\n') {
		--lens[y];
	}
	else {
		//Delete '\n'
		int free_space = (WIDTH - 1) - cursor.x;
		lens[y] = WIDTH;
		while (lens[y] == WIDTH) {
			++y;
		}
		lens[y] -= free_space;
	}
	if (lens[y] == 0) {
		lens.erase(lens.begin() + y);
	}
}

void Editor::insert_char(int character) {
	len_inc(cursor.y, character);
	char str[] = "\0\0";
	str[0] = character;
	line.insert(cursor.row_offset, str);
	move_cursor_right();
}

void Editor::delete_char() {
	len_dec(cursor.y, line[cursor.row_offset]);
	line.erase(cursor.row_offset, 1);
	move_cursor_left();
}
//End part

void Editor::command_mode_action(int character) {
	if (character == ESCAPE_KEY) {
		current_mode = Mode::NORMAL;
		screen.text_mode = "NAV";
		screen.command = "";
		return;
	}

	switch (character) {
		//Comands
	case '\r':
		do_com_command();
		screen.command = "";
		break;
	case '\b':
		screen.command.erase(screen.command.length() - 1, 1);
		break;
	default:
		screen.command += character;
		break;
	}
}

void Editor::find_mode_action(int character) {
	if (character == ESCAPE_KEY) {
		current_mode = Mode::NORMAL;
		screen.text_mode = "NAV";
		screen.command = "";
		return;
	}

	if (!screen.command.length()) {
		switch (character) {
		case 'n':
			find_next('r');
			return;
		case 'N':
			find_next('l');
			return;
		case '/':
			screen.text_mode = "F->";
			screen.command += character;
			return;
		case '?':
			screen.text_mode = "<-F";
			screen.command += character;
			return;
		}

	}
	else {
		switch (character) {
			//Comands
		case '\r':
			copy_search = screen.command;
			do_search_command();
			screen.log = "S: " + screen.command;
			screen.command = "";
			break;
		case '\b':
			screen.command.erase(screen.command.length() - 1, 1);
			break;
		default:
			screen.command += character;
			break;
		}
	}
}

Editor::~Editor() {
	//nothing
}

