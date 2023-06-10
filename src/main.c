#include <gint/display.h>
#include <gint/keyboard.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define SCL_EVALUATION_FUNCTION SCL_boardEvaluateStatic


#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#include "smallchesslib.h"
#pragma GCC diagnostic pop


const char* menu_opts[2] = {"2Player", "VsAI"};
const short menu_opts_size = 2;

SCL_Game setupchess() {
	SCL_Game game;
	SCL_gameInit(&game, NULL);
	return game;
}

#define pieceImage(idx) dsubimage(x, y, &pieces_map, idx*piece_width, white*piece_height, piece_width, piece_height, DIMAGE_NONE)
#define pieceCase(piece,num) case piece: pieceImage(num);break;

void renderpiece(char piece, int x, int y) {
	extern bopti_image_t pieces_map;

	const int piece_width = 28;
	const int piece_height = 28;
	int white = SCL_pieceIsWhite(piece);
	switch ((char)tolower(piece)) {
		pieceCase('b',0)
		pieceCase('k',1)
		pieceCase('n',2)
		pieceCase('p',3)
		pieceCase('q',4)
		pieceCase('r',5)
		default:
			break;
	}
}

#undef pieceImage
#undef pieceCase

#define MIN(a, b) a>b?b:a
#define MAX(a, b) a>b?a:b

void renderboard(SCL_Game* game, char sel_rank, char sel_file, int attempting_move, const SCL_SquareSet* highlight) {
	dclear(C_WHITE);
	const char size=28;
	const int invImgY = DHEIGHT - size;
	for (char rank=0; rank < 8; rank++) {
		for (char file=0; file < 8; file++) {
			int square = SCL_SQUARE_INTS(file, rank);
			color_t col;
			if ((sel_rank == rank && sel_file == file) || square == attempting_move)
				col = C_RGB(31, 31, 0);
			else if (highlight && SCL_squareSetContains(*highlight, square))
				col = C_BLUE;
			else
				if ((rank + file) % 2 != 0)
					col = C_RED;
				else
					col = C_WHITE;
			
			
			drect(size*file, DHEIGHT-size*rank, size*file + size, DHEIGHT-(size*rank + size), col);
			
			
			renderpiece(game->board[square], size*file, invImgY - size*rank);
			char moveString[4];
			moveString[0] = 0;
			moveString[2] = game->board[square];
			SCL_squareToString(square, moveString);
			//dtext(size*file, invImgY - size*rank, C_BLACK, moveString);
		}
	}
	// sidebar
	const int sidebar_start = size*8 + 10;
	bool printSide = true;
	char turnString[21] = "Current turn: ";
	if (SCL_boardGameOver(game->board)) {
		drect(0,0,size*8,size*8,C_INVERT);
		if (game->state & 0xf0) {
			printSide = false;
			strcpy(turnString, "Draw");
		} 
		else {
			switch (game->state) {
				case SCL_GAME_STATE_BLACK_WIN:
				case SCL_GAME_STATE_WHITE_WIN:
					strcpy(turnString, "Mate! Loser: ");
					break;
				default:
					strcpy(turnString, "what the scallop");
					break;
			}
		}
	}
	if (printSide)
		strcat(turnString, SCL_boardWhitesTurn(game->board) ? "white" : "black");


	dtext(sidebar_start, 10, C_BLACK, turnString);
	dline(size*8, 35, DWIDTH, 35, C_BLACK);
	const int move_list_pos = 40;
	const int move_list_max_size = (DHEIGHT-move_list_pos)/15;
	
	const int width = (DWIDTH-sidebar_start);
	const int centre = DWIDTH-(width/2);
	const int left_move_x = centre - width/4;
	const int right_move_x = centre + width/4;

	int plysize = SCL_recordLength(game->record);
	if (plysize > 0) {
		// convert ply into move list
		int ms = plysize/2;
		if (plysize % 2 != 0)
			ms++;
		char moves[ms][2][6];
		memset(moves, 0, sizeof moves);
		int mscount = 0;
		for (int i=0; i<plysize; i++) {
			uint8_t lsquareFrom;
			uint8_t lsquareTo;
			char lpromoted;
			SCL_recordGetMove(game->record, i, &lsquareFrom, &lsquareTo, &lpromoted);
			SCL_moveToString(game->board, lsquareFrom, lsquareTo, lpromoted, moves[mscount][i%2]);
			if (i % 2 != 0)
				mscount++;
		}
		int i = MAX(ms-move_list_max_size, 0);
		int imax = MIN(ms, i+move_list_max_size);
		
		int count = 0;
		char status[20] = "";
		if (SCL_boardCheck(game->board, SCL_boardWhitesTurn(game->board))) {
			strcat(status, "Check! ");
		}
		dprint(sidebar_start, 25, C_BLACK, status);
		for (; i<imax; i++) {
			count++;
			dprint_opt(sidebar_start+5, move_list_pos+(count*15)-7, C_BLACK, C_WHITE, DTEXT_LEFT, DTEXT_MIDDLE, "%d.", i);
			dtext_opt(left_move_x, (move_list_pos+count*15)-7, C_BLACK, C_WHITE, DTEXT_CENTER, DTEXT_MIDDLE, moves[i][0], -1);
			dtext_opt(right_move_x, (move_list_pos+count*15)-7, C_BLACK, C_WHITE, DTEXT_CENTER, DTEXT_MIDDLE, moves[i][1], -1);
		}
	}
	
	dupdate();
}



