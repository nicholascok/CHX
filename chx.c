#include "chx.h"
#include "chx_defaults.c"
#include "config.h"

struct chx_finfo chx_import(char* fpath) {
	struct chx_finfo finfo;
	FILE* inf = fopen(fpath, "r+b");
	if (!inf) {
		printf("file \"%s\" not found.\n", fpath);
		exit(0);
	}
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

void chx_redraw_line(int byte) {
	// calculate line number
	int line_start = (byte / CINST.bytes_per_row) * CINST.bytes_per_row;
	
	// print row number
	printf("\e[1;34m\e[%d;0H%0*X \e[0m", (int) (byte - CINST.scroll_pos) / CINST.bytes_per_row + TPD + 1, CINST.row_num_len, line_start);
	
	// print row contents
	cur_set(CINST.row_num_len + CINST.group_spacing, (int) (byte - CINST.scroll_pos) / CINST.bytes_per_row + TPD);
	for (int i = line_start; i < line_start + CINST.bytes_per_row; i++) {
		if (i % CINST.bytes_per_row && !(i % CINST.bytes_in_group) && CINST.group_spacing != 0)
			printf("%-*c", CINST.group_spacing, ' ');
		if (i < CINST.fdata.len) {
			if (CINST.selected && BETWEEN(i, CINST.sel_start, CINST.sel_stop))
				printf("\e[7m%02X\e[0m", CINST.fdata.data[i]);
			else if (CINST.style_data[i / 8] & (0x80 >> (i % 8)))
				printf("\e[1;33m%02X\e[0m", CINST.fdata.data[i]);
			else
				printf("%02X", CINST.fdata.data[i]);
		} else
			printf("..");
	}
	
	// draw ascii preview
	cur_set(CHX_CONTENT_END, (int) (byte - CINST.scroll_pos) / CINST.bytes_per_row + TPD);
	for (int i = line_start; i < line_start + CINST.bytes_per_row; i++) {
		if (i == CINST.cursor.pos)
			printf("\e[7m");
		else if (i == CINST.cursor.pos + 1)
			printf("\e[0m");
		if (i < CINST.fdata.len) {
			if (IS_PRINTABLE(CINST.fdata.data[i]))
				printf("%c", CINST.fdata.data[i]);
			else
				printf("·");
		} else
			printf("•");
	}
	printf("\e[0m");
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_print_status() {
	// print current mode setting
	cur_set(0, CINST.height);
	switch (CINST.mode) {
		case CHX_MODE_DEFAULT:
			printf("\e[2K[ COMMAND ]");
			break;
		case CHX_MODE_INSERT:
			printf("\e[2K[ INSERT ]");
			break;
		case CHX_MODE_REPLACE:
			printf("\e[2K[ REPLACE ]");
			break;
		default:
			printf("\e[2K[ UNKNOWN ]");
			break;
	}
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	
	fflush(stdout);
}

void chx_draw_extra() {
	if (CINST.cursor.pos < CINST.fdata.len) {
		cur_set(CHX_SIDEBAR_END, TPD);
		printf("\e[1m");
		printf("\e[0Kbinary: "BINARY_PATTERN, BYTE_TO_BINARY(CINST.fdata.data[CINST.cursor.pos]));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kint8: %i", INT8_AT(CINST.fdata.data, CINST.cursor.pos));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kint16: %i", (CINST.endianness) ? INT16_AT(CINST.fdata.data, CINST.cursor.pos) : __bswap_16 (INT16_AT(CINST.fdata.data, CINST.cursor.pos)));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kint32: %i", (CINST.endianness) ? INT32_AT(CINST.fdata.data, CINST.cursor.pos) : __bswap_32 (INT32_AT(CINST.fdata.data, CINST.cursor.pos)));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kint64: %li", (CINST.endianness) ? INT64_AT(CINST.fdata.data, CINST.cursor.pos) : __bswap_64 (INT64_AT(CINST.fdata.data, CINST.cursor.pos)));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kuint8: %u", UINT8_AT(CINST.fdata.data, CINST.cursor.pos));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kuint16: %u", (CINST.endianness) ? UINT16_AT(CINST.fdata.data, CINST.cursor.pos) : __bswap_16 (UINT16_AT(CINST.fdata.data, CINST.cursor.pos)));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kuint32: %u", (CINST.endianness) ? UINT32_AT(CINST.fdata.data, CINST.cursor.pos) : __bswap_32 (UINT32_AT(CINST.fdata.data, CINST.cursor.pos)));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kuint64: %lu", (CINST.endianness) ? UINT64_AT(CINST.fdata.data, CINST.cursor.pos) : __bswap_64 (UINT64_AT(CINST.fdata.data, CINST.cursor.pos)));
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		if (IS_PRINTABLE(CINST.fdata.data[CINST.cursor.pos]))
			printf("\e[0KANSI char: %c", CINST.fdata.data[CINST.cursor.pos]);
		else
			printf("\e[0KANSI char: \ufffd");
		printf("\e[%dG\e[1B ", CHX_SIDEBAR_END);
		printf("\e[0Kwide char: %lc", WCHAR_AT(CINST.fdata.data, CINST.cursor.pos));
		printf("\e[%dG\e[1B\e[0K\e[1B ", CHX_SIDEBAR_END);
		if (CINST.endianness) printf("\e[0K[LITTLE ENDIAN]");
		else printf("\e[0K[BIG ENDIAN]");
		printf("\e[%dG\e[1B\e[0K\e[0m", CHX_SIDEBAR_END);
	}
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	
	fflush(stdout);
}

