#include "chx.h"

void chx_type_mode_toggle() {
	if (CHXGC.mode == CHX_MODE_TYPE_HEXCHAR) CHXGC.mode = CHX_MODE_DEFAULT;
	else CHXGC.mode = CHX_MODE_TYPE_HEXCHAR;
}

void chx_cursor_move_up() {
	CHXCUR.y -= (CHXCUR.y > 0);
	CHXGC.section_start -= (CHXCUR.y < CHXGC.section_start);
	chx_draw_contents();
}

void chx_cursor_move_down() {
	CHXCUR.y++;
	CHXGC.section_start += (CHXCUR.y - CHXGC.section_start >= CHXGC.rows_in_section);
	chx_draw_contents();
}

void chx_cursor_move_right() {
	if (CHXCUR.sbpos < CHXGC.bytes_in_group * 2 - 1) CHXCUR.sbpos++;
	else if (CHXCUR.x < CHXGC.bytes_per_row / CHXGC.bytes_in_group - 1) CHXCUR.sbpos = 0, CHXCUR.x++;
	else CHXCUR.sbpos = 0, CHXCUR.x = 0, CHXCUR.y++;
	chx_update_cursor();
}

void chx_cursor_move_left() {
	if (CHXCUR.sbpos > 0) CHXCUR.sbpos--;
	else {
		CHXCUR.sbpos = CHXGC.bytes_in_group * 2 - 1;
		if (CHXCUR.x > 0) CHXCUR.x--;
		else if (CHXCUR.y > 0) CHXCUR.x = CHXGC.bytes_per_row / CHXGC.bytes_in_group - 1, CHXCUR.y--;
		else CHXCUR.sbpos = 0;
	}
	chx_update_cursor();
}

void chx_update_cursor() {
	cur_set(CHXGC.num_digits + 2 + CHXCUR.x * (2 + CHXGC.bytes_in_group * 2) + CHXCUR.sbpos, CHXCUR.y - CHXGC.section_start + TPD);
	fflush(stdout);
}

void chx_type_hexchar(char _c) {
	if (!IS_CHAR_HEX(_c)) return; // only accept hex characters
	if ((_c ^ 0x60) < 7) _c -= 32; // ensure everything is upper-case
	printf("\033[1;35m%c\033[0m\033[1D", _c); // print the character on the screen
	
	char nullkey[2] = {_c, 0};
	
	// update stored file data
	CHXGC.instances[CHXGC.sel_instance].data[CHXCUR.y * CHXGC.bytes_per_row + (CHXCUR.x * CHXGC.bytes_in_group) + CHXCUR.sbpos / 2] &= 0x0F << ((CHXCUR.sbpos % 2) * 4);
	CHXGC.instances[CHXGC.sel_instance].data[CHXCUR.y * CHXGC.bytes_per_row + (CHXCUR.x * CHXGC.bytes_in_group) + CHXCUR.sbpos / 2] |= strtol(nullkey, NULL, 16) << (((CHXCUR.sbpos + 1) % 2) * 4);
	cur_set(0, 0);
	
	// move cursor after typing a char
	// if the cursor is in a sub position, add to the sub position of the cursor, else change the selected byte
	chx_cursor_move_right();
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