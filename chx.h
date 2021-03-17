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
#define cls() printf("\e[2J");
#define cur_set(X, Y) printf("\033[%d;%dH", Y + 1, X + 1)

#define TPD 1
#define BPD 1
#define PD (TPD + BPD)

#define IS_LETTER(C) ((C ^ 0x40) < 26 || (C ^ 0x60) < 26)
#define IS_CHAR_HEX(C) ((C ^ 0x40) < 7 || (C ^ 0x60) < 7 || (C ^ 0x30) < 10)
#define IS_DIGIT(C) ((C ^ 0x30) < 10)

#define CINST CHX_INSTANCES[CHX_SEL_INSTANCE]
#define BETWEEN(X, A, B) (X >= min(A, B) && X <= max(A, B))
#define CHX_CURSOR_X (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((CINST.cursor.pos % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (CINST.cursor.pos % CINST.bytes_in_group) + CINST.cursor.sbpos + CINST.group_spacing)
#define CHX_CURSOR_Y (int) ((CINST.cursor.pos - CINST.scroll_pos) / CINST.bytes_per_row + TPD)
#define WORD(X) *((uint16_t*) &X)

struct chx_command {
	void (*execute)(void);
	char* str;
};

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
	int copy_buffer_len;
	char* copy_buffer;
	struct chx_finfo fdata;
	struct CHX_CURSOR cursor;
	char bytes_per_row;
	char bytes_in_group;
	char group_spacing;
	char row_num_len;
	int height;
	int width;
	int x_offset;
	int y_offset;
	int num_bytes;
	int num_rows;
	long scroll_pos;
	long sel_start;
	long sel_stop;
	char selected;
	char saved;
	char mode;
};

struct CHX_INSTANCE CHX_INSTANCES[8];
int CHX_SEL_INSTANCE = 0;

void fvoid() {};

struct chx_finfo chx_import(char* fpath);
void chx_export(char* fpath);

void chx_prompt_command();

struct chx_key chx_get_key();
char chx_get_char();

void chx_print_status();
void chx_draw_contents();
void chx_redraw_line();

void chx_main();

#endif