void chx_draw_contents() {
	// print header
	printf("\e[0;0H%-*c\e[1;34m", CINST.row_num_len + CINST.group_spacing, ' ');
	if (CINST.group_spacing != 0)
		for (int i = 0; i < CINST.bytes_per_row / CINST.bytes_in_group; i++)
			printf("%02X%-*c", i * CINST.bytes_in_group, CINST.bytes_in_group * 2 + CINST.group_spacing - 2, ' ');
	else 
		for (int i = 0; i < CINST.bytes_per_row / CINST.bytes_in_group; i++)
			printf("%02X", i * CINST.bytes_in_group);
	
	// print row numbers
	for (int i = 0; i < CINST.num_rows; i++)
		printf("\e[%d;0H%0*lX", i + TPD + 1, CINST.row_num_len, i * CINST.bytes_per_row + CINST.scroll_pos);
	
	printf("\e[0m");
	
	// print main contents
	for (int i = CINST.scroll_pos; i < CINST.scroll_pos + CINST.num_bytes; i++) {
		if (!(i % CINST.bytes_per_row))
			cur_set(CINST.row_num_len + CINST.group_spacing, (int) (i - CINST.scroll_pos) / CINST.bytes_per_row + TPD);
		else if (!(i % CINST.bytes_in_group) && CINST.group_spacing != 0)
			printf("%-*c", CINST.group_spacing, ' ');
		if (i < CINST.fdata.len) {
			if (CINST.selected && BETWEEN(i, CINST.sel_start, CINST.sel_stop))
				printf("\e[7m%02X\e[0m", CINST.fdata.data[i]);
			else if (CINST.style_data[i / 8] & (0x80 >> (i % 8)))
				printf("\e[1;33m%02X\e[0m", CINST.fdata.data[i]);
			else
				printf("%02X", CINST.fdata.data[i]);
		} else
			printf("..");
	}
	
	chx_draw_sidebar();
	chx_draw_extra();
}

