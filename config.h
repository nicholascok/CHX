#define KEY_UP 		0x0241
#define KEY_DOWN 	0x0242
#define KEY_RIGHT 	0x0243
#define KEY_LEFT 	0x0244

#define KEY_S_UP 	0x0341
#define KEY_S_DOWN 	0x0342
#define KEY_S_RIGHT 0x0343
#define KEY_S_LEFT 	0x0344

#define KEY_ESCAPE 	0x0100
#define KEY_MAX_VAL 0x03FF

#define CHX_CTRL(C) (C & 0x1F)
#define CHX_ALT(C) ((C & 0x00FF) | 0x0100)

void (*chx_keybinds_global[])(void) = {
	[KEY_MAX_VAL] = fvoid, // do not remove
	[KEY_ESCAPE] = chx_mode_set_default,
	[KEY_UP] = chx_cursor_move_up,
	[KEY_DOWN] = chx_cursor_move_down,
	[KEY_RIGHT] = chx_cursor_move_right,
	[KEY_LEFT] = chx_cursor_move_left,
	[KEY_S_UP] = chx_cursor_select_up,
	[KEY_S_DOWN] = chx_cursor_select_down,
	[KEY_S_RIGHT] = chx_cursor_select_right,
	[KEY_S_LEFT] = chx_cursor_select_left,
	[CHX_CTRL('i')] = chx_type_mode_toggle,
	[CHX_CTRL('w')] = chx_save,
	[CHX_CTRL('x')] = chx_quit,
	[CHX_CTRL('e')] = chx_save_as
};

void (*chx_keybinds_mode_command[])(void) = {
	[KEY_MAX_VAL] = fvoid, // do not remove
	['i'] = chx_mode_set_insert,
	['q'] = chx_quit,
	['s'] = chx_save,
	['e'] = chx_save_as
};