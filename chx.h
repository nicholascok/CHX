#ifndef __CHX_CAOIMH__
#define __CHX_CAOIMH__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

#define CHX_MODE_DEFAULT 		0
#define CHX_MODE_TYPE_DEFAULT 	1
#define CHX_MODE_TYPE_HEXCHAR 	2

#define tenter() system("tput smcup")
#define texit() system("tput rmcup")
#define cls() system("clear")
#define cur_set(X, Y) printf("\033[%d;%dH", Y + 1, X + 1)

#define TPD 1
#define BPD 0
#define PD TPD + BPD

#define IS_LETTER(C) ((C ^ 0x40) < 26 || (C ^ 0x60) < 26)
#define IS_CHAR_HEX(C) ((C ^ 0x40) < 7 || (C ^ 0x60) < 7 || (C ^ 0x30) < 10)
#define IS_DIGIT(C) ((C ^ 0x30) < 10)

#define CINST CHX_INSTANCES[CHX_SEL_INSTANCE]
#define BETWEEN(X, A, B) (X >= min(A, B) && X <= max(A, B))
#define CHX_CURSOR_X (int) (CINST.row_num_len + 4 * (CINST.cursor.pos % CINST.bytes_per_row) + CINST.cursor.sbpos + 2)
#define CHX_GET_CURSOR_Y (int) (CINST.cursor.pos / CINST.bytes_per_row - CINST.scroll_pos + TPD)
#define WORD(X) *((uint16_t*) &X)

struct CHX_CURSOR {
	long pos;
	char sbpos;
} CHXCUR;

struct chx_key {
	char val, type;
} __attribute__ ((__packed__));

struct chx_finfo {
	unsigned char* data;
	char* filename;
	long len, num_rows;
};

struct CHX_INSTANCE {
	unsigned char* style_data;
	struct chx_finfo fdata;
	struct CHX_CURSOR cursor;
	char bytes_per_row;
	char bytes_in_group;
	char row_num_len;
	int height;
	int width;
	int x_offset;
	int y_offset;
	int scroll_pos;
	int num_bytes;
	int num_rows;
	int selecting;
	int sel_start;
	int sel_end;
	char mode;
	char saved;
};

struct CHX_INSTANCE CHX_INSTANCES[8];
int CHX_SEL_INSTANCE = 0;

void fvoid() {};

struct chx_finfo chx_import(char* fpath);
void chx_export(char* fpath);

struct chx_key chx_get_key();
struct chx_key chx_get_char();
void chx_draw_contents();
void chx_main();

#endif