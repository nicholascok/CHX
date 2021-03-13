#include "chx.h"
#include "chx_defaults.c"
#include "CONFIG.c"

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

void chx_draw_contents() {
	// save sursor position
	int tmp_curx = CHXCUR.x, tmp_cury = CHXCUR.y;
	
	// reset cursor
	cur_set(0, 0);
	
	// print header
	printf("%-*c\e[1;34m", CHXGC.num_digits + 1, ' ');
	for (int i = 0; i < CHXGC.bytes_per_row / CHXGC.bytes_in_group; i++) printf(" %02X%-*c", i * CHXGC.bytes_in_group, (CHXGC.bytes_in_group == 1) ? 1 : CHXGC.bytes_in_group * 2 - 1, ' ');
	
	// print row numbers
	for (int i = 0; i < CHXGC.rows_in_section - BPD; i++)
		printf("\033[%d;0H %0*X ", i + TPD + 1, CHXGC.num_digits, (i + CHXGC.section_start) * CHXGC.bytes_per_row);
	
	printf("\e[0m");
	
	// print main contents
	for (int j = 0; j < CHXGC.rows_in_section; j++)
		for (int i = 0; i < CHXGC.bytes_per_row / CHXGC.bytes_in_group; i++) {
			printf(" \033[%d;%dH ", j + TPD + 1, CHXGC.num_digits + 2 + i * (2 + CHXGC.bytes_in_group * 2));
			for (int k = 0; k < CHXGC.bytes_in_group; k++)
				printf("%02X", CHXGC.instances[CHXGC.sel_instance].data[(j + CHXGC.section_start) * CHXGC.bytes_per_row + (i * CHXGC.bytes_in_group) + k]);
		}
	
	// print last row of bytes
	if (CHXGC.section_start == CHXGC.num_rows - CHXGC.rows_in_section + 1) {
		printf("\e[1;34m\033[2K\033[%d;0H %0*X\e[0m", CHXGC.rows_in_section + TPD, CHXGC.num_digits, (CHXGC.rows_in_section + CHXGC.section_start - 1) * CHXGC.bytes_per_row);
		for (int i = 0; i < CHXGC.bytes_in_last_row / CHXGC.bytes_in_group; i++) {
			printf("\033[%d;%dH", CHXGC.rows_in_section + TPD, CHXGC.num_digits + 2 + i * (2 + CHXGC.bytes_in_group * 2) + 1);
			for (int k = 0; k < CHXGC.bytes_in_group; k++)
				printf("%02X", CHXGC.instances[CHXGC.sel_instance].data[(CHXGC.rows_in_section + CHXGC.section_start - 1) * CHXGC.bytes_per_row + (i * CHXGC.bytes_in_group) + k]);
		}
		if (CHXGC.bytes_in_last_row % CHXGC.bytes_in_group) {
			printf("\033[%d;%dH", CHXGC.rows_in_section + TPD, CHXGC.num_digits + 2 + CHXGC.bytes_in_last_row / CHXGC.bytes_in_group * (2 + CHXGC.bytes_in_group * 2) + 1);
			for (int k = 0; k < CHXGC.bytes_in_last_row % CHXGC.bytes_in_group; k++)
				printf("%02X", CHXGC.instances[CHXGC.sel_instance].data[(CHXGC.rows_in_section + CHXGC.section_start - 1) * CHXGC.bytes_per_row + CHXGC.bytes_in_last_row - CHXGC.bytes_in_last_row % CHXGC.bytes_in_group + k]);
		}
	}
	
	// restore cursor position
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * (2 + CHXGC.bytes_in_group * 2) + CHXCUR.sbpos, CHXCUR.y - CHXGC.section_start + TPD);
	
	fflush(stdout);
}

void chx_main() {
	// draw content
	chx_draw_contents();
	
	// main loop
	for(char key;; key = chx_getch()) {
		// control keys are global
		if (chx_keybinds_global[key]) chx_keybinds_global[key]();
		
		switch (key) {
			// escape key and ansi escape keys
			case -1: // actual escape key (return to command mode)
				CHXGC.mode = CHX_MODE_DEFAULT;
				continue;
			case '\033': ; // ANSI escape sequence
				char esc_key = chx_getch();
				if (chx_keybinds_global_escape[esc_key]) chx_keybinds_global_escape[esc_key]();
				continue;
		}
		// keys have different actions depending on active mode setting
		switch (CHXGC.mode) {
			default:
			case CHX_MODE_DEFAULT:
				if (chx_keybinds_mode_command[key]) chx_keybinds_mode_command[key]();
				break;
			case CHX_MODE_TYPE_HEXCHAR:
				if (IS_CHAR_HEX(key)) chx_type_hexchar(key);
				break;
		}
	}
}

int chx_count_digits(long num) {
	int count = 1;
	while (num > 15) num /= 16, count++;
	return count;
}

char chx_getch() {
        short buf = 0;
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
        read(0, &buf, 2);
		if (buf == '\033') buf = -1;
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        tcsetattr(0, TCSADRAIN, &old);
        return (char) buf;
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
	
	// setup global variables
	CHXGC.mode = 0;
	CHXGC.instances = malloc(sizeof(struct chx_finfo));
	CHXGC.instances[0] = hdata;
	CHXGC.num_instances = 1;
	CHXGC.sel_instance = 0;
	CHXGC.theight = size.ws_row;
	CHXGC.twidth = size.ws_col;
	CHXGC.bytes_per_row = 16;
	CHXGC.bytes_in_group = 1;
	CHXGC.bytes_in_last_row = hdata.len % CHXGC.bytes_per_row;
	CHXGC.num_rows = hdata.len / CHXGC.bytes_per_row;
	CHXGC.num_digits = chx_count_digits(hdata.len);
	CHXGC.rows_in_section = (CHXGC.num_rows < size.ws_row - PD) ? CHXGC.num_rows + 1 : size.ws_row - PD;
	CHXGC.section_start = 0;
	
	if (CHXGC.bytes_per_row % CHXGC.bytes_in_group) {
		CHXGC.bytes_per_row = 16;
		CHXGC.bytes_in_group = 1;
	}
	
	// initialize cursor
	CHXCUR = (struct CHX_CURSOR) {0, 0, 0, 0};
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * 4, CHXCUR.y + 1 - CHXGC.section_start);
	fflush(stdout);
	
	lb_return:
	
	// call main loop
	chx_main();
	
	// after exiting ask if user would like to save
	chx_quit();
	
	// restore terminal state
	texit();
}