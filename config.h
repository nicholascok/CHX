#define CHX_CTRL(C) (C & 0x1F)
#define CHX_ALT(C) ((C & 0x00FF) | 0x0300)
#define CHX_SHIFT(C) ((C & 0x00FF) | 0x0200)
#define CHX_CTRL_M(C) ((C & 0x00FF) | 0x0500)

/* GENERAL SETTINGS */
#define CHX_DEFAULT_ENDIANNESS CHX_LITTLE_ENDIAN

/* LAYOUT SETTINGS */
#define CHX_FRAME_COLOUR COLOUR_CYAN
#define CHX_UNSAVED_COLOUR "\033[38;2;0;240;240m"
#define CHX_ASCII_SELECT_COLOUR FORMAT_UNDERLINE
#define CHX_SELECT_COLOUR COLOUR_REVERSE

#define CHX_BYTES_PER_ROW 16
#define CHX_BYTES_IN_GROUP 1
#define CHX_GROUP_SPACING 1
#define CHX_ROW_NUM_LEN 8

/* FEATURES (COMMENT TO DISABLE) */
#define CHX_RESIZE_FILE_ON_BACKSPACE
#define CHX_RESIZE_FILE_ON_INSERTION
#define CHX_SHOW_PREVIEW
#define CHX_SHOW_INSPECTOR

/* GLOBAL KEYBINDS (WORK IN ANY MODE) */
void (*chx_keybinds_global[])(void) = {
	[KEY_MAX_VAL] = fvoid, // do not remove
	[KEY_ESCAPE] = chx_mode_set_default,
	[KEY_UP] = chx_cursor_move_up,
	[KEY_DOWN] = chx_cursor_move_down,
	[KEY_RIGHT] = chx_cursor_move_right,
	[KEY_LEFT] = chx_cursor_move_left,
	[CHX_SHIFT(KEY_UP)] = chx_cursor_select_up,
	[CHX_SHIFT(KEY_DOWN)] = chx_cursor_select_down,
	[CHX_SHIFT(KEY_RIGHT)] = chx_cursor_select_right,
	[CHX_SHIFT(KEY_LEFT)] = chx_cursor_select_left,
	[CHX_CTRL_M(KEY_UP)] = chx_cursor_move_up_by_5,
	[CHX_CTRL_M(KEY_DOWN)] = chx_cursor_move_down_by_5,
	[CHX_CTRL_M(KEY_RIGHT)] = chx_cursor_move_right_by_5,
	[CHX_CTRL_M(KEY_LEFT)] = chx_cursor_move_left_by_5,
	[CHX_ALT(KEY_UP)] = chx_cursor_move_up,
	[CHX_ALT(KEY_DOWN)] = chx_cursor_move_down,
	[CHX_ALT(KEY_RIGHT)] = chx_cursor_next_byte,
	[CHX_ALT(KEY_LEFT)] = chx_cursor_prev_byte,
	[CHX_CTRL('y')] = chx_copy,
	[CHX_CTRL('p')] = chx_paste_after,
	[CHX_CTRL('P')] = chx_paste_before,
	[CHX_CTRL('t')] = chx_type_mode_toggle,
	[CHX_CTRL('i')] = chx_insert_mode_toggle,
	[CHX_CTRL('r')] = chx_replace_mode_toggle,
	[CHX_CTRL('e')] = chx_swap_endianness,
	[CHX_CTRL('s')] = chx_save,
	[CHX_CTRL('w')] = chx_save_as,
	[CHX_CTRL('u')] = chx_revert,
	[CHX_CTRL('x')] = chx_exit,
	[CHX_CTRL('q')] = chx_exit,
	[CHX_CTRL('z')] = chx_quit,
	[KEY_DELETE] = chx_delete_hexchar,
	['^'] = chx_to_line_start,
	['$'] = chx_to_line_end,
	[':'] = chx_prompt_command,
};

/* COMMAND MODE KEYBINDS */
void (*chx_keybinds_mode_command[])(void) = {
	[KEY_MAX_VAL] = fvoid, // do not remove
	['k'] = chx_cursor_move_up,
	['j'] = chx_cursor_move_down,
	['l'] = chx_cursor_move_right,
	['h'] = chx_cursor_move_left,
	['K'] = chx_cursor_select_up,
	['J'] = chx_cursor_select_down,
	['L'] = chx_cursor_select_right,
	['H'] = chx_cursor_select_left,
	[CHX_CTRL('k')] = chx_cursor_move_up_by_5,
	[CHX_CTRL('j')] = chx_cursor_move_down_by_5,
	[CHX_CTRL('l')] = chx_cursor_move_right_by_5,
	[CHX_CTRL('h')] = chx_cursor_move_left_by_5,
	[CHX_ALT('k')] = chx_cursor_move_up,
	[CHX_ALT('j')] = chx_cursor_move_down,
	[CHX_ALT('l')] = chx_cursor_next_byte,
	[CHX_ALT('h')] = chx_cursor_prev_byte,
	['u'] = chx_revert,
	['y'] = chx_copy,
	['P'] = chx_paste_before,
	['p'] = chx_paste_after,
	['D'] = chx_remove_selected,
	['d'] = chx_delete_selected,
	['x'] = chx_delete_hexchar,
	['t'] = chx_mode_set_type,
	['i'] = chx_mode_set_insert,
	['r'] = chx_mode_set_replace,
	['g'] = chx_to_start,
	['q'] = chx_quit,
	['G'] = chx_to_end,
	['.'] = chx_execute_last_action,
};

/* INTERPRETER COMMANDS */
struct chx_command chx_commands[] = {
	(struct chx_command) {chx_swap_endianness, "se"},
	(struct chx_command) {chx_save, "w"},
	(struct chx_command) {chx_save_as, "saveas"},
	(struct chx_command) {chx_save_as, "sav"},
	(struct chx_command) {chx_exit, "q!"},
	(struct chx_command) {chx_quit, "q"},
	(struct chx_command) {chx_save_and_quit, "wq"},
	(struct chx_command) {chx_save_and_quit, "x"},
	(struct chx_command) {chx_to_start, "0"},
	(struct chx_command) {0, 0} // do not remove
};

/* FUNCTIONS TO EXCLUDE FROM ACTION HISTORY */
void (*func_exceptions[])(void) = {
	chx_cursor_move_up,
	chx_cursor_move_down,
	chx_cursor_move_right,
	chx_cursor_move_left,
	chx_cursor_select_up,
	chx_cursor_select_down,
	chx_cursor_select_right,
	chx_cursor_select_left,
	chx_execute_last_action,
	chx_prompt_command,
};