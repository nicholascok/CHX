void chx_insert_mode_toggle() {
	if (CINST.mode == CHX_MODE_INSERT) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_INSERT;
	chx_print_status();
}

void chx_replace_mode_toggle() {
	if (CINST.mode == CHX_MODE_REPLACE) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_REPLACE;
	chx_print_status();
}

void chx_update_cursor() {
	// stop cursor at 0;
	CINST.cursor.pos *= (CINST.cursor.pos >= 0);
	
	// scroll if pasting outside of visible screen
	if (CINST.cursor.pos > (CINST.scroll_pos - 1) * CINST.bytes_per_row + CINST.num_bytes) {
		CINST.scroll_pos = (CINST.cursor.pos - CINST.num_bytes) / CINST.bytes_per_row + 1;
		chx_draw_contents();
	} else if (CINST.cursor.pos < CINST.scroll_pos * CINST.bytes_per_row) {
		CINST.scroll_pos = (CINST.cursor.pos / CINST.bytes_per_row > 0) ? CINST.cursor.pos / CINST.bytes_per_row : 0;
		chx_draw_contents();
	}
	
	// redraw cursor
	chx_draw_extra();
	chx_draw_sidebar();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_swap_endianness() {
	CINST.endianness = ! CINST.endianness;
	chx_draw_extra();
	fflush(stdout);
}

void chx_cursor_move_vertical_by(int _n) {
	CINST.cursor.pos += _n * CINST.bytes_per_row;
	chx_update_cursor();
}

void chx_cursor_move_horizontal_by(int _n) {
	CINST.cursor.pos += _n;
	chx_update_cursor();
}

void chx_cursor_move_up_by_5() {
	chx_cursor_move_vertical_by(-5);
}

void chx_cursor_move_down_by_5() {
	chx_cursor_move_vertical_by(5);
}

void chx_cursor_move_right_by_5() {
	chx_cursor_move_horizontal_by(5);
}

void chx_cursor_move_left_by_5() {
	chx_cursor_move_horizontal_by(-5);
}

void chx_cursor_move_up() {
	CINST.cursor.pos -= (CINST.cursor.pos >= CINST.bytes_per_row) * CINST.bytes_per_row;
	chx_update_cursor();
}

void chx_cursor_move_down() {
	CINST.cursor.pos += CINST.bytes_per_row;
	chx_update_cursor();
}

void chx_cursor_move_right() {
	CINST.cursor.pos += (CINST.cursor.sbpos == 1);
	CINST.cursor.sbpos = !CINST.cursor.sbpos;
	chx_update_cursor();
}

void chx_cursor_move_left() {
	CINST.cursor.pos -= !CINST.cursor.sbpos;
	CINST.cursor.sbpos = !CINST.cursor.sbpos;
	chx_update_cursor();
}

void chx_start_selection() {
	CINST.sel_start = CINST.cursor.pos;
	CINST.sel_stop = CINST.cursor.pos;
	CINST.selected = 1;
}

void chx_clear_selection() {
	CINST.selected = 0;
	chx_draw_contents();
	fflush(stdout);
}

void chx_cursor_select_up() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_up();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	fflush(stdout);
}

void chx_cursor_select_down() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_down();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	fflush(stdout);
}

void chx_cursor_select_right() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_right();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	fflush(stdout);
}

void chx_cursor_select_left() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_left();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	fflush(stdout);
}

long chx_abs(long _n) {
	return _n + 2 * _n * -(_n < 0);
}

long min(long _a, long _b) {
	if (_a < _b) return _a;
	return _b;
}

long max(long _a, long _b) {
	if (_a > _b) return _a;
	return _b;
}

char* recalloc(char* _p, long _o, long _n) {
	char* ptr = calloc(1, _n);
	for (int i = 0; i < min(_o, _n); i++) ptr[i] = _p[i];
	free(_p);
	return ptr;
}

char str_is_num(char* _s) {
	for (int i = 0; _s[i]; i++) if (!IS_DIGIT(_s[i])) return 0;
	return 1;
}

char str_is_hex(char* _s) {
	for (int i = 0; _s[i]; i++) if (!IS_CHAR_HEX(_s[i])) return 0;
	return 1;
}

int str_to_num(char* _s) {
	int total = 0;
	for (int i = 0; _s[i]; i++)
		total = total * 10 + _s[i] - 0x30;
	return total;
}

int str_to_hex(char* _s) {
	int total = 0;
	for (int i = 0; _s[i]; i++) {
		total *= 16;
		if ((_s[i] ^ 0x60) < 7) _s[i] -= 32;
		total += (_s[i] > 0x40) ? _s[i] - 0x37 : _s[i] - 0x30;
	}
	return total;
}

void chx_resize_file(long _n) {
	CINST.fdata.data = recalloc(CINST.fdata.data, CINST.fdata.len, _n);
	CINST.style_data = recalloc(CINST.style_data, CINST.fdata.len / 8 + 1, (_n) / 8 + 1);
	CINST.fdata.len = _n;
}

