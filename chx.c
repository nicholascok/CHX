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
#define cur_set(X, Y) printf("\033[%d;%dH", Y + 1, X + 1)

struct CHX_GLOBAL_CONFIG {
	struct chx_fdata* instances;
	int num_instances, sel_instance;
	int theight, twidth;
	int bytes_per_row, bytes_in_last_row;
	int num_rows, num_digits, npr;
	int section_start;
} CHX_GC;

// get the version of a key with ctrl pressed
#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return buf;
}

struct chx_fdata {
	unsigned char* data;
	long len;
};

struct chx_fdata chx_load(char* fpath) {
	struct chx_fdata fdata;
	FILE* inf = fopen(fpath, "r+b");
	if (!inf) inf = fopen(fpath, "w+b");
	fseek(inf, 0, SEEK_END);
	long flen = ftell(inf);
	rewind(inf);
	fdata.len = flen;
	fdata.data = malloc(flen);
	fread(fdata.data, 1, flen, inf);
	fclose(inf);
	return fdata;
}

void chx_export(char* fpath) {
	FILE* outf = fopen(fpath, "w+b");
	fwrite(CHX_GC.instances[CHX_GC.sel_instance].data, 1, CHX_GC.instances[CHX_GC.sel_instance].len, outf);
	fclose(outf);
}

int chx_num_digits(long num) {
	int count = 1;
	while (num > 15) {
		num /= 16;
		count++;
	}
	return count;
}

struct chx_cursor {
	int x, y;
	char blink : 1;
	char light : 1;
	char color : 2;
	char sbpos : 4;
};

void chx_draw_contents() {
	for (int j = 0; j < CHX_GC.npr - 1; j++)
		for (int i = 0; i < CHX_GC.bytes_per_row; i++)
			printf("\033[%d;%dH %02X", j + 2, CHX_GC.num_digits + 2 + i * 4, CHX_GC.instances[CHX_GC.sel_instance].data[j * CHX_GC.bytes_per_row + i]);
	if (CHX_GC.num_rows == CHX_GC.npr - 1) for (int i = 0; i < CHX_GC.bytes_in_last_row; i++) printf("\033[%d;%dH %02X", CHX_GC.npr + 1, CHX_GC.num_digits + 2 + i * 4, CHX_GC.instances[CHX_GC.sel_instance].data[(CHX_GC.npr - 1) * CHX_GC.bytes_per_row + i]);
}

