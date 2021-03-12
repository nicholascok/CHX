#include "chx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

#define enter() system("tput smcup")
#define exit() system("tput rmcup")
#define cls() system("clear")
#define cur_set(X, Y) printf("\033[%ld;%ldH", Y + 1, X + 1)

struct chx_finfo chx_import(char* fpath) {
	struct chx_finfo finfo;
	FILE* inf = fopen(fpath, "r+b");
	fseek(inf, 0, SEEK_END);
	long flen = ftell(inf);
	rewind(inf);
	finfo.len = flen;
	finfo.data = malloc(flen);
	fread(finfo.data, 1, flen, inf);
	fclose(inf);
	return finfo;
}

void chx_export(char* fpath) {
	FILE* outf = fopen(fpath, "w+b");
	fwrite(CHXGC.instances[CHXGC.sel_instance].data, 1, CHXGC.instances[CHXGC.sel_instance].len, outf);
	fclose(outf);
}

void chx_save_dialoge() {
	// setup user input buffer
	char usrin[256];
	
	// print save dialoge and recieve user input
	cur_set(0, CHXGC.theight);
	printf("SAVE AS? (LEAVE EMPTY TO CANCEL): ");
	fflush(stdout);
	
	fgets(usrin, 256, stdin);
	
	// cut input at first newline
	usrin[strcspn(usrin, "\n")] = 0;
	
	// only export if filename was entered
	if (usrin[0]) chx_export(usrin);
	
	// erase save dialoge
	printf("\033[1A\033[2K");
	fflush(stdout);
	
	// redraw contents
	chx_draw_contents();
}

int chx_count_digits(long num) {
	int count = 1;
	while (num > 15) {
		num /= 16;
		count++;
	}
	return count;
}

void chx_draw_contents() {
	// save sursor position
	int tmp_curx = CHXCUR.x, tmp_cury = CHXCUR.y;
	
	// reset cursor
	cur_set(0, 0);
	
	// print header
	printf("%-*c", CHXGC.num_digits + 1, ' ');
	for (int i = 0; i < CHXGC.bytes_per_row; i++) printf(" \e[1;34m%02X \e[m", i);
	
	// print row numbers
	for (int i = CHXGC.section_start; i < CHXGC.rows_in_section + CHXGC.section_start + 1; i++) printf("\e[1;34m\033[%d;0H %0*X \e[m", i - CHXGC.section_start + 2, CHXGC.num_digits, i * CHXGC.bytes_per_row);
	
	// print main contents
	for (int j = CHXGC.section_start; j < CHXGC.section_start + CHXGC.rows_in_section + 1; j++)
		for (int i = 0; i < CHXGC.bytes_per_row; i++)
			printf("\033[%d;%dH %02X", j - CHXGC.section_start + 2, CHXGC.num_digits + 2 + i * 4, CHXGC.instances[CHXGC.sel_instance].data[j * CHXGC.bytes_per_row + i]);
	
	// print last row of bytes
	if (CHXGC.num_rows == CHXGC.section_start + CHXGC.rows_in_section - 1) for (int i = 0; i < CHXGC.bytes_in_last_row; i++) printf("\033[%d;%dH %02X", CHXGC.rows_in_section + 1, CHXGC.num_digits + 2 + i * 4, CHXGC.instances[CHXGC.sel_instance].data[(CHXGC.rows_in_section - 1) * CHXGC.bytes_per_row + i]);
	
	// restore cursor position
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4 + CHXCUR.sbpos, CHXCUR.y + 1 - CHXGC.section_start);
	
	fflush(stdout);
}

void chx_cursor_move_up() {
	CHXCUR.y -= CHXCUR.y > 0;
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4 + CHXCUR.sbpos, CHXCUR.y + 1 - CHXGC.section_start);
		if (CHXCUR.y < CHXGC.section_start) {
			CHXGC.section_start--;
			chx_draw_contents();
		}
}

void chx_cursor_move_down() {
	if (!(CHXCUR.y >= CHXGC.rows_in_section - 2 && CHXCUR.x >= CHXGC.bytes_in_last_row && CHXGC.section_start + CHXGC.theight - 1 > CHXGC.num_rows)) {
		CHXCUR.y = (CHXCUR.y >= CHXGC.num_rows) ? 0 : CHXCUR.y + 1;
		cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4 + CHXCUR.sbpos, CHXCUR.y + 1 - CHXGC.section_start);
		if (CHXCUR.y - CHXGC.section_start > CHXGC.theight - 1) {
			CHXGC.section_start++;
			chx_draw_contents();
		}
	}
}

void chx_cursor_move_right() {
	char is_end = (CHXCUR.y >= CHXGC.rows_in_section - 1 && CHXCUR.x >= CHXGC.bytes_in_last_row - 1 && CHXGC.section_start + CHXGC.theight - 1 > CHXGC.num_rows);
	if (CHXCUR.sbpos < 1) CHXCUR.sbpos++;
	else if (!is_end) {
		CHXCUR.sbpos = 0;
		CHXCUR.x = (CHXCUR.x >= CHXGC.bytes_per_row - 1) ? 0 : CHXCUR.x + 1;
	}
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4 + CHXCUR.sbpos, CHXCUR.y + 1 - CHXGC.section_start);
}

void chx_cursor_move_left() {
	if (CHXCUR.sbpos > 0) CHXCUR.sbpos--;
	else {
		if (CHXCUR.x) CHXCUR.sbpos = 1;
		CHXCUR.x -= (CHXCUR.x) && 1;
	}
}

