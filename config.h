#define CHX_CTRL(C) (C & 0x1F)
#define CHX_ALT(C) ((C & 0x00FF) | 0x0300)
#define CHX_SHIFT(C) ((C & 0x00FF) | 0x0200)
#define CHX_CTRL_M(C) ((C & 0x00FF) | 0x0500)

/* OPTIMISATION SETTINGS (COMMENT TO DISABLE) */
#define CHX_SCROLL_SUPPORT

/* GENERAL SETTINGS */
#define CHX_DEFAULT_ENDIANNESS CHX_LITTLE_ENDIAN
#define CHX_SHOW_PREVIEW_ON_STARTUP TRUE // can be overridden if screen is small
#define CHX_SHOW_INSPECTOR_ON_STARTUP TRUE // can be overridden if screen is small
#define CHX_MAX_NUM_INSTANCES 8 // max number of files open at a time
#define CHX_MAX_NUM_PARAMS 8 // max number of parameters for interpreter commands

/* LAYOUT SETTINGS */
#define CHX_FRAME_COLOUR COLOUR_CYAN
#define CHX_UNSAVED_COLOUR "\033[38;2;0;240;240m"
#define CHX_ASCII_CUR_FORMAT FORMAT_UNDERLINE
#define CHX_ASCII_SELECT_FORMAT FORMAT_REVERSE
#define CHX_SELECT_FORMAT FORMAT_REVERSE

#define CHX_BYTES_PER_ROW 16
#define CHX_BYTES_IN_GROUP 1
#define CHX_GROUP_SPACING 1
#define CHX_MIN_ROW_NUM_LEN 4

/* FEATURES (COMMENT TO DISABLE) */
#define CHX_RESIZE_FILE_ON_BACKSPACE
#define CHX_RESIZE_FILE_ON_INSERTION

/* IMPLEMENT / INCLUDE YOUR OWN FUNCTIONS HERE */
// function layout (interpreter command with params): void <my_func>(char <num_params>, char** <param_list>)
// function layout (keybind / void interpreter command): void <my_func>(void)



/* GLOBAL KEYBINDS (WORK IN ANY MODE) */
void (*chx_keybinds_global[])(void) = {
	[KEY_MAX_VAL] = fvoid, // do not remove
	[KEY_ESCAPE] = chx_mode_set_default,
	[KEY_UP] = chx_cursor_move_up,
	[KEY_DOWN] = chx_cursor_move_down,
	[KEY_RIGHT] = chx_cursor_next_byte,
	[KEY_LEFT] = chx_cursor_prev_byte,
	[CHX_SHIFT(KEY_UP)] = chx_cursor_select_up,
	[CHX_SHIFT(KEY_DOWN)] = chx_cursor_select_down,
	[CHX_SHIFT(KEY_RIGHT)] = chx_cursor_select_right,
	[CHX_SHIFT(KEY_LEFT)] = chx_cursor_select_left,
	[CHX_ALT(KEY_UP)] = chx_cursor_move_up_by_5,
	[CHX_ALT(KEY_DOWN)] = chx_cursor_move_down_by_5,
	[CHX_ALT(KEY_RIGHT)] = chx_cursor_move_right_by_5,
	[CHX_ALT(KEY_LEFT)] = chx_cursor_move_left_by_5,
	[CHX_CTRL_M(KEY_UP)] = chx_cursor_move_up,
	[CHX_CTRL_M(KEY_DOWN)] = chx_cursor_move_down,
	[CHX_CTRL_M(KEY_RIGHT)] = chx_cursor_move_right,
	[CHX_CTRL_M(KEY_LEFT)] = chx_cursor_move_left,
	[CHX_CTRL('y')] = chx_copy,
	[CHX_CTRL('p')] = chx_paste_after,
	[CHX_CTRL('P')] = chx_paste_before,
	[CHX_CTRL('t')] = chx_type_mode_toggle,
	[CHX_CTRL('i')] = chx_insert_mode_toggle,
	[CHX_CTRL('r')] = chx_replace_mode_toggle,
	[CHX_CTRL('e')] = chx_swap_endianness,
	[CHX_CTRL('w')] = chx_save,
	[CHX_CTRL('e')] = chx_prompt_save_as,
	[CHX_CTRL('u')] = chx_revert,
	[CHX_CTRL('x')] = chx_exit,
	[KEY_DELETE] = chx_delete_hexchar,
	[KEY_PG_UP] = chx_page_up,
	[KEY_PG_DN] = chx_page_down,
	[CHX_CTRL_M(KEY_PG_UP)] = chx_next_inst,
	[CHX_CTRL_M(KEY_PG_DN)] = chx_prev_inst,
};