int main(int arc, char** argv) {
	exit();
	enter();
	char usrin[256];
	
	struct winsize size;
	ioctl(0, TIOCGWINSZ, (char*) &size);
	
	struct chx_fdata hdata = chx_load(argv[1]);
	
	int bytes_per_row = 16;
	int bytes_in_last_row = hdata.len % bytes_per_row;
	int num_rows = (hdata.len - bytes_in_last_row) / bytes_per_row;
	int num_digits = chx_num_digits(num_rows);
	int npr = (num_rows < size.ws_row) ? num_rows + 1 : size.ws_row;
	int section_start = 0;
	CHX_GC = (struct CHX_GLOBAL_CONFIG) {&hdata, 1, 0, size.ws_row, size.ws_col, bytes_per_row, bytes_in_last_row, num_rows, num_digits, npr, section_start};
	
	cls();
	printf("%-*c", num_digits + 1, ' ');
	for (int i = 0; i < bytes_per_row; i++) printf(" \e[1;34m%02X \e[m", i);
	for (int i = section_start; i < npr; i++) printf("\e[1;34m\033[%d;0H %0*X \e[m", i + 2, num_digits, i * bytes_per_row);
	
	chx_draw_contents();
	
	struct chx_cursor CURSOR = {0, 0, 0, 1, 3};
	cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4, CURSOR.y + 1);
	fflush(stdout);
	
	for(char key; key != 'q'; key = getch()) {
		if (key == ';') continue;
		if ((key ^ 0x40) < 7 || (key ^ 0x60) < 7 || (key ^ 0x30) < 10) {
			if ((key ^ 0x60) < 7) key -= 32;
			printf("\033[1;35m%c\033[0m\033[1D", key);
			CHX_GC.instances[CHX_GC.sel_instance].data[CURSOR.y * CHX_GC.bytes_per_row + CURSOR.x] &= 0xF0 << ((1 - CURSOR.sbpos) * 4);
			CHX_GC.instances[CHX_GC.sel_instance].data[CURSOR.y * CHX_GC.bytes_per_row + CURSOR.x] |= ((char) strtol(&key, NULL, 16)) << ((1 - CURSOR.sbpos) * 4);
			fflush(stdout);
			if (CURSOR.sbpos < 1) CURSOR.sbpos++;
			else {
				char is_end = (CURSOR.y >= npr - 1 && CURSOR.x >= CHX_GC.bytes_in_last_row - 1);
				CURSOR.sbpos = 0;
				if (is_end) {
					CURSOR.sbpos = 1;
				} else if (CURSOR.x >= CHX_GC.bytes_per_row - 1) {
					CURSOR.x = 0;
					CURSOR.y++;
				} else CURSOR.x++;
			}
			cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4 + CURSOR.sbpos, CURSOR.y + 1);
			fflush(stdout);
		} else {
			switch(key) {
				case 's':
					chx_export(argv[1]);
					break;
				case 5:
					for (int i = 0; i < 256; i++) usrin[i] = 0;
					int tmp_curx = CURSOR.x, tmp_cury = CURSOR.y;
					cur_set(0, CHX_GC.theight - 2);
					printf("SAVE AS? (LEAVE EMPTY TO CANCEL): ");
					fflush(stdout);
					fgets(usrin, 256, stdin);
					usrin[strcspn(usrin, "\n")] = 0;
					if (usrin[0]) chx_export(usrin);
					printf("\033[1A\033[2K");
					fflush(stdout);
					if (usrin[0]) chx_draw_contents();
					cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4 + CURSOR.sbpos, CURSOR.y + 1);
					fflush(stdout);
					break;
			}
		}
		if (key == '\033') {
			getch();
			char is_end = (CURSOR.y >= npr - 1 && CURSOR.x >= CHX_GC.bytes_in_last_row - 1);
			switch(getch()) {
				case 'A'://up
					CURSOR.y -= CURSOR.y > 0;
					cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4 + CURSOR.sbpos, CURSOR.y + 1);
					break;
				case 'B'://down
					if (!(CURSOR.y >= npr - 2 && CURSOR.x >= CHX_GC.bytes_in_last_row)) {
						CURSOR.y = (CURSOR.y >= npr - 1) ? 0 : CURSOR.y + 1;
						cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4 + CURSOR.sbpos, CURSOR.y + 1);
					}
					break;
				case 'C'://right
					if (CURSOR.sbpos < 1) CURSOR.sbpos++;
					else if (!is_end) {
						CURSOR.sbpos = 0;
						CURSOR.x = (CURSOR.x >= bytes_per_row - 1) ? 0 : CURSOR.x + 1;
					}
					cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4 + CURSOR.sbpos, CURSOR.y + 1);
					break;
				case 'D'://left
					if (CURSOR.sbpos > 0) CURSOR.sbpos--;
					else {
						if (CURSOR.x) CURSOR.sbpos = 1;
						CURSOR.x -= (CURSOR.x) && 1;
					}
					cur_set(CHX_GC.num_digits + 2 + CURSOR.x * 4 + CURSOR.sbpos, CURSOR.y + 1);
					break;
			}
			fflush(stdout);
		}
	}
	l_exit:
	cur_set(0, CHX_GC.theight - 2);
	for (int i = 0; i < 256; i++) usrin[i] = 0;
	printf("WOULD YOU LIKE TO SAVE? (Y / N): ");
	switch (getchar()) {
		case 'y':
		case 'Y':
			chx_export(argv[1]);
			break;
		default:
			printf("\033[1A\033[2K");
			goto l_exit;
			break;
		case 'n':
		case 'N':
			break;
	}
	exit();
}