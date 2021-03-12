#ifndef __CHX_CAOIMH_CONFIG__
#define __CHX_CAOIMH_CONFIG__

void (*chx_keybinds_global_control[])(void) = {
	['i'] = chx_type_mode_toggle,
	['w'] = chx_save,
	['x'] = chx_quit,
	['e'] = chx_save_as
};

void (*chx_keybinds_global_escape[])(void) = {
	[KEY_UP] = chx_cursor_move_up,
	[KEY_DOWN] = chx_cursor_move_down,
	[KEY_RIGHT] = chx_cursor_move_right,
	[KEY_LEFT] = chx_cursor_move_left
};

void (*chx_keybinds_mode_command[])(void) = {
	['i'] = chx_mode_set_insert,
	['q'] = chx_quit,
	['s'] = chx_save,
	['e'] = chx_save_as
};

#endif