/* COMMAND MODE KEYBINDS */
void (*chx_keybinds_mode_command[])(void) = {
	[KEY_MAX_VAL] = fvoid, // do not remove
	['k'] = chx_cursor_move_up,
	['j'] = chx_cursor_move_down,
	['l'] = chx_cursor_next_byte,
	['h'] = chx_cursor_prev_byte,
	['K'] = chx_cursor_select_up,
	['J'] = chx_cursor_select_down,
	['L'] = chx_cursor_select_right,
	['H'] = chx_cursor_select_left,
	[CHX_ALT('k')] = chx_cursor_move_up_by_5,
	[CHX_ALT('j')] = chx_cursor_move_down_by_5,
	[CHX_ALT('l')] = chx_cursor_move_right_by_5,
	[CHX_ALT('h')] = chx_cursor_move_left_by_5,
	[CHX_CTRL('k')] = chx_cursor_move_up,
	[CHX_CTRL('j')] = chx_cursor_move_down,
	[CHX_CTRL('l')] = chx_cursor_move_right,
	[CHX_CTRL('h')] = chx_cursor_move_left,
	['u'] = chx_revert,
	['y'] = chx_copy,
	['P'] = chx_paste_before,
	['p'] = chx_paste_after,
	['V'] = chx_erase_hexchar,
	['v'] = chx_remove_hexchar,
	['X'] = chx_erase_ascii,
	['x'] = chx_remove_ascii,
	['G'] = chx_to_end,
	['g'] = chx_to_start,
	['D'] = chx_remove_selected,
	['d'] = chx_delete_selected,
	['T'] = chx_mode_set_type_ascii,
	['t'] = chx_mode_set_type,
	['I'] = chx_mode_set_insert_ascii,
	['i'] = chx_mode_set_insert,
	['R'] = chx_mode_set_replace_ascii,
	['r'] = chx_mode_set_replace,
	['q'] = chx_quit,
	['.'] = chx_execute_last_action,
	['^'] = chx_to_line_start,
	['$'] = chx_to_line_end,
	[':'] = chx_prompt_command,
};

/* VOID INTERPRETER COMMANDS */
struct chx_void_command chx_void_commands[] = {
	(struct chx_void_command) {chx_print_finfo, "get-info"},
	(struct chx_void_command) {chx_toggle_inspector, "ti"},
	(struct chx_void_command) {chx_toggle_preview, "tp"},
	(struct chx_void_command) {chx_swap_endianness, "se"},
	(struct chx_void_command) {chx_prompt_save_as, "saveas"},
	(struct chx_void_command) {chx_exit, "exit"},
	(struct chx_void_command) {chx_exit, "q!"},
	(struct chx_void_command) {chx_quit, "quit"},
	(struct chx_void_command) {chx_quit, "q"},
	(struct chx_void_command) {chx_save_and_quit, "wq"},
	(struct chx_void_command) {chx_to_start, "0"},
	(struct chx_void_command) {0, 0} // do not remove
};

/* INTERPRETER COMMANDS (WITH PARAMS) */
struct chx_command chx_commands[] = {
	(struct chx_command) {chx_set_inst, "to"},
	(struct chx_command) {chx_set_endianness_global, "ge"},
	(struct chx_command) {chx_save_as, "save"},
	(struct chx_command) {chx_save_as, "sav"},
	(struct chx_command) {chx_save_as, "w"},
	(struct chx_command) {chx_config_layout_global, "gcfg"},
	(struct chx_command) {chx_config_layout, "cfg"},
	(struct chx_command) {chx_count_instances, "count"},
	(struct chx_command) {chx_count_instances, "cnt"},
	(struct chx_command) {chx_find_next, "find"},
	(struct chx_command) {chx_find_next, "f"},
	(struct chx_command) {chx_find_next, "/"},
	(struct chx_command) {chx_switch_file, "switch"},
	(struct chx_command) {chx_switch_file, "sw"},
	(struct chx_command) {chx_switch_file, "s"},
	(struct chx_command) {chx_open_instance, "open"},
	(struct chx_command) {chx_open_instance, "o"},
	(struct chx_command) {chx_close_instance, "close"},
	(struct chx_command) {chx_close_instance, "c"},
	(struct chx_command) {0, 0} // do not remove
};

/* FUNCTIONS TO EXCLUDE FROM ACTION HISTORY */
void* func_exceptions[] = {
	chx_cursor_move_up,
	chx_cursor_move_down,
	chx_cursor_move_right,
	chx_cursor_move_left,
	chx_cursor_select_up,
	chx_cursor_select_down,
	chx_cursor_select_right,
	chx_cursor_select_left,
	chx_cursor_next_byte,
	chx_cursor_prev_byte,
	chx_execute_last_action,
	chx_prompt_command,
	chx_mode_set_type_ascii,
	chx_mode_set_type,
	chx_mode_set_insert_ascii,
	chx_mode_set_insert,
	chx_mode_set_replace_ascii,
	chx_mode_set_replace,
	chx_remove_selected,
	chx_delete_selected,
	chx_erase_hexchar,
	chx_remove_hexchar,
	chx_erase_ascii,
	chx_remove_ascii,
	chx_open_instance,
	chx_close_instance,
	chx_set_inst,
	chx_switch_file,
};


