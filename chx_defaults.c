void chx_type_mode_toggle() {
	if (CINST.mode == CHX_MODE_TYPE_HEXCHAR) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_TYPE_HEXCHAR;
	chx_print_status();
}

void chx_update_cursor() {
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_move_up() {
	CINST.cursor.pos -= (CINST.cursor.pos >= CINST.bytes_per_row) * CINST.bytes_per_row;
	if (CINST.scroll_pos && CINST.cursor.pos - CINST.scroll_pos < 0) {
		CINST.scroll_pos -= CINST.bytes_per_row;
		chx_draw_contents();
	}
	chx_update_cursor();
}

void chx_cursor_move_down() {
	CINST.cursor.pos += CINST.bytes_per_row;
	if (CINST.cursor.pos >= CINST.num_bytes + CINST.scroll_pos) {
		CINST.scroll_pos += CINST.bytes_per_row;
		chx_draw_contents();
	}
	chx_update_cursor();
}

void chx_cursor_move_right() {
	CINST.cursor.pos += (CINST.cursor.sbpos == 1);
	CINST.cursor.sbpos = !CINST.cursor.sbpos;
	if (CINST.cursor.pos >= CINST.num_bytes + CINST.scroll_pos) {
		CINST.scroll_pos += CINST.bytes_per_row;
		chx_draw_contents();
	}
	chx_update_cursor();
}

void chx_cursor_move_left() {
	if (CINST.cursor.pos || CINST.cursor.sbpos) {
		CINST.cursor.pos -= !CINST.cursor.sbpos;
		CINST.cursor.sbpos = !CINST.cursor.sbpos;
		if (CINST.cursor.pos - CINST.scroll_pos < 0) {
			CINST.scroll_pos -= CINST.bytes_per_row;
			chx_draw_contents();
		}
	}
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
}

void chx_cursor_select_up() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_up();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
}

void chx_cursor_select_down() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_down();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
}

void chx_cursor_select_right() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_right();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
}

void chx_cursor_select_left() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_left();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
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

void chx_resize_file(long _n) {
	CINST.fdata.data = recalloc(CINST.fdata.data, CINST.fdata.len, _n);
	CINST.style_data = recalloc(CINST.style_data, CINST.fdata.len / 8 + 1, (_n) / 8 + 1);
	CINST.fdata.len = _n;
}

void chx_type_hexchar(char _c) {
	if (!IS_CHAR_HEX(_c)) return; // only accept hex characters
	if ((_c ^ 0x60) < 7) _c -= 32; // ensure everything is upper-case
	printf("\e[1;33m%c\e[0m", _c); // print the character on the screen
	
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
	
	// move cursor after typing a char
	chx_cursor_move_right();
}

void chx_delete_hexchar() {
	if (CINST.cursor.pos + 1 == CINST.fdata.len && !CINST.cursor.sbpos)
		if (!(CINST.fdata.data[CINST.cursor.pos] & 0x0F))
			chx_resize_file(CINST.fdata.len - 1);
		else
			CINST.fdata.data[CINST.cursor.pos] &= 0x0F;
	else if (CINST.cursor.pos < CINST.fdata.len)
		if (CINST.cursor.sbpos)
			CINST.fdata.data[CINST.cursor.pos] &= 0xF0;
		else
			CINST.fdata.data[CINST.cursor.pos] &= 0x0F;
	chx_redraw_line(CINST.cursor.pos);
}

void chx_backspace_hexchar() {
	chx_delete_hexchar();
	chx_cursor_move_left();
}

void chx_mode_set_insert() {
	CINST.mode = CHX_MODE_TYPE_HEXCHAR;
	chx_print_status();
}

void chx_mode_set_default() {
	CINST.mode = CHX_MODE_DEFAULT;
	chx_print_status();
}

void chx_save() {
	chx_export(CINST.fdata.filename);
	CINST.saved = 1;
	for (int i = 0; i < CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0); i++)
		CINST.style_data[i] = 0;
	
	// redraw content to remove unsaved highlights
	chx_draw_contents();
}

char cmp_str(char* _a, char* _b) {
	for (int i = 0; _a[i] || _b[i]; i++)
		if (_a[i] != _b[i]) return 0;
	return 1;
}

void chx_save_as() {
	// setup user input buffer
	char usrin[256];
	
	// print save dialoge and recieve user input
	cur_set(0, CINST.height);
	printf("SAVE AS? (LEAVE EMPTY TO CANCEL): ");
	fflush(stdout);
	
	fgets(usrin, 256, stdin);
	
	// cut input at first newline
	usrin[strcspn(usrin, "\n")] = 0;
	
	// only export if filename was entered
	if (usrin[0]) {
		chx_export(usrin);
		CINST.saved = 1;
		for (int i = 0; i < CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0); i++)
			CINST.style_data[i] = 0;
	}
	
	// erase save dialoge
	printf("\033[1A\033[2K");
	fflush(stdout);
	
	// redraw contents
	chx_draw_contents();
}

void chx_copy() {
	long sel_begin = min(CINST.sel_start, CINST.sel_stop);
	CINST.copy_buffer_len = chx_abs(CINST.sel_start - CINST.sel_stop) + 1;
	if (CINST.copy_buffer_len + sel_begin > CINST.fdata.len)
		CINST.copy_buffer_len -= CINST.copy_buffer_len + sel_begin - CINST.fdata.len;
	CINST.copy_buffer = malloc(CINST.copy_buffer_len);
	for (int i = 0; i < CINST.copy_buffer_len; i++) CINST.copy_buffer[i] = CINST.fdata.data[sel_begin + i];
}

void chx_paste() {
	CINST.saved = 0;
	if (CINST.cursor.pos + CINST.copy_buffer_len > CINST.fdata.len)
		chx_resize_file(CINST.cursor.pos + CINST.copy_buffer_len);
	for (int i = 0; i < CINST.copy_buffer_len; i++) {
		CINST.fdata.data[CINST.cursor.pos + i] = CINST.copy_buffer[i];
		CINST.style_data[(CINST.cursor.pos + i) / 8] |= 0x80 >> ((CINST.cursor.pos + i) % 8);
	}
	CINST.cursor.pos += CINST.copy_buffer_len;
	chx_draw_contents();
}

void chx_delete_selected() {
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
	chx_clear_selection();
	chx_draw_contents();
}

void chx_quit() {
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
	
	// restore terminal state
	texit();
	exit(0);
}