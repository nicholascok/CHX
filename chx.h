#ifndef __CHX_CAOIMH__
#define __CHX_CAOIMH__

#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <malloc.h>
#include <locale.h>
#include <byteswap.h>
#include <unistd.h>
#include <limits.h>
#include <termios.h>
#include <sys/ioctl.h>

#define TRUE 1
#define FALSE 0

#define KEY_UP 		0x0141
#define KEY_DOWN 	0x0142
#define KEY_RIGHT 	0x0143
#define KEY_LEFT 	0x0144

#define KEY_DELETE 0x0133
#define KEY_INSERT 0x0132
#define KEY_PG_UP 0x0135
#define KEY_PG_DN 0x0136
#define KEY_HOME 0x0148
#define KEY_END 0x0146

#define KEY_ENTER 0x000A
#define KEY_TAB 0x0009

#define KEY_ESCAPE 	0x0100
#define KEY_MAX_VAL 0xFFFF

#define CHX_MODE_DEFAULT		0
#define CHX_MODE_REPLACE		1
#define CHX_MODE_INSERT			2
#define CHX_MODE_TYPE			3
#define CHX_MODE_REPLACE_ASCII	4
#define CHX_MODE_INSERT_ASCII	5
#define CHX_MODE_TYPE_ASCII		6

#define tenter() system("tput smcup")
#define texit() system("tput rmcup")
#define cls() printf("\e[2J");
#define cur_set(X, Y) printf("\033[%d;%dH", Y + 1, X + 1)

#define TPD 1
#define BPD 1
#define PD (TPD + BPD)

#define CHX_LITTLE_ENDIAN 1
#define CHX_BIG_ENDIAN 0

#define FORMAT_UNDERLINE "\e[4m"
#define FORMAT_BOLD "\e[1m"
#define FORMAT_REVERSE "\e[7m"
#define COLOUR_BLACK "\e[30m"
#define COLOUR_RED "\e[31m"
#define COLOUR_GREEN "\e[32m"
#define COLOUR_YELLOW "\e[33m"
#define COLOUR_BLUE "\e[34m"
#define COLOUR_MAGENTA "\e[35m"
#define COLOUR_CYAN "\e[36m"
#define COLOUR_WHITE "\e[37m"
#define COLOUR_GREY "\e[90m"

#define IS_PRINTABLE(C) (C > 0x1F && C < 0x7F)
#define IS_LETTER(C) ((C ^ 0x40) < 26 || (C ^ 0x60) < 26)
#define IS_CHAR_HEX(C) ((C ^ 0x40) < 7 || (C ^ 0x60) < 7 || (C ^ 0x30) < 10)
#define IS_DIGIT(C) ((C ^ 0x30) < 10)
#define IS_QUOTATION(C) (C == '\'' || C == '"')

#define CINST CHX_INSTANCES[CHX_SEL_INSTANCE]
#define BETWEEN_GE1_L2(X, A, B) (X >= min(A, B) && X < max(A, B))
#define CHX_CONTENT_END (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + CINST.group_spacing)
#define CHX_PREVIEW_END (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * (CINST.bytes_per_row / CINST.bytes_in_group) + 2 * CINST.group_spacing + CINST.bytes_per_row)
#define CHX_CURSOR_X (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((CINST.cursor.pos % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (CINST.cursor.pos % CINST.bytes_in_group) + CINST.cursor.sbpos + CINST.group_spacing)
#define CHX_CURSOR_Y (int) (CINST.cursor.pos / CINST.bytes_per_row - CINST.scroll_pos + TPD)
#define CHX_GET_X(X) (int) (CINST.row_num_len + (CINST.bytes_in_group * 2 + CINST.group_spacing) * ((X % CINST.bytes_per_row) / CINST.bytes_in_group) + 2 * (X % CINST.bytes_in_group) + CINST.group_spacing)
#define CHX_GET_Y(X) (int) (X / CINST.bytes_per_row - CINST.scroll_pos + TPD)

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
#define DWORD(X) *((uint32_t*) &X)
#define INT8_AT(X) *((int8_t*) (X))
#define INT16_AT(X) *((int16_t*) (X))
#define INT32_AT(X) *((int32_t*) (X))
#define INT64_AT(X) *((int64_t*) (X))
#define UINT8_AT(X) *((uint8_t*) (X))
#define UINT16_AT(X) *((uint16_t*) (X))
#define UINT32_AT(X) *((uint32_t*) (X))
#define UINT64_AT(X) *((uint64_t*) (X))
#define WCHAR_AT(X) (wchar_t) *((int16_t*) (X))

union chx_last_action_ptr {
	void (*execute_void)(void);
	void (*execute_cmmd)(char _np, char** _pl);
};

struct chx_last_action {
	union chx_last_action_ptr action;
	char* params_raw;
	char** params;
	char num_params;
	char type;
};

struct chx_void_command {
	void (*execute)(void);
	char* str;
};

struct chx_command {
	void (*execute)(char _np, char** _pl);
	char* str;
};

struct chx_cursor {
	long pos;
	long line;
	char sbpos;
};

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
	struct chx_cursor cursor;
	struct chx_last_action last_action;
	char bytes_per_row;
	char bytes_in_group;
	char group_spacing;
	char row_num_len;
	char min_row_num_len;
	int height;
	int width;
	int x_offset;
	int y_offset;
	int num_rows;
	long scroll_pos;
	long sel_start;
	long sel_stop;
	char parity;
	char endianness;
	char selected;
	char saved;
	char mode;
	char show_inspector;
	char show_preview;
};

struct CHX_INSTANCE* CHX_INSTANCES;
int CHX_CUR_MAX_INSTANCE;
int CHX_SEL_INSTANCE;

void (*chx_keybinds_global[])(void);
void (*chx_keybinds_mode_command[])(void);
void* func_exceptions[];
struct chx_command chx_commands[];

void fvoid() {};

struct chx_finfo chx_import(char* fpath);
void chx_export(char* fpath);

void chx_config_layout(char _np, char** _pl);
void chx_config_layout_global(char _np, char** _pl);

void chx_add_instance(char* fpath);
void chx_remove_instance(int _n);
void chx_prompt_command();

struct chx_key chx_get_key();
char chx_get_char();
void chx_get_str();

void chx_scroll_up(int _n);
void chx_scroll_down(int _n);

void chx_print_status();
void chx_update_cursor();
void chx_swap_endianness();
void chx_draw_contents();
void chx_draw_sidebar();
void chx_draw_extra();
void chx_draw_header();
void chx_draw_all();
void chx_redraw_line();

void chx_set_hexchar(char _c);
void chx_type_hexchar(char _c);
void chx_insert_hexchar(char _c);
void chx_delete_hexchar();
void chx_backspace_hexchar();
void chx_remove_hexchar();
void chx_erase_hexchar();

void chx_set_ascii(char _c);
void chx_type_ascii(char _c);
void chx_insert_ascii(char _c);
void chx_delete_ascii();
void chx_backspace_ascii();
void chx_remove_ascii();
void chx_erase_ascii();

void chx_main();

#endif