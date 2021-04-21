#include "chx.h"
#include "chx_defaults.c"
#include "config.h"

struct chx_finfo chx_import(char* fpath) {
	struct chx_finfo finfo;
	FILE* inf = fopen(fpath, "r+b");
	if (!inf) return (struct chx_finfo) {0};
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

void chx_scroll_up(int _n) {
	printf("\e[%dS", _n);
	while (_n--)
		chx_redraw_line(CINST.scroll_pos - _n + CINST.num_rows - 1);
	chx_draw_header();
	chx_print_status();
}

void chx_scroll_down(int _n) {
	printf("\e[%dT", _n);
	while (_n--)
		chx_redraw_line(CINST.scroll_pos + _n);
	chx_draw_header();
	chx_print_status();
}

void chx_update_cursor() {
	CINST.cursor.line = CINST.cursor.pos / CINST.bytes_per_row;
	if (CINST.cursor.pos < 0) CINST.cursor = (struct chx_cursor) {0};
	else {
		// scroll if past visible screen
		int scroll_pos_prev = CINST.scroll_pos;
		if (CINST.cursor.line >= CINST.scroll_pos + CINST.num_rows) {
			CINST.scroll_pos = CINST.cursor.line - CINST.num_rows + 1;
			#ifdef CHX_SCROLL_SUPPORT
			if (CINST.scroll_pos < scroll_pos_prev + CINST.num_rows)
				chx_scroll_up(CINST.scroll_pos - scroll_pos_prev);
			else
				chx_draw_contents();
			#else
				chx_draw_contents();
			#endif
		} else if (CINST.cursor.line < CINST.scroll_pos) {
			CINST.scroll_pos = CINST.cursor.line;
			#ifdef CHX_SCROLL_SUPPORT
			if (scroll_pos_prev < CINST.scroll_pos + CINST.num_rows)
				chx_scroll_down(scroll_pos_prev - CINST.scroll_pos);
			else
				chx_draw_contents();
			#else
				chx_draw_contents();
			#endif
		}
	}
	
	if (CINST.show_inspector)
		chx_draw_extra();
	
	if (CINST.show_preview)
		chx_draw_sidebar();
	
	// redraw cursor
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_redraw_line(long line) {
	// calculate line number
	long line_start = line * CINST.bytes_per_row;
	int cur_y = line - CINST.scroll_pos + TPD;
	
	// print row number
	cur_set(0, cur_y);
	printf(CHX_FRAME_COLOUR"%0*lX \e[0m%-*c", CINST.row_num_len, line_start, CINST.group_spacing, ' ');
	
	// print row contents
	long sel_begin = min(CINST.sel_start, CINST.sel_stop);
	long sel_end = max(CINST.sel_start, CINST.sel_stop);
	
	cur_set(CINST.row_num_len + CINST.group_spacing, cur_y);
	
	if (CINST.selected && line_start >= sel_begin && line_start < sel_end)
		printf(CHX_ASCII_SELECT_FORMAT);
	
	for (long i = line_start; i < line_start + CINST.bytes_per_row; i++) {
		if (i != line_start && !(i % CINST.bytes_in_group) && CINST.group_spacing != 0)
			printf("%-*c", CINST.group_spacing, ' ');
		
		if (CINST.selected && i == sel_begin && sel_end != sel_begin)
			printf(CHX_ASCII_SELECT_FORMAT);
		
		if (i < CINST.fdata.len) {
			if ((!CINST.selected || i < sel_begin || i >= sel_end) && CINST.style_data[i / 8] & (0x80 >> (i % 8)))
				printf(CHX_UNSAVED_COLOUR"%02X\e[0m", CINST.fdata.data[i]);
			else
				printf("%02X", CINST.fdata.data[i]);
		} else
			printf("..");
		
		if (CINST.selected && i == sel_end - 1)
			printf("\e[0m");
	}
	
	printf("\e[0m");
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
}

void chx_print_status() {
	// print current mode setting
	cur_set(0, CINST.height);
	switch (CINST.mode) {
		case CHX_MODE_DEFAULT:
			printf("\e[2K[ COMMAND ]");
			break;
		case CHX_MODE_TYPE:
			printf("\e[2K[ TYPE ]");
			break;
		case CHX_MODE_INSERT:
			printf("\e[2K[ INSERT ]");
			break;
		case CHX_MODE_REPLACE:
			printf("\e[2K[ REPLACE ]");
			break;
		case CHX_MODE_TYPE_ASCII:
			printf("\e[2K[ ASCII TYPE ]");
			break;
		case CHX_MODE_INSERT_ASCII:
			printf("\e[2K[ ASCII INSERT ]");
			break;
		case CHX_MODE_REPLACE_ASCII:
			printf("\e[2K[ ASCII REPLACE ]");
			break;
		default:
			printf("\e[2K[ UNKNOWN ]");
			break;
	}
	printf(" I%02i '%s' (%li bytes)", CHX_SEL_INSTANCE, CINST.fdata.filename, CINST.fdata.len);
}

void chx_draw_extra() {
	// copy bytes from file
	char buf[16];
	for (long i = 0; i < 16; i++)
		if (CINST.cursor.pos + i < CINST.fdata.len)
			buf[i] = CINST.fdata.data[CINST.cursor.pos + i];
		else
			buf[i] = 0;
	
	int offset = (CINST.show_preview) ? CHX_PREVIEW_END : CHX_CONTENT_END;
	
	// clear bit of screen
	cur_set(offset - 1, 0);
	for (int i = 0; i < CINST.num_rows + PD; i++)
		printf("\e[0K\e[%dG\e[1B", offset);
	
	// print inspected data
	cur_set(offset, 0);
	printf("\e[1m");
	printf("\e[0KData Inspector:");
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kbinary: "BINARY_PATTERN, BYTE_TO_BINARY(buf[0]));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kint8: %i", INT8_AT(&buf));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kint16: %i", (CINST.endianness) ? INT16_AT(&buf) : __bswap_16 (INT16_AT(&buf)));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kint32: %i", (CINST.endianness) ? INT32_AT(&buf) : __bswap_32 (INT32_AT(&buf)));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kint64: %li", (CINST.endianness) ? INT64_AT(&buf) : __bswap_64 (INT64_AT(&buf)));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kuint8: %u", UINT8_AT(&buf));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kuint16: %u", (CINST.endianness) ? UINT16_AT(&buf) : __bswap_16 (UINT16_AT(&buf)));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kuint32: %u", (CINST.endianness) ? UINT32_AT(&buf) : __bswap_32 (UINT32_AT(&buf)));
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kuint64: %lu", (CINST.endianness) ? UINT64_AT(&buf) : __bswap_64 (UINT64_AT(&buf)));
	printf("\e[%dG\e[1B ", offset);
	if (IS_PRINTABLE(buf[0]))
		printf("\e[0KANSI char: %c", buf[0]);
	else
		printf("\e[0KANSI char: \ufffd");
	printf("\e[%dG\e[1B ", offset);
	printf("\e[0Kwide char: %lc", (CINST.endianness) ? WCHAR_AT(&buf) : __bswap_16 (WCHAR_AT(&buf)));
	printf("\e[%dG\e[1B\e[0K\e[1B ", offset);
	if (CINST.endianness) printf("\e[0K[LITTLE ENDIAN]");
	else printf("\e[0K[BIG ENDIAN]");
	printf("\e[0m");
}

