#ifndef __CHX_CAOIMH__
#define __CHX_CAOIMH__

#define CHX_MODE_DEFAULT 		0
#define CHX_MODE_TYPE_DEFAULT 	1
#define CHX_MODE_TYPE_HEXCHAR 	2

#define KEY_UP 		0x41
#define KEY_DOWN 	0x42
#define KEY_RIGHT 	0x43
#define KEY_LEFT 	0x44

#define CHX_CTRL(C) (C & 0x1F)

#define IS_LETTER(C) (C ^ 0x40) < 26 || (C ^ 0x60) < 26
#define IS_HEX_CHAR(C) (C ^ 0x40) < 7 || (C ^ 0x60) < 7
#define IS_DIGIT(C) (C ^ 0x30) < 10

struct CHX_GLOBAL_CONFIG {
	struct chx_finfo* instances;
	char num_instances;
	char sel_instance;
	char mode;
	char bytes_per_row;
	char bytes_in_group;
	char bytes_in_last_row;
	char num_digits;
	long num_rows;
	long section_start;
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

char chx_getch();
void chx_draw_contents();

#endif