void chx_main() {
	// draw content
	chx_draw_contents();
	
	// main loop
	for(char key;; key = chx_getch()) {
		// mode setting and movement keys are global
		switch (key) {
			case '\033': // ANSI escape sequence
				chx_getch(); // skip '[' char
				switch (chx_getch()) { // read escape char
					case KEY_UP:
						chx_cursor_move_up();
						break;
					case KEY_DOWN:
						chx_cursor_move_down();
						break;
					case KEY_RIGHT:
						chx_cursor_move_right();
						break;
					case KEY_LEFT:
						chx_cursor_move_left();
						break;
				}
				
				// update cursor
				cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4 + CHXCUR.sbpos, CHXCUR.y + 1 - CHXGC.section_start);
				
				fflush(stdout);
				break;
			case CTRL('i'): // toggle between command and type mode
				if (CHXGC.mode == CHX_MODE_TYPE_HEXCHAR) CHXGC.mode = CHX_MODE_DEFAULT;
				else CHXGC.mode = CHX_MODE_TYPE_HEXCHAR;
				break;
			case CTRL('w'): // save
				chx_export(CHXGC.instances[CHXGC.sel_instance].filename);
				
				// remove unsaved data highlight
				chx_draw_contents();
				break;
			case CTRL('e'): // export
				chx_save_dialoge();
				break;
			case CHX_CTRL('x'): // quit
				return;
			default:
				// keys have different actions depending on active mode setting
				switch (CHXGC.mode) {
					default:
					case CHX_MODE_DEFAULT:
						switch (key) {
							case '\033': // ANSI escape sequence
								chx_getch(); // skip '[' char
								switch (chx_getch()) { // read escape char
									default:
										break;
								}
								break;
							case 'q': // quit
								return;
							case 's': // save
								chx_export(CHXGC.instances[CHXGC.sel_instance].filename);
								
								// remove unsaved data highlight
								chx_draw_contents();
								break;
							case 'w': // save as
								chx_save_dialoge();
								break;
						}
						break;
					case CHX_MODE_TYPE_DEFAULT:
						break;
					case CHX_MODE_TYPE_HEXCHAR:
						if (IS_HEX_CHAR(key) || IS_DIGIT(key)) { // only accept hex characters
							if ((key ^ 0x60) < 7) key -= 32; // ensure everything is upper-case
							printf("\033[1;35m%c\033[0m\033[1D", key); // print the character on the screen
							
							// update stored file data
							CHXGC.instances[CHXGC.sel_instance].data[CHXCUR.y * CHXGC.bytes_per_row + CHXCUR.x] &= 0xF0 << ((1 - CHXCUR.sbpos) * 4);
							CHXGC.instances[CHXGC.sel_instance].data[CHXCUR.y * CHXGC.bytes_per_row + CHXCUR.x] |= ((char) strtol(&key, NULL, 16)) << ((1 - CHXCUR.sbpos) * 4);
							
							fflush(stdout);
							
							// move sursor after typing a char
							// if the cursor is in a sub position, add to the sub position of the cursor, else change the selected byte
							if (CHXCUR.sbpos < 1) CHXCUR.sbpos++;
							else {
								// set if the cursor is sttempting to move past the end of the file
								char is_end = (CHXCUR.y >= CHXGC.rows_in_section - 1 && CHXCUR.x >= CHXGC.bytes_in_last_row - 1 && CHXGC.section_start + CHXGC.theight - 1 > CHXGC.num_rows);
								CHXCUR.sbpos = 0;
								if (is_end) CHXCUR.sbpos = 1;
								else if (CHXCUR.x >= CHXGC.bytes_per_row - 1) {
									CHXCUR.x = 0;
									CHXCUR.y++;
								} else CHXCUR.x++;
							}
							
							// update cursor position on screen
							cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4 + CHXCUR.sbpos, CHXCUR.y + 1 - CHXGC.section_start);
							
							fflush(stdout);
						}
						break;
				}
				break;
		}
	}
}

int main(int arc, char** argv) {
	// enter new terminal state
	enter();
	
	// get window dimensions
	struct winsize size;
	ioctl(0, TIOCGWINSZ, (char*) &size);
	
	// load file
	struct chx_finfo hdata = chx_import(argv[1]);
	hdata.filename = argv[1];
	
	// setup global variables
	CHXGC.mode = 0;
	CHXGC.instances = malloc(sizeof(struct chx_finfo));
	CHXGC.instances[0] = hdata;
	CHXGC.num_instances = 1;
	CHXGC.sel_instance = 0;
	CHXGC.theight = size.ws_row;
	CHXGC.twidth = size.ws_col;
	CHXGC.bytes_per_row = 16;
	CHXGC.bytes_in_group = 2;
	CHXGC.bytes_in_last_row = hdata.len % CHXGC.bytes_per_row;
	CHXGC.num_rows = (hdata.len - CHXGC.bytes_in_last_row) / CHXGC.bytes_per_row;
	CHXGC.num_digits = chx_count_digits(CHXGC.num_rows);
	CHXGC.rows_in_section = (CHXGC.num_rows < size.ws_row) ? CHXGC.num_rows + 1 : size.ws_row - 2;
	CHXGC.section_start = 1;
	
	// initialize cursor
	CHXCUR = (struct CHX_CURSOR) {0, 1, 0, 0};
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4, CHXCUR.y + 1 - CHXGC.section_start);
	fflush(stdout);
	
	lb_return:
	
	// call main loop
	chx_main();
	
	// after exiting ask if user would like to save
	cur_set(0, CHXGC.theight);
	printf("WOULD YOU LIKE TO SAVE? (Y / N): ");
	
	switch (fgetc(stdin)) {
		case 'y':
		case 'Y':
			chx_export(argv[1]);
			break;
		default:
			cls();
			goto lb_return;
			break;
		case 'n':
		case 'N':
			break;
	}
	
	// restore terminal state
	exit();
}

char chx_getch() {
        char buf = 0;
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
        read(0, &buf, 1);
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
        return buf;
}