void chx_to_line_start() {
	CINST.cursor.pos -= CINST.cursor.pos % CINST.bytes_per_row;
	CINST.cursor.sbpos = 0;
	chx_update_cursor();
}

void chx_to_line_end() {
	CINST.cursor.pos -= CINST.cursor.pos % CINST.bytes_per_row - CINST.bytes_per_row + 1;
	CINST.cursor.sbpos = 0;
	chx_update_cursor();
}

void chx_to_start() {
	CINST.cursor.pos = 0;
	CINST.cursor.sbpos = 0;
	CINST.scroll_pos = 0;
	chx_draw_all();
	fflush(stdout);
}

void chx_to_end() {
	CINST.cursor.pos = CINST.fdata.len - 1;
	CINST.cursor.sbpos = 1;
	int new_scroll = (CINST.cursor.pos / CINST.bytes_per_row) - CINST.num_rows / 2;
	CINST.scroll_pos = (new_scroll >= 0) ? new_scroll : 0;
	chx_draw_all();
	fflush(stdout);
}

void chx_set_hexchar(char _c) {
	if (!IS_CHAR_HEX(_c)) return; // only accept hex characters
	if ((_c ^ 0x60) < 7) _c -= 32; // ensure everything is upper-case
	printf("\e[31m%c\e[0m", _c); // print the character on the screen
	
	char nullkey[2] = {_c, 0};
	
	// resize file if typing past current file length
	if (CINST.cursor.pos >= CINST.fdata.len) {
		chx_resize_file(CINST.cursor.pos + 1);
		chx_draw_contents();
	}
	
	// update stored file data
	CINST.fdata.data[CINST.cursor.pos] &= 0x0F << (CINST.cursor.sbpos * 4);
	CINST.fdata.data[CINST.cursor.pos] |= strtol(nullkey, NULL, 16) << (!CINST.cursor.sbpos * 4);
	
	// highlight unsaved changes
	CINST.saved = 0;
	CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	chx_redraw_line(CINST.cursor.pos);
	fflush(stdout);
}

void chx_type_hexchar(char _c) {
	chx_set_hexchar(_c);
	chx_cursor_move_right();
}

void chx_delete_hexchar() {
	CINST.saved = 0;
	
	// only delete if cursor is before EOF
	if (CINST.cursor.pos < CINST.fdata.len)
		if (CINST.cursor.sbpos)
			CINST.fdata.data[CINST.cursor.pos] &= 0xF0;
		else
			CINST.fdata.data[CINST.cursor.pos] &= 0x0F;
	
	// hightlight as unsaved change
	CINST.style_data[CINST.cursor.pos / 8] |= 0x80 >> (CINST.cursor.pos % 8);
	
	// update screen
	chx_redraw_line(CINST.cursor.pos);
}

void chx_backspace_hexchar() {
	CINST.saved = 0;
	
	// if cursor is just after EOF, resize file to remove last byte
	if (CINST.cursor.pos == CINST.fdata.len && !CINST.cursor.sbpos) {
		chx_resize_file(CINST.fdata.len - 1);
		CINST.cursor.pos--;
	} else
		chx_delete_hexchar();
	
	// move cursor after removing char
	chx_cursor_move_left();
	chx_redraw_line(CINST.cursor.pos);
}

void chx_mode_set_insert() {
	CINST.mode = CHX_MODE_INSERT;
	chx_print_status();
}

void chx_mode_set_replace() {
	CINST.mode = CHX_MODE_REPLACE;
	chx_print_status();
}

void chx_mode_set_default() {
	CINST.mode = CHX_MODE_DEFAULT;
	chx_print_status();
}

void chx_revert() {
	// reload file to remove unsaved changes
	char* old_filename = CINST.fdata.filename;
	CINST.fdata = chx_import(old_filename);
	CINST.fdata.filename = old_filename;
	CINST.saved = 1;
	
	// remove highlighting for unsaved data
	free(CINST.style_data);
	CINST.style_data = calloc(1, CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0));
	
	// redraw elements
	chx_draw_all();
}

void chx_save() {
	// remove highlighting for unsaved data
	for (int i = 0; i < CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0); i++)
		CINST.style_data[i] = 0;
	
	// export file and redraw file contents
	CINST.saved = 1;
	chx_export(CINST.fdata.filename);
	chx_draw_contents();
	fflush(stdout);
}

char cmp_str(char* _a, char* _b) {
	for (int i = 0; _a[i] || _b[i]; i++)
		if (_a[i] != _b[i]) return 0;
	return 1;
}

void str_terminate_at(char* _s, char _c) {
	int n = 0;
	for (;_s[n] && _s[n] != _c; n++);
	_s[n] = 0;
}

void chx_save_as() {
	chx_draw_all();
	
	// setup user input buffer
	char usrin[256];
	
	// print save dialoge and recieve user input
	cur_set(0, CINST.height);
	printf("SAVE AS? (LEAVE EMPTY TO CANCEL): ");
	fflush(stdout);
	
	fgets(usrin, 256, stdin);
	
	// null terminate input at first newline
	str_terminate_at(usrin, '\n');
	
	// only export if filename was entered
	if (usrin[0]) {
		chx_export(usrin);
		CINST.saved = 1;
		for (int i = 0; i < CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0); i++)
			CINST.style_data[i] = 0;
	}
	
	// erase save dialoge
	printf("\e[1A\e[2K");
	fflush(stdout);
	
	// redraw elements
	chx_draw_all();
}

