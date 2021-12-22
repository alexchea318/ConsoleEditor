#include "screen.h"

// Legal color pair values are in the range 1 to COLOR_PAIRS - 1, inclusive.
// That is why the color here starts at 1 and not 0.
static const int FILE_BAR_COLOR = 1;

Screen::Screen(const char* file_name)  
: rows{}, cols{}, file_name{file_name}, file_info_bar{} 
{
	initscr();
	start_color();
	init_pair(FILE_BAR_COLOR, COLOR_BLACK, COLOR_WHITE);
	keypad(stdscr, 1); //!!!!

	noecho();
	raw();
	getmaxyx(stdscr, rows, cols);
	scrollok(stdscr, true);

	rows -= 2;
	file_info_bar = newwin(1, cols, rows, 0);
}

void Screen::display(std::string line, const Cursor& cursor, std::vector<int> lens) const 
{
	clear();
	std::size_t i = 0;

	printw("%s", line.substr(scroll_offset).c_str());
	/*for (; i < rows; ++i) {
		printw("~");
		if (i != rows - 1) {
			printw("\n");
		}
	}*/
	refresh();
	draw_file_info_bar(cursor, lens);
	move_cursor(cursor);
}

void Screen::draw_file_info_bar(const Cursor& cursor, std::vector<int> lens) const  {
	wbkgd(file_info_bar, COLOR_PAIR(FILE_BAR_COLOR));
	wclear(file_info_bar);

	mvwprintw(file_info_bar, 0, 1, "%s, MOD: %s, LINE: %d of %d, POS: %d.%d of %d, INDEX: %d", 
		file_name.c_str(), text_mode.c_str(), cursor.y + 1, lens.size(), cursor.y, cursor.x, lens[cursor.y]-1, cursor.row_offset);

	if (is_file_modified) {
		mvwprintw(file_info_bar, 0, 0, "*");
	}

	if (text_mode!="INS") {
		mvwprintw(file_info_bar, 0, 80, "Command: %s", command.c_str());

		if (log.length())
			mvwprintw(file_info_bar, 0, log_pos, "Log: %s", log.c_str());
	}
	
	wrefresh(file_info_bar);
}

void Screen::move_cursor(const Cursor& cursor) const  {
	move(cursor.y, cursor.x);		
}

Screen::~Screen() {
	delwin(file_info_bar);
	endwin();
}
