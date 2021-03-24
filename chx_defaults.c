void chx_type_mode_toggle() {
	if (CINST.mode == CHX_MODE_TYPE) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_TYPE;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_insert_mode_toggle() {
	if (CINST.mode == CHX_MODE_INSERT) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_INSERT;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_replace_mode_toggle() {
	if (CINST.mode == CHX_MODE_REPLACE) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_REPLACE;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_type_ascii_mode_toggle() {
	if (CINST.mode == CHX_MODE_TYPE_ASCII) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_TYPE_ASCII;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_insert_ascii_mode_toggle() {
	if (CINST.mode == CHX_MODE_INSERT_ASCII) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_INSERT_ASCII;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_replace_ascii_mode_toggle() {
	if (CINST.mode == CHX_MODE_REPLACE_ASCII) CINST.mode = CHX_MODE_DEFAULT;
	else CINST.mode = CHX_MODE_REPLACE_ASCII;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_move_vertical_by(int _n) {
	int new_pos = CINST.cursor.pos + _n * CINST.bytes_per_row;
	CINST.cursor.pos = (new_pos >= CINST.bytes_per_row) ? new_pos : CINST.cursor.pos % CINST.bytes_per_row;
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

void chx_cursor_prev_byte() {
	CINST.cursor.pos--;
	CINST.cursor.sbpos = 0;
	chx_update_cursor();
}

void chx_cursor_next_byte() {
	CINST.cursor.pos++;
	CINST.cursor.sbpos = 0;
	chx_update_cursor();
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
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_select_up() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_up();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_select_down() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_down();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_select_right() {
	if (!CINST.selected) chx_start_selection();
	chx_cursor_move_right();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_select_left() {
	if (CINST.cursor.pos || CINST.cursor.sbpos)
		if (!CINST.selected) chx_start_selection();
	chx_cursor_move_left();
	CINST.sel_stop = CINST.cursor.pos;
	chx_draw_contents();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
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
	if (!(_s[0] == '0' && _s[1] == 'x')) return 0;
	for (int i = 2; _s[i]; i++) if (!IS_CHAR_HEX(_s[i])) return 0;
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
	for (int i = 2; _s[i]; i++) {
		total *= 16;
		if ((_s[i] ^ 0x60) < 7) _s[i] -= 32;
		total += (_s[i] > 0x40) ? _s[i] - 0x37 : _s[i] - 0x30;
	}
	return total;
}

void chx_resize_file(long _n) {
	CINST.fdata.data = recalloc(CINST.fdata.data, CINST.fdata.len, _n);
	CINST.style_data = recalloc(CINST.style_data, (CINST.fdata.len - 1) / 8 + 1, _n / 8 + 1);
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
}

void chx_to_end() {
	CINST.cursor.pos = CINST.fdata.len - 1;
	CINST.cursor.sbpos = 1;
	int new_scroll = (CINST.cursor.pos / CINST.bytes_per_row) - CINST.num_rows / 2;
	CINST.scroll_pos = (new_scroll >= 0) ? new_scroll : 0;
	chx_draw_all();
}

void chx_toggle_inspector() {
	CINST.show_inspector = !CINST.show_inspector;
	chx_draw_all();
}

void chx_toggle_preview() {
	CINST.show_preview = !CINST.show_preview;
	chx_draw_all();
}

void chx_mode_set_type() {
	CINST.mode = CHX_MODE_TYPE;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_mode_set_insert() {
	CINST.mode = CHX_MODE_INSERT;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_mode_set_replace() {
	CINST.mode = CHX_MODE_REPLACE;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_mode_set_type_ascii() {
	CINST.mode = CHX_MODE_TYPE_ASCII;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_mode_set_insert_ascii() {
	CINST.mode = CHX_MODE_INSERT_ASCII;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_mode_set_replace_ascii() {
	CINST.mode = CHX_MODE_REPLACE_ASCII;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_mode_set_default() {
	CINST.mode = CHX_MODE_DEFAULT;
	chx_print_status();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
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
	for (int i = 0; i < CINST.fdata.len / 8; i++)
		CINST.style_data[i] = 0;
	
	// export file and redraw file contents
	CINST.saved = 1;
	chx_export(CINST.fdata.filename);
	chx_draw_contents();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

char cmp_str(char* _a, char* _b) {
	for (int i = 0; _a[i] || _b[i]; i++)
		if (_a[i] != _b[i]) return 0;
	return 1;
}

char* chx_extract_param(char* _s, int _n) {
	int n;
	// extract param
	char* param = _s;
	for (int i = 0; i < _n; i++) {
		for (n = 0; param[n] > 0x20 && param[n] < 0x7F; n++);
		param += n + 1;
	}
	
	// terminate param at first non-typable char or space (' ', '\n', '\t', etc.)
	for (n = 0; param[n] > 0x20 && param[n] < 0x7F; n++);
	param[n] = 0;
	return param;
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
	char* filename = chx_extract_param(usrin, 0);
	
	// only export if filename was entered
	if (filename[0]) {
		chx_export(filename);
		CINST.saved = 1;
		for (int i = 0; i < CINST.fdata.len / 8; i++)
			CINST.style_data[i] = 0;
	}
	
	// redraw elements
	cls();
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
	for (int i = 0; i < CINST.copy_buffer_len && CINST.cursor.pos - i > 0; i++) {
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
	if (CINST.cursor.pos + CINST.copy_buffer_len > CINST.scroll_pos * CINST.bytes_per_row + CINST.num_rows * CINST.bytes_per_row)
		CINST.scroll_pos = (CINST.cursor.pos + CINST.copy_buffer_len - CINST.num_rows * CINST.bytes_per_row) / CINST.bytes_per_row + 1;
	
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
	CINST.cursor.sbpos = 0;
	chx_draw_contents();
	chx_update_cursor();
}

void chx_clear_buffer() {
	free(CINST.copy_buffer);
	CINST.copy_buffer = 0;
	CINST.copy_buffer_len = 0;
}

void chx_remove_selected() {
	if (CINST.selected) {
		long sel_begin = min(CINST.sel_start, CINST.sel_stop);
		long sel_end = max(CINST.sel_start, CINST.sel_stop);
		long sel_size = sel_end - sel_begin + 1;
		CINST.saved = 0;
		if (sel_end > CINST.fdata.len - 1)
			chx_resize_file(sel_begin);
		else {
			for (int i = sel_end + 1; i < CINST.fdata.len; i++)
				CINST.fdata.data[i - sel_size] = CINST.fdata.data[i];
			chx_resize_file(CINST.fdata.len - sel_size);
		}
		CINST.cursor.pos = (sel_begin > 0) ? sel_begin - 1 : 0;
		CINST.cursor.sbpos = 1;
		chx_clear_selection();
		chx_draw_all();
	}
}

void chx_delete_selected() {
	if (CINST.selected) {
		long sel_begin = min(CINST.sel_start, CINST.sel_stop);
		long sel_end = max(CINST.sel_start, CINST.sel_stop);
		CINST.saved = 0;
		if (sel_end > CINST.fdata.len - 1)
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
				// erase save dialoge and redraw elements
				cls();
				chx_draw_all();
				chx_main();
				break;
			case 'n':
			case 'N':
				break;
		}
	}
	
	chx_exit();
}