void chx_copy() {
	long sel_begin = min(CINST.sel_start, CINST.sel_stop);
	CINST.copy_buffer_len = chx_abs(CINST.sel_start - CINST.sel_stop) + 1;
	if (CINST.copy_buffer_len + sel_begin > CINST.fdata.len)
		CINST.copy_buffer_len -= CINST.copy_buffer_len + sel_begin - CINST.fdata.len;
	CINST.copy_buffer = malloc(CINST.copy_buffer_len);
	for (int i = 0; i < CINST.copy_buffer_len; i++) CINST.copy_buffer[i] = CINST.fdata.data[sel_begin + i];
}

void chx_execute_last_action() {
	CINST.last_action();
}

void chx_paste_before() {
	CINST.saved = 0;
	
	// scroll if pasting past visible screen
	if (CINST.cursor.pos - CINST.copy_buffer_len < CINST.scroll_pos * CINST.bytes_per_row)
		CINST.scroll_pos = ((CINST.cursor.pos - CINST.copy_buffer_len) / CINST.bytes_per_row > 0) ? (CINST.cursor.pos - CINST.copy_buffer_len) / CINST.bytes_per_row : 0;
	
	// resize file if pasting past end
	if (CINST.cursor.pos > CINST.fdata.len)
		chx_resize_file(CINST.cursor.pos + 1);
	
	// copy data into file buffer
	for (int i = 0; i < CINST.copy_buffer_len && CINST.cursor.pos - i >= 0; i++) {
		CINST.fdata.data[CINST.cursor.pos - i] = CINST.copy_buffer[CINST.copy_buffer_len - i - 1];
		CINST.style_data[(CINST.cursor.pos - i) / 8] |= 0x80 >> ((CINST.cursor.pos - i) % 8);
	}
	
	// move cursor to beginning of pasted data
	CINST.cursor.pos -= CINST.copy_buffer_len;
	CINST.cursor.sbpos = 1;
	chx_draw_contents();
	chx_update_cursor();
}

void chx_paste_after() {
	CINST.saved = 0;
	
	// scroll if pasting past visible screen
	if (CINST.cursor.pos + CINST.copy_buffer_len > CINST.scroll_pos * CINST.bytes_per_row + CINST.num_bytes)
		CINST.scroll_pos = (CINST.cursor.pos + CINST.copy_buffer_len - CINST.num_bytes) / CINST.bytes_per_row + 1;
	
	// resize file if pasting past end
	if (CINST.cursor.pos + CINST.copy_buffer_len > CINST.fdata.len)
		chx_resize_file(CINST.cursor.pos + CINST.copy_buffer_len);
	
	// copy data into file buffer
	for (int i = 0; i < CINST.copy_buffer_len; i++) {
		CINST.fdata.data[CINST.cursor.pos + i] = CINST.copy_buffer[i];
		CINST.style_data[(CINST.cursor.pos + i) / 8] |= 0x80 >> ((CINST.cursor.pos + i) % 8);
	}
	
	// move cursor to end of pasted data
	CINST.cursor.pos += CINST.copy_buffer_len;
	CINST.cursor.sbpos = 1;
	chx_draw_contents();
	chx_update_cursor();
}

void chx_clear_buffer() {
	free(CINST.copy_buffer);
	CINST.copy_buffer = 0;
	CINST.copy_buffer_len = 0;
}

void chx_delete_selected() {
	if (CINST.selected) {
		long sel_begin = min(CINST.sel_start, CINST.sel_stop);
		long sel_end = max(CINST.sel_start, CINST.sel_stop);
		CINST.saved = 0;
		if (sel_end >= CINST.fdata.len - 1)
			chx_resize_file(sel_begin);
		else
			for (int i = sel_begin; i < sel_end + 1; i++) {
				CINST.fdata.data[i] = 0;
				CINST.style_data[i / 8] |= 0x80 >> (i % 8);
			}
		CINST.cursor.pos = (sel_begin > 0) ? sel_begin - 1 : 0;
		CINST.cursor.sbpos = 1;
		chx_clear_selection();
		chx_draw_all();
	}
}

void chx_save_and_quit() {
	chx_export(CINST.fdata.filename);
	texit();
	exit(0);
}

void chx_exit() {
	texit();
	exit(0);
}

void chx_quit() {
	chx_draw_all();
	
	// ask user if they would like to save
	if (!CINST.saved) {
		cur_set(0, CINST.height);
		printf("WOULD YOU LIKE TO SAVE? (Y / N): ");
		fflush(stdout);
		
		switch (chx_get_char()) {
			case 'y':
			case 'Y':
				chx_export(CINST.fdata.filename);
				break;
			default:
				cls();
				chx_main();
				break;
			case 'n':
			case 'N':
				break;
		}
	}
	
	chx_exit();
}