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
	if (CINST.show_preview)
		chx_draw_sidebar();
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
}

void chx_cursor_select_up() {
	if (!CINST.selected) chx_start_selection();
	CINST.cursor.pos -= (CINST.cursor.pos >= CINST.bytes_per_row) * CINST.bytes_per_row;
	CINST.cursor.sbpos = 0;
	CINST.sel_stop = CINST.cursor.pos;
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row + 1);
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	chx_update_cursor();
}

void chx_cursor_select_down() {
	if (!CINST.selected) chx_start_selection();
	CINST.cursor.pos += CINST.bytes_per_row;
	CINST.cursor.sbpos = 0;
	CINST.sel_stop = CINST.cursor.pos;
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row - 1);
	chx_update_cursor();
}

void chx_cursor_select_right() {
	if (!CINST.selected) chx_start_selection();
	CINST.cursor.pos++;
	CINST.cursor.sbpos = 0;
	CINST.sel_stop = CINST.cursor.pos;
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row - 1);
	chx_update_cursor();
}

void chx_cursor_select_left() {
	if (!CINST.selected) chx_start_selection();
	CINST.cursor.pos--;
	CINST.cursor.sbpos = 0;
	CINST.sel_stop = CINST.cursor.pos;
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row + 1);
	chx_redraw_line(CINST.cursor.pos / CINST.bytes_per_row);
	chx_update_cursor();
}

void chx_exit_with_message(char* _msg) {
	// re-enable key echoing
	struct termios old = {0};
	tcgetattr(0, &old);
	old.c_lflag |= ECHO;
	tcsetattr(0, TCSADRAIN, &old);
	
	// exit
	cls();
	cur_set(0, 0);
	texit();
	printf(_msg);
	exit(0);
}

void chx_exit() {
	// re-enable key echoing
	struct termios old = {0};
	tcgetattr(0, &old);
	old.c_lflag |= ECHO;
	tcsetattr(0, TCSADRAIN, &old);
	
	// exit
	cls();
	cur_set(0, 0);
	texit();
	exit(0);
}

void chx_swap_endianness() {
	CINST.endianness = !CINST.endianness;
	if (CINST.show_inspector) {
		chx_draw_extra();
		cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
		fflush(stdout);
	}
}

