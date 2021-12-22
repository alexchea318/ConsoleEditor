#ifndef SCREEN_H
#define SCREEN_H

#include "curses.h"
#include <vector>
#include <string>

#include "cursor.h"

// All the functionality related to displaying things on the screen.
class Screen {
public:
	std::size_t rows;
	std::size_t cols;
	bool is_file_modified{false};
	std::string text_mode = "NAV";
	std::string command = "";
	std::string log = "";
	std::string file_name;
	int log_pos = 100;

	Screen(const char* file_name) ;
	Screen(const Screen& s) = delete;
	Screen& operator=(const Screen& s) = delete;

	void display(std::string line, const Cursor& cursor, std::vector<int> lens) const ;

	~Screen();
private:
	WINDOW* file_info_bar;

	void move_cursor(const Cursor& cursor) const ;
	
	void draw_file_info_bar(const Cursor& cursor, std::vector<int> lens) const ;
};
#endif
