#include "chx.h"

void chx_type_mode_toggle() {
	if (CHXGC.mode == CHX_MODE_TYPE_HEXCHAR) CHXGC.mode = CHX_MODE_DEFAULT;
	else CHXGC.mode = CHX_MODE_TYPE_HEXCHAR;
}

void chx_cursor_move_up() {
	if (CHXCUR.y > 0) CHXCUR.y--;
	if (CHXCUR.y < CHXGC.section_start) CHXGC.section_start--, chx_draw_contents();
	chx_update_cursor();
}

void chx_cursor_move_down() {
	if (CHXCUR.y < CHXGC.num_rows) CHXCUR.y++;
	if (CHXCUR.y > CHXGC.section_start + CHXGC.rows_in_section - 1) CHXGC.section_start++, chx_draw_contents();
	chx_update_cursor();
}

void chx_cursor_move_right() {
	if (CHXCUR.sbpos < CHXGC.bytes_in_group * 2 - 1) CHXCUR.sbpos++;
	else if (CHXCUR.x < CHXGC.bytes_per_row / CHXGC.bytes_in_group - 1) CHXCUR.sbpos = 0, CHXCUR.x++;
	else CHXCUR.sbpos = 0, CHXCUR.x = 0, CHXCUR.y++;
	chx_update_cursor();
}

void chx_cursor_move_left() {
	if (CHXCUR.sbpos > 0) CHXCUR.sbpos--;
	else if (CHXCUR.x > 0) CHXCUR.sbpos = CHXGC.bytes_in_group * 2 - 1, CHXCUR.x--;
	else if (CHXCUR.y > 0) CHXCUR.sbpos = CHXGC.bytes_in_group * 2 - 1, CHXCUR.x = CHXGC.bytes_per_row / CHXGC.bytes_in_group - 1, CHXCUR.y--;
	chx_update_cursor();
}

void chx_mode_set_insert() {
	CHXGC.mode = CHX_MODE_TYPE_HEXCHAR;
}

void chx_save() {
	chx_export(CHXGC.instances[CHXGC.sel_instance].filename);
	
	// redraw content to remove unsaved highlights
	chx_draw_contents();
}

void chx_save_as() {
	// setup user input buffer
	char usrin[256];
	
	// print save dialoge and recieve user input
	cur_set(0, CHXGC.theight);
	printf("SAVE AS? (LEAVE EMPTY TO CANCEL): ");
	fflush(stdout);
	
	fgets(usrin, 256, stdin);
	
	// cut input at first newline
	usrin[strcspn(usrin, "\n")] = 0;
	
	// only export if filename was entered
	if (usrin[0]) chx_export(usrin);
	
	// erase save dialoge
	printf("\033[1A\033[2K");
	fflush(stdout);
	
	// redraw contents
	chx_draw_contents();
}

void chx_quit() {
	// ask user if they would like to save
	cur_set(0, CHXGC.theight);
	printf("WOULD YOU LIKE TO SAVE? (Y / N): ");
	
	switch (fgetc(stdin)) {
		case 'y':
		case 'Y':
			chx_export(CHXGC.instances[CHXGC.sel_instance].filename);
			break;
		default:
			cls();
			chx_main();
			break;
		case 'n':
		case 'N':
			break;
	}
	
	// restore terminal state
	texit();
	exit(0);
}