void turn(SCL_Game* game) {
	static short sel_rank = 0;
	static short sel_file = 0;
	int attempt_move = -1;
	SCL_SquareSet target_squares;
	SCL_squareSetClear(target_squares);
	while (true) {
		renderboard(game, sel_rank, sel_file, attempt_move, &target_squares);

		key_event_t key = getkey();
		switch (key.key) {
			case KEY_DOWN:
				sel_rank--;
				break;
			case KEY_UP:
				sel_rank++;
				break;
			case KEY_LEFT:
				sel_file--;
				break;
			case KEY_RIGHT:
				sel_file++;
				break;
			case KEY_EXIT:
				if (attempt_move != -1) {
					attempt_move = -1;
					SCL_squareSetClear(target_squares);
				} else {
					dclear(C_WHITE);
					dtext(0, 0, C_BLACK, "Press exit twice to quit game,");
					dtext(0, 10, C_BLACK, "anything else to resume");
					dupdate();
					key_event_t key = getkey();
					if (key.key == KEY_EXIT) {
						game->state = SCL_GAME_STATE_END;
						return;
					} else {
						renderboard(game, sel_rank, sel_file, attempt_move, &target_squares);
						break;
					}

				}
				break;
			case KEY_EXE:
				char sel = SCL_SQUARE_INTS(sel_file, sel_rank);
				if (attempt_move == -1) { // if no piece being moved
					if (game->board[(int)sel] == '.')
						break;
					if (SCL_pieceIsWhite(game->board[(int)sel]) != SCL_boardWhitesTurn(game->board)) { // ensure correct turn piece
						dupdate();
						break;
					}
					SCL_boardGetMoves(game->board, sel, target_squares);
					if (SCL_squareSetSize(target_squares) == 0) {
						dupdate();
						break;
					}
					attempt_move = sel;
				} else {
					if (SCL_squareSetContains(target_squares, sel)) {
						char promotion = 'q';
						if (tolower(game->board[(int)attempt_move]) == 'p' && (sel_rank == 0 || sel_rank == 7)) {
							dclear(C_WHITE);
							dtext(0, 0, C_BLACK, "Left-Q Right-R Down-N Up-B");
							dupdate();
							while (true) {
								key_event_t key = getkey();
								if (key.key == KEY_LEFT) {
									promotion = 'q';
									break;
								} else if (key.key == KEY_RIGHT) {
									promotion = 'r';
									break;
								} else if (key.key == KEY_DOWN) {
									promotion = 'n';
									break;
								} else if (key.key == KEY_UP) {
									promotion = 'b';
									break;
								}
							}
						}
						SCL_gameMakeMove(game, attempt_move, sel, promotion);
						renderboard(game, sel_rank, sel_file, attempt_move, &target_squares);
						return;
					} else {
						attempt_move = -1;
						SCL_squareSetClear(target_squares);
					}
				}
				break;
			default:
				break;
		}
		sel_rank = MIN(sel_rank, 7);
		sel_rank = MAX(sel_rank, 0);
		sel_file = MIN(sel_file, 7);
		sel_file = MAX(sel_file, 0);
	}
}

uint8_t randfunc() {
	return (char)rand();
}

void vsai() {

	// choose white or black
	dclear(C_WHITE);
	dtext(0, 0, C_BLACK, "Press left to be white, press right to be black.");
	dupdate();
	int side = -1;
	while (true) {
		key_event_t key = getkey();
		if (key.key == KEY_LEFT) {
			side = 1;
			break;
		} else if (key.key == KEY_RIGHT) {
			side = 0;
			break;
		} else if (key.key == KEY_EXIT) {
			return;
		}
	}


	SCL_Game game = setupchess();
	renderboard(&game, -1, -1, -1, NULL);

	while (game.state == SCL_GAME_STATE_PLAYING) {
		if (SCL_boardWhitesTurn(game.board) == side)
			turn(&game);
		else {
			// ai turn
			uint8_t from;
			uint8_t to;
			uint8_t rfrom;
			uint8_t rto;
			char promotion = 'q';
			SCL_gameGetRepetiotionMove(&game, &rfrom, &rto);
			SCL_getAIMove(game.board, 2, 1, 1, SCL_boardEvaluateStatic, randfunc, 2, rfrom, rto, &from, &to, &promotion);
			char mstr[20];
			SCL_moveToString(game.board, from, to, promotion, mstr);
			SCL_gameMakeMove(&game, from, to, promotion);
		}
	}
		
	renderboard(&game, -1, -1, -1, NULL);
	getkey();
}

void vslocal() {
	SCL_Game game = setupchess();
	renderboard(&game, -1, -1, -1, NULL);
	while (game.state == SCL_GAME_STATE_PLAYING)
		turn(&game);
	getkey();
}


int main(void)
{
	randfunc();
	short selected = 0;
	// Main menu loop
	while (true) {
		dclear(C_WHITE);
		dtext_opt(DWIDTH/2, 10, C_BLACK, C_WHITE, DTEXT_CENTER, DTEXT_TOP, "FxChess", -1);
		for (short i=0; i<menu_opts_size; i++) {
			dtext_opt(DWIDTH/2, 40 + (i*10), C_BLACK, C_WHITE, DTEXT_CENTER, DTEXT_TOP, menu_opts[i], -1);
			if (selected == i) {
				drect(0, 40 + (i*10), DWIDTH, 40 + (i*10) + 10, C_INVERT);
			}
		}

		dupdate();

		key_event_t key = getkey();
		switch (key.key)
		{
		case KEY_DOWN:
			selected++;
			break;
		case KEY_UP:
			selected--;
			break;
		case KEY_EXE:
			if (selected)
				vsai();
			else
				vslocal();
			break;
		default:
			break;
		}
		if (selected > 1)
			selected = 0;
		if (selected < 0)
			selected = 1;
	}
	return 1;
}