void chx_set_endianness_global(char _np, char** _pl) {
	if (!_np) return;
	switch (_pl[0][0]) {
		case 'l':
		case 'L':
			for (int i = 0; i <= CHX_CUR_MAX_INSTANCE; i++)
				CHX_INSTANCES[i].endianness = 1;
			break;
		case 'b':
		case 'B':
			for (int i = 0; i <= CHX_CUR_MAX_INSTANCE; i++)
				CHX_INSTANCES[i].endianness = 0;
			break;
		default:
			return;
	}
	
	if (CINST.show_inspector) {
		chx_draw_extra();
		cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
		fflush(stdout);
	}
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

char* memfork(char* _p, int _l) {
	char* np = malloc(_l);
	for (int i = 0; i < _l; i++)
		np[i] = _p[i];
	return np;
}

char* recalloc(char* _p, long _o, long _n) {
	char* ptr = calloc(1, _n);
	for (int i = 0; i < min(_o, _n); i++) ptr[i] = _p[i];
	free(_p);
	return ptr;
}

int chx_count_digits(long _n) {
	int c = 0;
	while ((_n /= 16) >= 1) c++;
	return ++c;
}

char str_is_num(char* _s) {
	if (!_s[0]) return 0;
	for (int i = 0; _s[i]; i++) if (!IS_DIGIT(_s[i])) return 0;
	return 1;
}

char str_is_hex(char* _s) {
	if (!(_s[0] == '0' && _s[1] == 'x')) return 0;
	for (int i = 2; _s[i]; i++) if (!IS_CHAR_HEX(_s[i])) return 0;
	return 1;
}

int str_to_num(char* _s) {
	long total = 0;
	for (int i = 0; _s[i]; i++)
		total = total * 10 + _s[i] - 0x30;
	return total;
}

long str_to_hex(char* _s) {
	long total = 0;
	for (int i = 2; _s[i]; i++) {
		total *= 16;
		if ((_s[i] ^ 0x60) < 7) _s[i] -= 32;
		total += (_s[i] > 0x40) ? _s[i] - 0x37 : _s[i] - 0x30;
	}
	return total;
}

int str_len(char* _s) {
	int c = 0;
	while (_s[c]) c++;
	return c;
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
	char qf = 0;
	for (n = 0; param[n] > 0x20 - qf && param[n] < 0x7F; n++)
		if (IS_QUOTATION(param[n])) qf = !qf;
	
	if (IS_QUOTATION(param[0]) && IS_QUOTATION(param[n - 1])) {
		*(param++ + n - 1) = 0;
	} else {
		param[n] = 0;
	}
	
	return param;
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
	CINST.cursor = (struct chx_cursor) {0};
	chx_update_cursor();
	chx_draw_all();
}

void chx_to_end() {
	CINST.cursor.pos = CINST.fdata.len - 1;
	CINST.cursor.sbpos = 0;
	chx_update_cursor();
	
	if (CINST.cursor.line >= CINST.num_rows)
		CINST.scroll_pos = CINST.cursor.line - CINST.num_rows / 2;
	
	chx_draw_all();
}

void chx_count_instances(char _np, char** _pl) {
	if (!_np) return;
	int len = str_len(_pl[0]);
	char* buf = malloc(len + 1);
	buf[len] = 0;
	
	// count instances in file
	long count = 0;
	for (long i = 0; i <= CINST.fdata.len - len; i++) {
		for (int n = 0; n < len; n++)
			buf[n] = CINST.fdata.data[i + n];
		if (cmp_str(buf, _pl[0])) count++;
	}
	
	free(buf);
	
	// print number of occurances and wait for key input to continue
	cur_set(0, CINST.height);
	printf("\e[2Kfound %li occurances of '%s' in file '%s'", count, _pl[0], CINST.fdata.filename);
	fflush(stdout);
	chx_get_char();
	
	// redraw elements
	chx_draw_all();
}

void chx_switch_file(char _np, char** _pl) {
	if (!_np) return;
	
	// load file
	struct chx_finfo hdata = chx_import(_pl[0]);
	if (!hdata.data) {
		// alert user file could not be found and wait for key input to continue
		cur_set(0, CINST.height);
		printf("\e[2Kfile '%s' not found.", _pl[0]);
		fflush(stdout);
		chx_get_char();
		return;
	};
	hdata.filename = memfork(_pl[0], str_len(_pl[0]) + 1);
	
	// update instance
	free(CINST.fdata.filename);
	CINST.fdata = hdata;
	CINST.saved = 1;
	
	// remove highlighting for unsaved data
	free(CINST.style_data);
	CINST.style_data = calloc(1, CINST.fdata.len / 8 + (CINST.fdata.len % 8 != 0));
	
	// redraw elements
	chx_draw_all();
}

void chx_open_instance(char _np, char** _pl) {
	if (!_np) return;
	chx_add_instance(_pl[0]);
	chx_draw_all();
}

void chx_close_instance(char _np, char** _pl) {
	int inst;
	if (str_is_num(_pl[0])) inst = str_to_num(_pl[0]);
	else inst = CHX_SEL_INSTANCE;
	chx_remove_instance(inst);
	chx_draw_all();
}

void chx_find_next(char _np, char** _pl) {
	if (!_np) return;
	int len = str_len(_pl[0]);
	char* buf = malloc(len + 1);
	buf[len] = 0;
	
	// look for first occurance starting at cursor pos.
	long b = CINST.cursor.pos + 1;
	for (long i = 0; !cmp_str(buf, _pl[0]) && i < CINST.fdata.len; i++, b++) {
		if (b >= CINST.fdata.len) b = 0;
		for (int n = 0; n < len; n++)
			buf[n] = CINST.fdata.data[b + n];
	}
	
	free(buf);
	
	// update cursor pos to start of occurance
	CINST.cursor.pos = b - 1;
	chx_update_cursor();
}

void chx_page_up() {
	if (CINST.scroll_pos > 0) {
		CINST.scroll_pos--;
		#ifdef CHX_SCROLL_SUPPORT
			chx_scroll_down(1);
		#else
			chx_draw_contents();
		#endif
		chx_cursor_move_up();
	}
}

void chx_page_down() {
	CINST.scroll_pos++;
	#ifdef CHX_SCROLL_SUPPORT
		chx_scroll_up(1);
	#else
		chx_draw_contents();
	#endif
	chx_cursor_move_down();
}

void chx_toggle_inspector() {
	CINST.show_inspector = !CINST.show_inspector;
	cls();
	chx_draw_all();
}

void chx_toggle_preview() {
	CINST.show_preview = !CINST.show_preview;
	cls();
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

void chx_save_as(char _np, char** _pl) {
	if (_np) {
		free(CINST.fdata.filename);
		CINST.fdata.filename = memfork(_pl[0], str_len(_pl[0]) + 1);
	}
	
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

void chx_prompt_save_as() {
	chx_draw_all();
	
	// setup user input buffer
	char usrin[256];
	
	// print save dialoge and recieve user input
	cur_set(0, CINST.height);
	printf("\e[2KSAVE AS? (LEAVE EMPTY TO CANCEL): ");
	fflush(stdout);
	
	chx_get_str(usrin, 256);
	
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
	CINST.copy_buffer_len = chx_abs(CINST.sel_start - CINST.sel_stop);
	if (CINST.copy_buffer_len + sel_begin > CINST.fdata.len)
		CINST.copy_buffer_len -= CINST.copy_buffer_len + sel_begin - CINST.fdata.len;
	CINST.copy_buffer = malloc(CINST.copy_buffer_len);
	for (int i = 0; i < CINST.copy_buffer_len; i++) CINST.copy_buffer[i] = CINST.fdata.data[sel_begin + i];
}

void chx_execute_last_action() {
	if (CINST.last_action.type)
		CINST.last_action.action.execute_cmmd(CINST.last_action.num_params, CINST.last_action.params);
	else
		CINST.last_action.action.execute_void();
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

void chx_set_inst(char _np, char** _pl) {
	if (!str_is_num(_pl[0])) return;
	int inst = str_to_num(_pl[0]);
	if (inst <= CHX_CUR_MAX_INSTANCE && inst >= 0)
		CHX_SEL_INSTANCE = inst;
	chx_draw_all();
}

void chx_next_inst() {
	if (CHX_SEL_INSTANCE < CHX_CUR_MAX_INSTANCE) CHX_SEL_INSTANCE++;
	else CHX_SEL_INSTANCE = 0;
	chx_draw_all();
}

void chx_prev_inst() {
	if (CHX_SEL_INSTANCE > 0) CHX_SEL_INSTANCE--;
	else CHX_SEL_INSTANCE = CHX_CUR_MAX_INSTANCE;
	chx_draw_all();
}

void chx_config_layout(char _np, char** _pl) {
	if (_np < 2) return;
	
	char* prop_ptr = 0;
	
	if (cmp_str("rnl", _pl[0]))
		prop_ptr = &CINST.min_row_num_len;
	else if (cmp_str("gs", _pl[0]))
		prop_ptr = &CINST.group_spacing;
	else if (cmp_str("bpr", _pl[0]))
		prop_ptr = &CINST.bytes_per_row;
	else if (cmp_str("big", _pl[0]))
		prop_ptr = &CINST.bytes_in_group;
	
	if (prop_ptr && str_is_num(_pl[1]))
		*prop_ptr = (str_to_num(_pl[1])) ? str_to_num(_pl[1]) : 1;
	
	cls();
	chx_draw_all();
}

void chx_config_layout_global(char _np, char** _pl) {
	for (int i = 0; i <= CHX_CUR_MAX_INSTANCE; i++) {
		chx_config_layout(_np, _pl);
		chx_next_inst();
	}
}

void chx_print_finfo() {
	// count lines
	int nlc = 1;
	int chc = 0;
	for (int i = 0; i < CINST.fdata.len; i++) {
		if (IS_PRINTABLE(CINST.fdata.data[i])) chc++;
		else if (CINST.fdata.data[i] == 0x0A) nlc++;
	}
	
	// print info and for key input to ocntinue
	cur_set(0, CINST.height);
	printf("\e[2K'%s' %liB %iL %iC (offset: %#lx)", CINST.fdata.filename, CINST.fdata.len, nlc, chc, CINST.cursor.pos);
	cur_set(CHX_CURSOR_X, CHX_CURSOR_Y);
	fflush(stdout);
	chx_get_char();
	
	// redraw elements
	chx_draw_all();
}

void chx_remove_selected() {
	if (CINST.selected) {
		long sel_begin = min(CINST.sel_start, CINST.sel_stop);
		long sel_end = max(CINST.sel_start, CINST.sel_stop);
		long sel_size = sel_end - sel_begin;
		CINST.saved = 0;
		if (sel_end > CINST.fdata.len - 1)
			chx_resize_file(sel_begin);
		else {
			for (int i = sel_end + 1; i < CINST.fdata.len; i++)
				CINST.fdata.data[i - sel_size] = CINST.fdata.data[i];
			chx_resize_file(CINST.fdata.len - sel_size);
		}
		CINST.cursor.pos = (sel_begin > 0) ? sel_begin : 0;
		CINST.cursor.sbpos = 0;
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
			for (int i = sel_begin; i < sel_end; i++) {
				CINST.fdata.data[i] = 0;
				CINST.style_data[i / 8] |= 0x80 >> (i % 8);
			}
		CINST.cursor.pos = (sel_begin > 0) ? sel_begin : 0;
		CINST.cursor.sbpos = 0;
		chx_clear_selection();
		chx_draw_all();
	}
}

void chx_save_and_quit() {
	chx_export(CINST.fdata.filename);
	chx_exit();
}

void chx_quit() {
	chx_draw_all();
	
	// ask user if they would like to save
	if (!CINST.saved) {
		cur_set(0, CINST.height);
		printf("\e[2KWOULD YOU LIKE TO SAVE? (Y / N): ");
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