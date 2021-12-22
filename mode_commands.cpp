#include "editor.h"

void Editor::page_scroll(char dir) {
	int off = cursor.y + cursor.scroll_y_offset;
	if (dir == 'u') {
		if (off > HEIGHT) {
			go_line(off - HEIGHT);
		}
		else
			go_line(1);
	}
	else if (dir == 'd') {
		if (off + HEIGHT < lens.size()) {
			go_line(off + HEIGHT);
		}
		else
			go_line(lens.size());
 	}
}

void Editor::find_letter(char dir, int dist) {
	if (dist)
		dir == 'r' ? move_cursor_left() : move_cursor_right();
	while (!isalnum(line[cursor.row_offset])) {
		int old_x = cursor.row_offset;

		dir == 'r' ? move_cursor_left() : move_cursor_right();

		if (old_x == cursor.row_offset)
			return;
	}
}

void Editor::do_w_motion(char dir) {
	if (dir == 'r') {
		find_letter('l', 1);
		while (isalnum(line[cursor.row_offset]) || line[cursor.row_offset] == '\'') {
			int old_x = cursor.row_offset;
			move_cursor_right();
			if (old_x == cursor.row_offset) {
				find_letter('r', 0);
				return;
			}
		}
		find_letter('r', 0);
	}
	else if (dir == 'l') {
		find_letter('r', 1);
		while (isalnum(line[cursor.row_offset]) || line[cursor.row_offset] == '\'') {
			move_cursor_left();
			if (cursor.row_offset == 0)
				return;
		}
		move_cursor_right();
	}
}

void Editor::insert_line() {
	for (int i = 0; i < copy_buf.length(); ++i) {
		insert_char(copy_buf[i]);
	}
}

void Editor::copy_line() {
	copy_buf = line.substr(cursor.row_offset, clen());
	if (copy_buf.length())
		screen.log = "C: " + copy_buf;
}

void Editor::delete_word() {
	if (!isalnum(line[cursor.row_offset])) {
		screen.log = "Empty!";
		return;
	}

	int left_pos = get_one_word('l');
	int right_pos = get_one_word('r');
	int ost = cursor.row_offset - left_pos;
	
	if (!isspace(line[right_pos]))
		--right_pos;

	for (int i = cursor.row_offset; i < right_pos; ++i) {
		move_cursor_right();
	}

	for (int i = left_pos; i <right_pos; ++i) {
		delete_char();
	}

	for (int i = 0; i < ost; ++i) {
		move_cursor_right();
	}
}

int Editor::get_one_word(char dir) {
	int pos = cursor.row_offset;
	while (isalnum(line[pos]) || line[pos] == '\'') {
		if (dir == 'l') {
			--pos;
			if (pos == -1)
				break;
		}
		else {
			++pos;
			if (pos == calc_lens(lens.size() - 1))
				break;
		}
	}
	return pos;
}

void Editor::copy_word() {
	if (!isalnum(line[cursor.row_offset])) {
		screen.log = "Empty!";
		return;
	}

	int left_pos = get_one_word('l');
	int right_pos = get_one_word('r');

	--right_pos;
	copy_buf = line.substr(left_pos + 1, right_pos - left_pos);

	if (copy_buf.length())
		screen.log = "C: " + copy_buf;
}

void Editor::go_line(int line_num) {
	if (line_num<1 || line_num>lens.size()) {
		screen.log = "Error!";
		return;
	}
	--line_num;
	while (line_num != (cursor.y+cursor.scroll_y_offset)) {
		line_num > (cursor.y + cursor.scroll_y_offset) ? move_cursor_down() : move_cursor_up();
	}
}

void Editor::delete_line() {
	int line_len = clen();
	line.erase(cursor.row_offset, line_len);
	lens.erase(lens.begin() + cursor.y);
}

void Editor::clear_and_print() {
	int line_len = clen();
	lens[cursor.y] = 1;
	line.erase(cursor.row_offset, line_len);
	line.insert(cursor.row_offset, "\n");
}


void Editor::delete_char_after() {
	int old_pos = cursor.row_offset;
	move_cursor_right();
	if (old_pos == cursor.row_offset) return;
	delete_char();
}

void Editor::replace_char(int character) {
	delete_char();
	insert_mode_action(character);

	current_mode = Mode::NORMAL;
	screen.text_mode = "NAV";
}

//Find
void Editor::find_next(char dir) {
	if (copy_search == "") {
		screen.log = "No str!";
		return;
	}
	
	int old_pos = cursor.row_offset;
	if (dir == 'r') {
		copy_search[0] = '/';
		move_cursor_right();
		if (!do_search_command()) {
			if (old_pos != cursor.row_offset)
				move_cursor_left();
		}
	}

	else if (dir == 'l') {
		copy_search[0] = '?';
		move_cursor_left();
		if (!do_search_command()) {
			if (old_pos != cursor.row_offset)
				move_cursor_right();
		}
	}
}

int Editor::find(char dir, std::string arg) {
	int cur = cursor.row_offset;
	if (dir == 'r') {
		int res = line.find(arg, cur);

		if (res == -1) {
			screen.log = "No match";
			return 0;
		}

		if (res > cur) {
			while (cur != res) {
				move_cursor_right();
				++cur;
			}
		}
	}
	else {
		int res = line.find(arg);
		if (res == -1) {
			screen.log = "No match";
			return 0;
		}
		if (res < cur) {
			while (cur != res) {
				move_cursor_left();
				--cur;
			}
		}
	}
	return 1;
}

bool Editor::do_search_command() {
	if (copy_search[0] == '/') {
		return copy_search[1] == ' ' ? find('r', copy_search.substr(2)) : find('r', copy_search.substr(1));
	}
	if (copy_search[0] == '?') {
		return copy_search[1] == ' ' ? find('l', copy_search.substr(2)) : find('l', copy_search.substr(1));
	}
	else {
		screen.log="Error!";
	}
	return 0;
}

void Editor::do_com_command() {
	if (screen.command[0] == 'o') {
		open_file = screen.command.substr(2);
		create_file_contents();
	}
	else if (screen.command == "x") {
		if (!save(open_file)) return;
		open_file = ST_NAME;
		create_file_contents();
	}
	else if (screen.command == "w") {
		save(open_file);
	}
	else if (screen.command[0] == 'w') {
		save(screen.command.substr(2));
	}
	else if (screen.command == "q") {
		exit_editor();
	}
	else if (screen.command == "q!") {
		exit_editor();
	}
	else if (screen.command == "wq!") {
		if (!save(open_file)) return;
		exit_editor();
	}
	else if (isdigit(screen.command[0])) {
		std::string number;
		int i = 0;

		while (isdigit(screen.command[i])) {
			number += screen.command[i];
			++i;
			if (i == screen.command.length())
				break;
		}

		int line_num = atoi(number.c_str());
		go_line(line_num);
	}
	else if (screen.command == "h") {
		screen.log = "Hello in Chechenev TE! Ins mode: i, I, S, A, r. Find mode: /,?. Exit mode: ESC\n";
		screen.log_pos = 0;
	}
	else {
		screen.log = "Error!";
	}
}

bool Editor::save(std::string save_name) {
	std::fstream file;
	file.open(save_name, std::ios_base::out);
	if (!file.is_open()) {
		screen.log = "No such file";
		return 0;
	}
	file << line;
	file.close();
	screen.is_file_modified = false;
	return 1;
}

void Editor::exit_editor() {
	endwin();
	exit(1);
}
