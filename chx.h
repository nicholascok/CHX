#ifndef __CHX_CAOIMH__
#define __CHX_CAOIMH__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <malloc.h>
#include <locale.h>
#include <byteswap.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#define CHX_MODE_DEFAULT 	0
#define CHX_MODE_REPLACE 	1
#define CHX_MODE_INSERT 	2

#define tenter() system("tput smcup")
#define texit() system("tput rmcup")
#define cls() printf("\e[2J");
#define cur_set(X, Y) printf("\033[%d;%dH", Y + 1, X + 1)

#define TPD 1
#define BPD 1
#define PD (TPD + BPD)

#define IS_PRINTABLE(C) (C > 0x20 && C < 0x7F)
#define IS_LETTER(C) ((C ^ 0x40) < 26 || (C ^ 0x60) < 26)
#define IS_CHAR_HEX(C) ((C ^ 0x40) < 7 || (C ^ 0x60) < 7 || (C ^ 0x30) < 10)
#define IS_DIGIT(C) ((C ^ 0x30) < 10)

#define CINST CHX_INSTANCES[CHX_SEL_INSTANCE]
#define BETWEEN(X, A, B) (X >= min(A, B) && X <= max(A, B))
#define CHX_CONTENT_END (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + CINST.group_spacing)
#define CHX_SIDEBAR_END (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + 2 * CINST.group_spacing + CINST.bytes_per_row)
#define CHX_CURSOR_X (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((CINST.cursor.pos % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (CINST.cursor.pos % CINST.bytes_in_group) + CINST.cursor.sbpos + CINST.group_spacing)
#define CHX_CURSOR_Y (int) ((CINST.cursor.pos - CINST.scroll_pos) / CINST.bytes_per_row + TPD)

#define BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte) \
	(byte & 0x80 ? '1' : '0'), \
	(byte & 0x40 ? '1' : '0'), \
	(byte & 0x20 ? '1' : '0'), \
	(byte & 0x10 ? '1' : '0'), \
	(byte & 0x08 ? '1' : '0'), \
	(byte & 0x04 ? '1' : '0'), \
	(byte & 0x02 ? '1' : '0'), \
	(byte & 0x01 ? '1' : '0')

#define WORD(X) *((uint16_t*) &X)
#define INT8_AT(X, P) *((int8_t*) (X + P))
#define INT16_AT(X, P) *((int16_t*) (X + P))
#define INT32_AT(X, P) *((int32_t*) (X + P))
#define INT64_AT(X, P) *((int64_t*) (X + P))
#define UINT8_AT(X, P) *((uint8_t*) (X + P))
#define UINT16_AT(X, P) *((uint16_t*) (X + P))
#define UINT32_AT(X, P) *((uint32_t*) (X + P))
#define UINT64_AT(X, P) *((uint64_t*) (X + P))
#define WCHAR_AT(X, P) (wchar_t) *((int16_t*) (X + P))

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
	char endianness;
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
void chx_draw_sidebar();
void chx_draw_extra();
void chx_redraw_line();

void chx_main();

#endif