void chx_draw_all() {
	// draw elements
	chx_draw_header();
	chx_draw_contents();
	
	if (CINST.show_preview)
		chx_draw_sidebar();
	
	if (CINST.show_inspector)
		chx_draw_extra();
	
	chx_print_status();
	
	// restore cursor position
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_draw_header() {
	int rnum_digits = chx_count_digits((CINST.scroll_pos + CINST.num_rows) * CINST.bytes_per_row - 1);
	int rnl_old = CINST.row_num_len;
	
	if (rnum_digits > CINST.min_row_num_len)
		CINST.row_num_len = rnum_digits;
	else
		CINST.row_num_len = CINST.min_row_num_len;
	
	if (rnl_old != CINST.row_num_len)
		chx_draw_all();
	
	printf("\e[0;0H%-*c"CHX_FRAME_COLOUR, CINST.row_num_len + CINST.group_spacing, ' ');
	for (int i = 0; i < CINST.bytes_per_row / CINST.bytes_in_group; i++)
		printf("%02X%-*c", i * CINST.bytes_in_group, CINST.bytes_in_group * 2 + CINST.group_spacing - 2, ' ');
	printf("\e[0m");
}

void chx_draw_contents() {
	// print row numbers
	for (int i = 0; i < CINST.num_rows; i++) {
		cur_set(0, i + TPD);
		printf(CHX_FRAME_COLOUR"%0*lX \e[0m%-*c", CINST.row_num_len, (long) ((i + CINST.scroll_pos) * CINST.bytes_per_row), CINST.group_spacing, ' ');
	}
	
	printf("\e[0m");
	
	// print main contents
	long sel_begin = min(CINST.sel_start, CINST.sel_stop);
	long sel_end = max(CINST.sel_start, CINST.sel_stop);
	
	if (CINST.selected && sel_begin < CINST.scroll_pos * CINST.bytes_per_row)
		printf(CHX_ASCII_SELECT_FORMAT);
	
	for (long i = CINST.scroll_pos * CINST.bytes_per_row; i < CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row; i++) {
		if (!(i % CINST.bytes_per_row)) {
			printf("%-*c", CINST.group_spacing, ' ');
			cur_set(CINST.row_num_len + CINST.group_spacing, CHX_GET_Y(i));
		} else if (!(i % CINST.bytes_in_group) && CINST.group_spacing != 0)
			printf("%-*c", CINST.group_spacing, ' ');
		
		if (CINST.selected && i == sel_begin && sel_end != sel_begin)
			printf(CHX_ASCII_SELECT_FORMAT);
		
		if (i < CINST.fdata.len) {
			if ((!CINST.selected || i < sel_begin || i >= sel_end) && CINST.style_data[i / 8] & (0x80 >> (i % 8)))
				printf(CHX_UNSAVED_COLOUR"%02X\e[0m", CINST.fdata.data[i]);
			else
				printf("%02X", CINST.fdata.data[i]);
		} else
			printf("..");
		
		if (CINST.selected && i == sel_end - 1)
			printf("\e[0m");
	}
	
	printf("\e[0m%-*c", CINST.group_spacing, ' ');
}

void chx_draw_sidebar() {
	// clear bit of screen if inspector is off
	if (!CINST.show_inspector) {
		cur_set(CHX_CONTENT_END, 0);
		for (int i = 0; i < CINST.num_rows + PD; i++)
			printf("\e[0K\e[%dG\e[1B", CHX_CONTENT_END);
	}
	
	cur_set(CHX_CONTENT_END, 0);
	printf("%-*c", CINST.bytes_per_row, ' ');
	
	long sel_begin = min(CINST.sel_start, CINST.sel_stop);
	long sel_end = max(CINST.sel_start, CINST.sel_stop);
	
	if (CINST.selected && sel_begin < CINST.scroll_pos * CINST.bytes_per_row)
		printf(CHX_ASCII_SELECT_FORMAT);
	
	for (long i = CINST.scroll_pos * CINST.bytes_per_row; i < CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row; i++) {
		if (!(i % CINST.bytes_per_row))
			cur_set(CHX_CONTENT_END, CHX_GET_Y(i));
		
		if (i == CINST.cursor.pos && !CINST.selected)
			printf(CHX_ASCII_CUR_FORMAT);
		
		if (i < CINST.fdata.len) {
			if (CINST.selected && i == sel_begin && sel_end != sel_begin)
				printf(CHX_ASCII_SELECT_FORMAT);
			
			if (IS_PRINTABLE(CINST.fdata.data[i]))
				printf("%c", CINST.fdata.data[i]);
			else
				printf("·");
		} else
			printf("•");
		
		if (CINST.selected) {
			if (i == sel_end - 1)
				printf("\e[0m");
		} else if (i == CINST.cursor.pos)
			printf("\e[0m");
	}
	
	printf("\e[0m%-*c", CINST.group_spacing, ' ');
}

void chx_prompt_command() {
	// setup user input buffer
	char* usrin = calloc(1, 256);
	
	// command interpreter recieve user input
	cur_set(0, CINST.height);
	printf("\e[2K: ");
	fflush(stdout);
	
	chx_get_str(usrin, 256);
	
	// extract command and its parameters
	char np = 0;
	char* cmd = chx_extract_param(usrin, 0);
	char** p = malloc(CHX_MAX_NUM_PARAMS * sizeof(void*));
	
	for (int i = 0; i < CHX_MAX_NUM_PARAMS; i++) {
		p[i] = chx_extract_param(usrin, i + 1);
		if (p[i][0]) np++;
	}
	
	// lookup entered command and execute procedure
	// for numbers (decimal or hex, prefixed with '0x') jump to the corresponging byte
	if (cmd[0]) {
		if (str_is_num(cmd)) {
			CINST.cursor.pos = str_to_num(cmd);
			CINST.cursor.sbpos = 0;
			chx_update_cursor();
			chx_draw_all();
		} else if (str_is_hex(cmd)) {
			CINST.cursor.pos = str_to_hex(cmd);
			CINST.cursor.sbpos = 0;
			chx_update_cursor();
			chx_draw_all();
		} else {
			for (int i = 0; chx_void_commands[i].str; i++)
				if (cmp_str(chx_void_commands[i].str, cmd)) {
					chx_void_commands[i].execute();
					CINST.last_action.action.execute_void = chx_void_commands[i].execute;
					CINST.last_action.type = 0;
					break;
				}
			
			for (int i = 0; chx_commands[i].str; i++)
				if (cmp_str(chx_commands[i].str, cmd)) {
					chx_commands[i].execute(np, p);
					free(CINST.last_action.params_raw);
					free(CINST.last_action.params);
					CINST.last_action.action.execute_cmmd = chx_commands[i].execute;
					CINST.last_action.params_raw = usrin;
					CINST.last_action.num_params = np;
					CINST.last_action.params = p;
					CINST.last_action.type = 1;
					break;
				}
		}
	}
	
	// redraw elements
	chx_draw_all();
}

void chx_set_hexchar(char _c) {
	if (!IS_CHAR_HEX(_c)) return; // only accept hex characters
	if ((_c ^ 0x60) < 7) _c -= 32; // ensure everything is upper-case
	
	char nullkey[2] = {_c, 0};
	
	// resize file if typing past current file length
	if (CINST.cursor.pos >= CINST.fdata.len) {
		chx_resize_file(CINST.cursor.pos + 1);
		chx_draw_contents();
	}
	
	// update stored file data
	CINST.fdata.data[CINST.cursor.pos] &= 0x0F << (CINST.cursor.sbpos * 4);
	CINST.fdata.data[CINST.cursor.pos] |= strtol(nullkey, NULL, 16) << (!CINST.cursor.sbpos * 4);
	
	// highlight unsaved change
	CINST.saved = 0;
	CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	chx_update_cursor();
}

void chx_type_hexchar(char _c) {
	chx_set_hexchar(_c);
	chx_cursor_move_right();
}

void chx_insert_hexchar_old(char _c) {
	chx_set_hexchar(_c);
	if (CINST.cursor.sbpos) {
		chx_resize_file(CINST.fdata.len + 1);
		for (int i = CINST.fdata.len - 1; i > CINST.cursor.pos; i--)
			CINST.fdata.data[i] = CINST.fdata.data[i - 1];
		CINST.fdata.data[CINST.cursor.pos + 1] = 0;
		chx_draw_all();
	}
	chx_cursor_move_right();
}

void chx_insert_hexchar(char _c) {
	#ifdef CHX_RESIZE_FILE_ON_INSERTION
	CINST.parity = !CINST.parity;
	if (CINST.parity && CINST.cursor.pos < CINST.fdata.len)
		chx_resize_file(CINST.fdata.len + 1);
	#endif
	
	// resize file if typing past current file length
	if (CINST.cursor.pos >= CINST.fdata.len) {
		chx_resize_file(CINST.cursor.pos + 1);
		chx_draw_contents();
	}
	
	// shift data after cursor by 4 bits
	unsigned char cr = 0;
	
	for (int i = CINST.fdata.len - 1; i > CINST.cursor.pos - !CINST.cursor.sbpos; i--) {
		cr = CINST.fdata.data[i - 1] & 0x0F;
		CINST.fdata.data[i] >>= 4;
		CINST.fdata.data[i] |= cr << 4;
	}
	
	// hightlight as unsaved change
	CINST.saved = 0;
	CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	
	// type hexchar and move cursor
	chx_set_hexchar(_c);
	chx_cursor_move_right();
	
	chx_draw_all();
	fflush(stdout);
}

void chx_delete_hexchar() {
	// only delete if cursor is before EOF
	if (CINST.cursor.pos < CINST.fdata.len)
		if (CINST.cursor.sbpos)
			CINST.fdata.data[CINST.cursor.pos] &= 0xF0;
		else
			CINST.fdata.data[CINST.cursor.pos] &= 0x0F;
	
	// hightlight as unsaved change
	CINST.saved = 0;
	CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	
	if (CINST.show_preview)
		chx_draw_sidebar();
	
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	fflush(stdout);
}

void chx_backspace_hexchar() {
	chx_cursor_move_left();
	chx_delete_hexchar();
}

void chx_remove_hexchar() {
	// only remove characters in the file
	if (CINST.cursor.pos < CINST.fdata.len) {
		CINST.saved = 0;
		
		// shift data after cursor by 4 bits
		if (CINST.cursor.sbpos)
			CINST.fdata.data[CINST.cursor.pos] &= 0xF0;
		else
			CINST.fdata.data[CINST.cursor.pos] <<= 4;
		
		unsigned char cr = 0;
		
		for (int i = CINST.cursor.pos; i < CINST.fdata.len - 1; i++) {
			cr = CINST.fdata.data[i + 1] & 0xF0;
			CINST.fdata.data[i] |= cr >> 4;
			CINST.fdata.data[i + 1] <<= 4;
		}
		
		#ifdef CHX_RESIZE_FILE_ON_BACKSPACE
		CINST.parity = !CINST.parity;
		if (!CINST.parity)
			chx_resize_file(CINST.fdata.len - 1);
		#endif
		
		chx_draw_all();
		fflush(stdout);
	} else if (CINST.cursor.pos == CINST.fdata.len - 1 && CINST.cursor.sbpos) {
		// if cursor is just after EOF, resize file to remove last byte
		chx_resize_file(CINST.fdata.len - 1);
		CINST.cursor.sbpos = 0;
	}
}

void chx_erase_hexchar() {
	if (CINST.cursor.pos || CINST.cursor.sbpos) {
		chx_cursor_move_left();
		chx_remove_hexchar();
	}
}

void chx_set_ascii(char _c) {
	// resize file if typing past current file length
	if (CINST.cursor.pos >= CINST.fdata.len) {
		chx_resize_file(CINST.cursor.pos + 1);
		chx_draw_contents();
	}
	
	// set char
	CINST.fdata.data[CINST.cursor.pos] = _c;
	
	// highlight unsaved change
	CINST.saved = 0;
	CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	
	if (CINST.show_preview)
		chx_draw_sidebar();
	
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	fflush(stdout);
}

void chx_type_ascii(char _c) {
	chx_set_ascii(_c);
	chx_cursor_next_byte();
}

void chx_insert_ascii(char _c) {
	// resize file
	chx_resize_file(CINST.fdata.len + 1);
	
	// shift bytes after cursor right by one
	for (int i = CINST.fdata.len - 1; i > CINST.cursor.pos; i--)
		CINST.fdata.data[i] = CINST.fdata.data[i - 1];
	
	// type char
	chx_type_ascii(_c);
	
	// update screen
	chx_draw_all();
	fflush(stdout);
}

void chx_delete_ascii() {
	// only delete if cursor is before EOF
	if (CINST.cursor.pos < CINST.fdata.len) {
		chx_set_ascii(0);
	
		// highlight unsaved change
		CINST.saved = 0;
		CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	}
}

void chx_backspace_ascii() {
	chx_cursor_prev_byte();
	chx_delete_ascii();
}

void chx_remove_ascii() {
	// only remove characters in the file
	if (CINST.cursor.pos < CINST.fdata.len) {
		// shift bytes after cursor left by one
		CINST.cursor.sbpos = 0;
		for (int i = CINST.cursor.pos; i < CINST.fdata.len - 1; i++)
			CINST.fdata.data[i] = CINST.fdata.data[i + 1];
		
		// resize file
		chx_resize_file(CINST.fdata.len - 1);
		
		// redraw contents and update cursor
		chx_draw_all();
		fflush(stdout);
	}
}

void chx_erase_ascii() {
	if (CINST.cursor.pos) {
		CINST.cursor.sbpos = 0;
		chx_cursor_prev_byte();
		chx_remove_ascii();
	}
}

void chx_main() {
	for(struct chx_key key;; key = chx_get_key()) {
		// execute key sequence, if available
		if (chx_keybinds_global[WORD(key)]) {
			chx_keybinds_global[WORD(key)]();
			
			// make sure function is not in exclusion list, if so then set the last action to the function pointer
			char is_valid = 1;
			for (int i = 0; i < sizeof(func_exceptions) / sizeof(void*); i++)
				if (chx_keybinds_global[WORD(key)] == func_exceptions[i])
					is_valid = 0;
			if (is_valid) {
				CINST.last_action.action.execute_void = chx_keybinds_global[WORD(key)];
				CINST.last_action.type = 0;
			}
		}
		
		switch (CINST.mode) {
			default:
			case CHX_MODE_DEFAULT:
				// execute key sequence, if available
				if (chx_keybinds_mode_command[WORD(key)]) {
					chx_keybinds_mode_command[WORD(key)]();
			
					// make sure function is not in exclusion list, if so then set the last action to the function pointer
					char is_valid = 1;
					for (int i = 0; i < sizeof(func_exceptions) / sizeof(void*); i++)
						if (chx_keybinds_mode_command[WORD(key)] == func_exceptions[i])
							is_valid = 0;
					if (is_valid) {
						CINST.last_action.action.execute_void = chx_keybinds_mode_command[WORD(key)];
						CINST.last_action.type = 0;
					}
				}
				break;
			case CHX_MODE_INSERT:
				if (IS_CHAR_HEX(WORD(key))) chx_insert_hexchar(WORD(key));
				else if (WORD(key) == 0x7F) chx_erase_hexchar();
				break;
			case CHX_MODE_REPLACE:
				if (IS_CHAR_HEX(WORD(key))) chx_set_hexchar(WORD(key));
				else if (WORD(key) == 0x7F) chx_delete_hexchar();
				break;
			case CHX_MODE_TYPE:
				if (IS_CHAR_HEX(WORD(key))) chx_type_hexchar(WORD(key));
				else if (WORD(key) == 0x7F) chx_backspace_hexchar();
				break;
			case CHX_MODE_INSERT_ASCII:
				if (IS_PRINTABLE(WORD(key))) chx_insert_ascii(WORD(key));
				else if (WORD(key) == 0x7F) chx_erase_ascii();
				break;
			case CHX_MODE_REPLACE_ASCII:
				if (IS_PRINTABLE(WORD(key))) chx_set_ascii(WORD(key));
				else if (WORD(key) == 0x7F) chx_delete_ascii();
				break;
			case CHX_MODE_TYPE_ASCII:
				if (IS_PRINTABLE(WORD(key))) chx_type_ascii(WORD(key));
				else if (WORD(key) == 0x7F) chx_backspace_ascii();
				break;
		}
		
		// clear selection if cursor is not at the end of the selection (meaning the user is no longer selecting and the cursor has moved)
		if (CINST.selected && CINST.cursor.pos != CINST.sel_stop) chx_clear_selection();
	}
}

struct chx_key chx_get_key() {
		struct chx_key key;
        char buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
		// set terminal flags and enter raw mode
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
		
        read(0, &buf, 8);
		
		// check the type of key press (ALT, ESCAPE, etc.)
		switch (buf[0]) {
			case 0x1B:
				if (buf[5] == '~') key = (struct chx_key) {buf[2], buf[4] - 0x30};
				else if (buf[5]) key = (struct chx_key) {buf[5], buf[4] - 0x30};
				else if (buf[1] == 0x5B || !buf[1]) key = (struct chx_key) {buf[2], 0x01};
				else key = (struct chx_key) {buf[1], 0x03};
				break;
			default:
				key = (struct chx_key) {buf[0], 0};
				break;
		}
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        tcsetattr(0, TCSADRAIN, &old);
		
        return key;
}

char chx_get_char() {
		char buf;
		
		// set terminal flags and enter raw mode
        struct termios old = {0};
        tcgetattr(0, &old);
        old.c_lflag &= ~ICANON;
        old.c_lflag |= ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        tcsetattr(0, TCSANOW, &old);
		
        read(0, &buf, 1);
		
		// restore flags and re-enter cooked mode
        old.c_lflag |= ICANON;
        old.c_lflag &= ~ECHO;
        tcsetattr(0, TCSADRAIN, &old);
		
        return buf;
}

void chx_get_str(char* _buf, int _len) {
	struct chx_key k;
	for (int bpos = 0; WORD(k) != 0x0A; k = chx_get_key()) {
		if (WORD(k) == KEY_LEFT && bpos > 0) {
			bpos--;
			printf("\e[1D");
		} else if (WORD(k) == KEY_RIGHT && _buf[bpos]) {
			bpos++;
			printf("\e[1C");
		} else if (WORD(k) == 0x7F && bpos) {
			bpos--;
			int n;
			for (n = bpos; _buf[n]; n++)
				_buf[n] = _buf[n + 1];
			printf("\e[1D");
			for (n = bpos; _buf[n]; n++)
				printf("%c", _buf[n]);
			printf("\e[0K");
			if (n - bpos) printf("\e[%iD", n - bpos);
			
		} else if (IS_PRINTABLE(WORD(k))) {
			int n = 0;
			while (_buf[n]) n++;
			for (n++; n > 0; n--)
				_buf[bpos + n] = _buf[bpos + n - 1];
			_buf[bpos] = k.val;
			for (n = bpos; _buf[n]; n++)
				printf("%c", _buf[n]);
			printf("\e[0K");
			printf("\e[%iD", n - bpos);
			bpos++;
			printf("\e[1C");
		} else if (WORD(k) == KEY_ESCAPE) {
			for (int i = 0; _buf[i]; i++)
				_buf[i] = 0;
			return;
		}
		fflush(stdout);
	}
}

void chx_add_instance(char* fpath) {
	if (CHX_CUR_MAX_INSTANCE >= CHX_MAX_NUM_INSTANCES - 1) return;
	
	// load file
	struct chx_finfo hdata = chx_import(fpath);
	if (!hdata.data) {
		// alert user file could not be found and wait for key input to continue
		cur_set(0, CINST.height);
		printf("\e[2Kfile '%s' not found.", fpath);
		fflush(stdout);
		chx_get_char();
		return;
	};
	
	hdata.filename = memfork(fpath, str_len(fpath) + 1);
	
	// get window dimensions
	struct winsize size;
	ioctl(0, TIOCGWINSZ, (char*) &size);
	
	// setup instance
	CHX_CUR_MAX_INSTANCE++;
	CHX_SEL_INSTANCE = CHX_CUR_MAX_INSTANCE;
	CINST.cursor = (struct chx_cursor) {0};
	CINST.fdata = hdata;
	CINST.style_data = calloc(1, hdata.len / 8 + (hdata.len % 8 != 0));
	CINST.height = size.ws_row;
	CINST.width = size.ws_col;
	CINST.x_offset = 0;
	CINST.y_offset = 0;
	CINST.bytes_per_row = CHX_BYTES_PER_ROW;
	CINST.bytes_in_group = CHX_BYTES_IN_GROUP;
	CINST.group_spacing = CHX_GROUP_SPACING;
	CINST.min_row_num_len =
	CINST.row_num_len = CHX_MIN_ROW_NUM_LEN;
	CINST.num_rows = size.ws_row - PD;
	CINST.endianness = CHX_DEFAULT_ENDIANNESS;
	CINST.last_action.action.execute_void = fvoid;
	CINST.last_action.params = NULL;
	CINST.last_action.params_raw = NULL;
	CINST.last_action.type = 0;
	CINST.copy_buffer = NULL;
	CINST.saved = 1;
	
	CINST.show_inspector = (CHX_PREVIEW_END + 28 > CINST.width) ? 0 : CHX_SHOW_INSPECTOR_ON_STARTUP;
	CINST.show_preview = (CHX_PREVIEW_END > CINST.width) ? 0 : CHX_SHOW_PREVIEW_ON_STARTUP;
}

void chx_remove_instance(int _n) {
	if (_n < 0 || _n > CHX_CUR_MAX_INSTANCE || !CHX_CUR_MAX_INSTANCE) return;
	
	CHX_CUR_MAX_INSTANCE--;
	if (CHX_SEL_INSTANCE > CHX_CUR_MAX_INSTANCE)
		CHX_SEL_INSTANCE = CHX_CUR_MAX_INSTANCE;
	
	// empty struct
	free(CHX_INSTANCES[_n].copy_buffer);
	free(CHX_INSTANCES[_n].style_data);
	free(CHX_INSTANCES[_n].fdata.data);
	free(CHX_INSTANCES[_n].fdata.filename);
	
	CHX_INSTANCES[_n] = (struct CHX_INSTANCE) {0};
	
	// shift instances to remove spaces
	for (int i = 0; i < CHX_MAX_NUM_INSTANCES - 1; i++)
		if (!CHX_INSTANCES[i].style_data)
			CHX_INSTANCES[i] = CHX_INSTANCES[i + 1];
}

int main(int argc, char** argv) {
	// allow printing of wide chars
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
			printf("CAOIMH HEX EDITOR version 1.0.1\n");
			return 0;
		}
	}
	
	// load file
	struct chx_finfo hdata = chx_import(argv[1]);
	hdata.filename = memfork(argv[1], str_len(argv[1]) + 1);
	if (!hdata.data) {
		printf("file \"%s\" not found.\n", argv[1]);
		return 1;
	}
	
	// enter new terminal state
	tenter();
	cls();
	
	// disable key echoing
	struct termios old = {0};
	tcgetattr(0, &old);
	old.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &old);
	
	// override ctrl+z/ctrl+c termination
	signal(SIGINT, chx_quit);
	signal(SIGTSTP, chx_quit);
	
	// setup initial instance
	CHX_CUR_MAX_INSTANCE = -1;
	CHX_SEL_INSTANCE = 0;
	CHX_INSTANCES = (struct CHX_INSTANCE*) calloc(sizeof(struct CHX_INSTANCE), CHX_MAX_NUM_INSTANCES);
	
	chx_add_instance(argv[1]);
	
	// only show preview or inspector if it will fit on the screen
	CINST.show_inspector = (CHX_PREVIEW_END + 28 > CINST.width) ? 0 : CHX_SHOW_INSPECTOR_ON_STARTUP;
	CINST.show_preview = (CHX_PREVIEW_END > CINST.width) ? 0 : CHX_SHOW_PREVIEW_ON_STARTUP;
	
	// if the screen cannot fit the contents, remove one byte until it can be displayed
	while (CHX_CONTENT_END > CINST.width && CINST.bytes_per_row) {
		CINST.bytes_in_group = 1;
		CINST.bytes_per_row--;
	}
	
	// alert user if application cannot be functionally operated
	if (CINST.num_rows < 2 || !CINST.bytes_per_row)
		chx_exit_with_message("terminal is too small to run application.\n");
	
	// draw elements
	chx_draw_all();
	
	// call main loop
	chx_main();
	
	texit();
	return 0;
}