void chx_draw_sidebar() {
	for (int i = CINST.scroll_pos; i < CINST.scroll_pos + CINST.num_bytes; i++) {
		if (i == CINST.cursor.pos)
			printf("\e[7m");
		else if (i == CINST.cursor.pos + 1)
			printf("\e[0m");
		if (!(i % CINST.bytes_per_row))
			cur_set(CHX_CONTENT_END, (int) (i - CINST.scroll_pos) / CINST.bytes_per_row + TPD);
		if (i < CINST.fdata.len) {
			if (IS_PRINTABLE(CINST.fdata.data[i]))
				printf("%c", CINST.fdata.data[i]);
			else
				printf("·");
		} else
			printf("•");
	}
	printf("\e[0m");
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_prompt_command() {
	// setup user input buffer
	char usrin[256];
	
	// command interpreter recieve user input
	cur_set(0, CINST.height);
	printf("\e[2K: ");
	fflush(stdout);
	
	fgets(usrin, 256, stdin);
	
	// null terminate input at first newline
	str_terminate_at(usrin, '\n');
	
	// lookup entered command and execute procedure
	for (int i = 0; chx_commands[i].str; i++)
		if (cmp_str(chx_commands[i].str, usrin))
			chx_commands[i].execute();
	
	// redraw contents
	chx_print_status();
	chx_draw_contents();
}

void chx_main() {
	// draw content
	chx_draw_contents();
	chx_print_status();
	
	// main loop
	for(struct chx_key key;; key = chx_get_key()) {
		// control keys are global
		if (chx_keybinds_global[WORD(key)]) chx_keybinds_global[WORD(key)]();
		if (CINST.selected && CINST.cursor.pos != CINST.sel_stop) chx_clear_selection();
		
		// keys have different actions depending on active mode setting
		switch (CINST.mode) {
			default:
			case CHX_MODE_DEFAULT:
				if (chx_keybinds_mode_command[WORD(key)]) chx_keybinds_mode_command[WORD(key)]();
				break;
			case CHX_MODE_INSERT:
				if (IS_CHAR_HEX(WORD(key))) chx_type_hexchar(WORD(key));
				else if (WORD(key) == 0x7F) chx_backspace_hexchar();
				break;
			case CHX_MODE_REPLACE:
				if (IS_CHAR_HEX(WORD(key))) chx_set_hexchar(WORD(key));
				else if (WORD(key) == 0x7F) chx_delete_hexchar();
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
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
		
        return key;
}

char chx_get_char() {
		char buf;
		
		// set terminal flags and enter raw mode
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
		
        read(0, &buf, 1);
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
		
        return buf;
}

int main(int argc, char** argv) {
	// allow printinf og wide chars
	setlocale(LC_ALL, "");
	
	// command-line interface
	if (argc < 2) {
		printf("no filepath specified: see chx --help for more information.\n");
		return 0;
	} else {
		if (cmp_str(argv[1], "--help") || cmp_str(argv[1], "-help")) {
			printf("usage: chx <filepath>\n");
			return 0;
		}
		else if (cmp_str(argv[1], "-v") || cmp_str(argv[1], "--version")) {
			printf("CAOIMH HEX EDITOR version 1.0.0\n");
			return 0;
		}
	}
	
	// get window dimensions
	struct winsize size;
	ioctl(0, TIOCGWINSZ, (char*) &size);
	
	// load file
	struct chx_finfo hdata = chx_import(argv[1]);
	hdata.filename = argv[1];
	
	// enter new terminal state
	tenter();
	cls();
	
	// setup initial instance
	CINST.fdata = hdata;
	CINST.style_data = calloc(1, hdata.len / 8 + (hdata.len % 8 != 0));
	CINST.height = size.ws_row;
	CINST.width = size.ws_col;
	CINST.x_offset = 0;
	CINST.y_offset = 0;
	CINST.bytes_per_row = CHX_BYTES_PER_ROW;
	CINST.bytes_in_group = CHX_BYTES_IN_GROUP;
	CINST.group_spacing = CHX_GROUP_SPACING;
	CINST.row_num_len = CHX_ROW_NUM_LEN;
	CINST.num_rows = size.ws_row - PD;
	CINST.num_bytes = CINST.num_rows * CINST.bytes_per_row;
	CINST.saved = 1;
	
	// initialize cursor
	CINST.cursor = (struct CHX_CURSOR) {0, 0};
	
	// call main loop
	chx_main();
	
	// after exiting ask if user would like to save
	chx_quit();
	
	// restore terminal state
	texit();
	
	return 0;
}