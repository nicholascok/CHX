#ifndef __CHX_CAOIMH__
#define __CHX_CAOIMH__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

#define tenter() system("tput smcup")
#define texit() system("tput rmcup")
#define cls() system("clear")
#define cur_set(X, Y) printf("\033[%d;%dH", Y + 1, X + 1)

#define TPD 1
#define BPD 0
#define PD TPD + BPD

#define CHX_MODE_DEFAULT 		0
#define CHX_MODE_TYPE_DEFAULT 	1
#define CHX_MODE_TYPE_HEXCHAR 	2

#define KEY_UP 		0x41
#define KEY_DOWN 	0x42
#define KEY_RIGHT 	0x43
#define KEY_LEFT 	0x44

#define CHX_CTRL(C) (C & 0x1F)

#define IS_LETTER(C) ((C ^ 0x40) < 26 || (C ^ 0x60) < 26)
#define IS_CHAR_HEX(C) ((C ^ 0x40) < 7 || (C ^ 0x60) < 7 || (C ^ 0x30) < 10)
#define IS_DIGIT(C) ((C ^ 0x30) < 10)

struct CHX_GLOBAL_CONFIG {
	struct chx_finfo* instances;
	char num_instances;
	char sel_instance;
	char mode;
	char bytes_per_row;
	char bytes_in_group;
	char bytes_in_last_row;
	char num_digits;
	int num_rows;
	int section_start;
	int rows_in_section;
	int theight;
	int twidth;
} CHXGC;

struct CHX_CURSOR {
	int x, y;
	char style : 4;
	char sbpos : 4;
} CHXCUR;

struct chx_finfo {
	unsigned char* data;
	char* filename;
	long len;
};

struct chx_finfo chx_import(char* fpath);
void chx_export(char* fpath);
void chx_update_cursor();
void chx_draw_contents();
int chx_count_digits(long num);
char chx_getch();
void chx_main();

#endif