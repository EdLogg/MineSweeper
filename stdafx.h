// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN						// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <shlobj.h>									// to use SHGetKnownFolderPath
#include <commdlg.h>								// required for load and save files
#include <stdio.h>									// for fopen and sprintf
#include <time.h>									// for time functions
#include <math.h>
#include "resource.h"
#include "rand.h"


// build parameters
#define SPEED				10						// number of milliseconds between puzzles for solving 
#define	MAX_SEEDS			10						// number of seeds to use in testing
#define TEST_SAMPLES		500						// how many puzzles to test


// puzzle characteristics
#define MAX_WIDTH			30
#define MAX_HEIGHT			16
#define MAX_MINES			99
#define	SCREEN_WIDTH		(MAX_WIDTH * TILE_WIDTH)
#define	SCREEN_HEIGHT		(MAX_HEIGHT * TILE_HEIGHT)
#define	START_X				3						// default start puzzle here
#define START_Y				3
#define END_PLAY_COUNT		30						// when to start end play
#define MAX_COMBINE_LISTS	50						// combine lists unless we have this many or more entries
#define MAX_SEQUENCE		60						// warn us when we get sequences this long
#define MIN_GUESS1_COUNT	8						// min count to use guess1 to support guess2
#define GUESS1_OFFSET		-.02					// default value for this offset
#define EXPOSE_MINE			9						// code in exposed[] for mine
#define EXPOSE_EXPODE		10						// code in exposed[] for expoding mine
#define	MAX_LISTS			40						// number of lists for the tank method


enum RULES
{
	RULE_1 = 0,
	RULE_2,
	RULE_3,
	RULE_4,
	RULE_5,
	RULE_6,
	RULE_7,
	RULE_8,
	RULE_9,
	RULE_10,
	GUESS_1,
	GUESS_2,
	MAX_RULES											// number of rules to use to solve puzzle
};							


// file.cpp definitions
#define MAX_WINS_WITH_GUESSES	9
extern int		winsWithGuesses[MAX_WINS_WITH_GUESSES];
extern double	winProb;								// sum of prob for all wins
extern double	lossProb;
extern double	guess1Prob;								// sum of all guess1 prob 
extern double	guess2Prob;								// sum of all guess2 prob 
extern int		guessesPerWin;							// number of guesses per win
extern int		guessesPerLoss;
extern int		winTotal;								// total number of wins
extern int		winAllTotal;							// total number of wins over all seeds
extern int		guess1Failed;
extern int		guess2Failed;
extern int		totalWins;								// number of wins and tests
extern int		totalTests;
extern int		totalGuess1;							// total guess1 and guess2
extern int		totalGuess2;
extern int		totalGuess1Failed;						// number of failed guesses
extern int		totalGuess2Failed;
extern int		totalGuessesPerWin;						// number of guesses per win
extern int		totalGuessesPerLoss;
extern int		totalWinsWithGuesses[MAX_WINS_WITH_GUESSES];
extern double	totalWinProb;							// sum of prob for all wins
extern double	totalLossProb;
extern double	totalGuess1Prob;						// sum of all guess1 prob 
extern double	totalGuess2Prob;						// sum of all guess2 prob
void ClearTotalStats();									// clear total stats
void SaveTotalResults(FILE * file);						// save total results
void SaveResults(FILE * file);							// save results of the puzzle including errors
void SaveAllResults(FILE * file, U32 seed, int index, int count, bool done, int total);		// save results of the puzzle including errors
void ShowTotalResults(FILE * file, int tests, int seed);	// display stats on remaining count and mines (min, max, avg)
bool SavePuzzle(char * fname);
bool LoadPuzzle(char * fname);
bool SaveFileName(HWND hwnd, char * fname, char * filterString, char * defaultExt);
bool LoadFileName(HWND hwnd, char * fname, char * filterString, char * defaultExt);
bool SaveGameFile();
bool LoadGameFile();

// game.c definitions
extern int	puzzle[MAX_HEIGHT][MAX_WIDTH];			// 1 for mine and 0 otherwise
extern int	exposed[MAX_HEIGHT][MAX_WIDTH];			// -1 if not exposed, 0-8 for numbers, 9 for mine
extern int	lastExposed[MAX_HEIGHT][MAX_WIDTH];		// used for backup
extern int	last[MAX_HEIGHT][MAX_WIDTH];			// used for backing up last puzzle
extern bool	started;								// puzzle started
extern bool	solving;								// puzzle solving in progress
extern bool	gameOver;								// true when game is over
extern bool	backup;									// if backup available
extern bool	cheating;								// show mines when cheating
extern bool	rules[MAX_RULES];						// what rules to follow to solve puzzle
extern int	rulesCount[MAX_RULES];					// count of rules used
extern int	rulesCountTotal[MAX_RULES];				// total count of rules used for all puzzles tested
extern int	guesses;								// number of guesses so far this puzzle
extern int	lastRuleUsed;							// last rule used
extern int	mineCount;								// mines left to find
extern int	remainingCount;							// number left to expose
extern int	lastMineCount;							// mines left to find on backup
extern int	lastRemainingCount;						// number left to expose on backup
extern double guess1Offset;							// amount to offset the guess1 probability (negative is more guess1)
extern int	endPlayCount;							// default value for end play 
extern int	endGameCount;							// number before use just one tank list
extern double	prob;								// current probability of getting this right
extern double	lastProb;							// last value of prob
extern int	unexposed[MAX_HEIGHT][MAX_WIDTH];
extern int	mines[MAX_HEIGHT][MAX_WIDTH];
extern int	needed[MAX_HEIGHT][MAX_WIDTH];
extern int	neighbors[MAX_HEIGHT][MAX_WIDTH];
void	TimeInit();
void	TimeStartOverall();
double	TimeEndOverall();
void SaveBackUp();									// save move for backup
bool BackUp();										// back up to the last move
void BackUpChanged();								// test to insure backup changed
int CountMines(int x, int y);						// count mines around (x,y)
void AddMine(int x, int y);							// add mine at (x,y)
void Expose(int x, int y);							// expose area at (x,y)
void NewGame(int x, int y);							// start new game at (x,y)
bool MakeMove();									// try to find one move to make

// tank.cpp definitions
void CreateTankLists();								// create lists for tank method
void CalculateProb();								// calculate proability for each entry
bool CreateTankSolutions();							// find all possible solutions on the lists	
bool FindEasyTankMove();							// find obvious tank move
bool PickOpenSpot(bool always);						// pick an open spot
bool PickBestSpot();								// pick best spot via tank rules

// MineSweeper.cpp definitions
extern	int		width, height, minesStart;			// size of puzzle and number of mines to start
extern	char	testFilePath[MAX_PATH];				// path if testing
extern	int		testing;							// puzzle testing remaining
extern	int		testStart;							// what starting position (index) are we testing
extern	int		testEndPlay;						// what end play (index) are we testing
extern	int		testOffset;							// what offset (index)are we testing
extern	int		startX, startY;						// starting location for new games
extern	int		wins[1000];							// save win count for IDM_TEST200x100 and IDM_TEST1000x100
extern	FILE *	testResults;						// file for test results
extern	FILE *	testAllResults;						// file for test results
