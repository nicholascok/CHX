#include "chx.h"
#include "chx_defaults.c"
#include "config.h"

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
	fwrite(CINST.fdata.data, 1, CINST.fdata.len, outf);
	fclose(outf);
}

long min(long _a, long _b) {
	if (_a < _b) return _a;
	return _b;
}

long max(long _a, long _b) {
	if (_a > _b) return _a;
	return _b;
}

void chx_draw_contents() {
	// print header
	printf("\033[0;0H%-*c\e[1;34m", CINST.row_num_len + 1, ' ');
	for (int i = 0; i < CINST.bytes_per_row / CINST.bytes_in_group; i++)
		printf(" %02X%-*c", i * CINST.bytes_in_group, CINST.bytes_in_group * 2 - 1, ' ');
	
	// print row numbers
	for (int i = 0; i < CINST.num_rows - BPD; i++)
		printf("\033[%d;0H %0*X ", i + TPD + 1, CINST.row_num_len, i * CINST.bytes_per_row + CINST.scroll_pos);
	
	printf("\e[0m");
	
	// print main contents
	for (int i = CINST.scroll_pos; i < CINST.scroll_pos + CINST.num_bytes; i++) {
		if (!(i % CINST.bytes_per_row))
			cur_set(CINST.row_num_len + 2, (i - CINST.scroll_pos) / CINST.bytes_per_row + TPD);
		else if (!(i % CINST.bytes_in_group))
			printf("  ");
		if (i < CINST.fdata.len) {
			if (CINST.style_data[i / 8] & (0x80 >> (i % 8)))
				printf("\e[1;33%02X\e[0m", CINST.fdata.data[i]);
			else
				if (CINST.sel_start != CINST.sel_end && BETWEEN(i, CINST.sel_start, CINST.sel_end)) printf("\e[7m%02X\e[0m", CINST.fdata.data[i]);
				else printf("%02X", CINST.fdata.data[i]);
		} else
			printf("..");
	}
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_GET_CURSOR_Y);
	
	fflush(stdout);
}

void chx_main() {
	// draw content
	chx_draw_contents();
	char SELECTING = 0;
	
	// main loop
	for(struct chx_key key;; key = chx_get_key()) {
		// control keys are global
		if (chx_keybinds_global[WORD(key)]) chx_keybinds_global[WORD(key)]();
		if (CINST.selecting && key.type != 0x03) chx_finish_selection();
		if (!CINST.selecting && CINST.cursor.pos != CINST.sel_end - 1) chx_clear_selection();
		
		// keys have different actions depending on active mode setting
		switch (CINST.mode) {
			default:
			case CHX_MODE_DEFAULT:
				if (chx_keybinds_mode_command[WORD(key)]) chx_keybinds_mode_command[WORD(key)]();
				break;
			case CHX_MODE_TYPE_HEXCHAR:
				if (IS_CHAR_HEX(WORD(key))) chx_type_hexchar(WORD(key));
				else if (WORD(key) == 0x7F) chx_backspace_hexchar();
				break;
		}
	}
}

struct chx_key chx_get_key() {
		struct chx_key key;
        char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
		// set terminal flags and enter raw mode
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
		
        read(0, &buf, 8);
		
		// check the type of key press (ALT, ESCAPE, etc.)
		switch (buf[0]) {
			case 0x1B:
				if (buf[5]) key = (struct chx_key) {buf[5], 0x03};
				else if (buf[1] == 0x5B || !buf[1]) key = (struct chx_key) {buf[2], 0x02};
				else key = (struct chx_key) {buf[1], 0x01};
				break;
			default:
				key = (struct chx_key) {buf[0], 0};
				break;
		}
		//printf("| %X %X %X %X %X %X %X |\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], key);
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
		
        return key;
}

struct chx_key chx_get_char() {
		struct chx_key key;
		
		// set terminal flags and enter raw mode
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
		
        read(0, &key, 1);
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
		
        return key;
}

int main(int arc, char** argv) {
	// enter new terminal state
	tenter();
	
	// get window dimensions
	struct winsize size;
	ioctl(0, TIOCGWINSZ, (char*) &size);
	
	// load file
	struct chx_finfo hdata = chx_import(argv[1]);
	hdata.filename = argv[1];
	
	// setup initial instance
	CINST.fdata = hdata;
	CINST.style_data = calloc(1, hdata.len / 8 + (hdata.len % 8 != 0));
	CINST.height = size.ws_row;
	CINST.width = size.ws_col;
	CINST.x_offset = 0;
	CINST.y_offset = 0;
	CINST.bytes_per_row = 16;
	CINST.bytes_in_group = 1;
	CINST.num_rows = hdata.len / CINST.bytes_per_row;
	CINST.row_num_len = 8;
	CINST.num_rows = size.ws_row - PD;
	CINST.num_bytes = CINST.num_rows * CINST.bytes_per_row;
	CINST.scroll_pos = 0;
	CINST.mode = 0;
	CINST.saved = 1;
	CINST.selecting = 0;
	
	// initialize cursor
	CINST.cursor = (struct CHX_CURSOR) {0, 0};
	
	// call main loop
	chx_main();
	
	// after exiting ask if user would like to save
	chx_quit();
	
	// restore terminal state
	texit();
}