#ifndef EDITOR_H
#define EDITOR_H
#define _CRT_SECURE_NO_WARNINGS
#include "curses.h"
#include <vector>
#include <string>
#include <cctype>

#include "screen.h"
#include "cursor.h"

enum class Mode {
	NORMAL = 0,
	INSERT,
	COMMAND,
	FIND,
	REPLACE,
};

static const int ESCAPE_KEY = '\x1B';
static const int BACKSPACE_KEY = '\x7F';
static const int SPACES_FOR_TAB = 8;
static const int ENTER_KEY = '\xA';
extern std::vector<int> lens; //All lens vector

// The core functionality of Text Editor.
class Editor {
	FILE* file;
	Screen screen;
	Mode current_mode{ Mode::NORMAL };
	std::string line{ create_file_contents() };
	Cursor cursor{};
	std::size_t file_contents_index = 0;
	std::size_t top_of_screen_index = 0;
	std::string copy_buf="";
	std::string copy_search = "";
	std::string open_file = "";

	std::string create_file_contents();
	std::vector<int>get_start_indexes();

	//Handler
	void normal_mode_action(int character);
	void insert_mode_action(int character);
	void command_mode_action(int character);
	void find_mode_action(int character);

	//Nav
	void move_cursor_down();
	void move_cursor_up();
	void move_cursor_left();
	void move_cursor_right();
	void move_cursor_start_line();
	void move_cursor_end_line();
	void page_scroll(char dir);
	void do_w_motion(char dir);
	void find_letter(char dir, int dist);
	void do_nav_command(std::string command);
	void copy_line();
	void delete_word();
	void copy_word();
	void go_line(int line_num);
	int get_one_word(char dir);

	//Insert
	void insert_line();
	void insert_char(int character);
	void delete_line();
	void delete_char();
	void delete_char_after();
	void replace_char(int character);
	void len_inc(int y, int character);
	void len_dec(int y, int character);
	void clear_and_print();

	//FIND
	void find_next(char dir);
	int find(char dir, std::string arg);
	bool do_search_command();

	//Command
	void do_com_command();
	void exit_editor();

	//Help
	void save();

public:
	Editor(const char* file_name = "");

	Editor(const Editor& e) = delete;
	Editor& operator=(const Editor& e) = delete;

	void process_keypress(int character);
	int calc_lens(int to); //calc sum of lens line
	int clen();

	~Editor();
};

#endif
