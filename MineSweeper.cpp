// MineSweeper.cpp : Defines the entry point for the application.
//

#include "stdafx.h"


#define MAX_LOADSTRING 100						// max size of app title and class name

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR	szTitle[MAX_LOADSTRING];                // The title bar text
TCHAR	szWindowClass[MAX_LOADSTRING];          // the main window class name
HBITMAP hBackgndExpert = NULL;
HBITMAP hBackgndMedium = NULL;
HBITMAP hBackgndSmall = NULL;
HBITMAP hMine = NULL;
HBITMAP hNumbers = NULL;
FILE *	testResults = NULL;						// for testing stats
FILE *	testAllResults = NULL;					// for testing starting, endplay, or offsets possibilities
char	startString[16];
char	seedString[16];
char	endPlayString[16];
char	offsetString[16];
char 	prefixString[128];
int		width, height, minesStart;				// size of puzzle and number of mines to start
int		testing;								// puzzle testing remaining
int		testingMax;								// max number of puzzles to test
int		testingAllTotal;						// total number of puzzles when testing multiple seeds
int		testStart;								// what starting position (index) are we testing
int		testEndPlay;							// what end play (index) are we testing
int		testOffset;								// what offset (index)are we testing
char	testFilePath[MAX_PATH];
char	testFileName[MAX_PATH];
char	testAllFileName[MAX_PATH];
int		startX, startY;							// starting location for new games
int		saveStartX, saveStartY;
int		saveEndPlayCount;
double	saveGuess1Offset;
int		startPositions;							// number of positions for start pos tests
int		seedsUsed;								// number of seeds used so far
U32		seed;									// seed for random number generator
U32		saveSeed;								// save seed used for testing
U32		startSeed;								// starting seed to use for starting multiple tests
U32		lastSeed;								// last seed to use
int		wins[1000];								// save win count for IDM_TEST200x100 and IDM_TEST1000x100
bool	savePuzzles;							// save puzzles when testing 


static	RECT Interior = { WIDTH_OFFSET, 0, WIDTH_OFFSET + SCREEN_WIDTH, 343 }; // include time and count area below
static	RECT TimeRect = { 114, 329, 145, 343 };
static	RECT RuleRect = { 200, 0, 400, 14 };
static	RECT GameOverRect = { 200, 329, 400, 343 };
static	RECT CountRect = { 456, 329, 487, 343 };
static	RECT ProbRect = { 100, 329, 160, 343 };

static char * RuleText[MAX_RULES] =
{
	"Rule 1",
	"Rule 2",
	"Rule 3",
	"Rule 4",
	"Rule 5",
	"Rule 6",
	"Rule 7",
	"Rule 8",
	"Rule 9",
	"Rule 10",
	"Guess 1",
	"Guess 2",
};


int EndPlayValues[] =
{
	8, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
};
#define END_PLAY_VALUES	(sizeof(EndPlayValues) / sizeof(EndPlayValues[0]))

double Guess1Offsets[] =
{
	-.08, -.07, -.06, -.05, -.04, -.03, -.02, -.01, 0.0, .01, .02, .03, .04, .05,
};
#define GUESS1_OFFSETS	(sizeof(Guess1Offsets) / sizeof(Guess1Offsets[0]))

int StartSmallPositions[][2] =
{
	{ 0, 0 },
	{ 1, 1 },
	{ 2, 2 },
	{ 2, 1 },
//	{ 1, 2 },
	{ 3, 3 },
	{ 3, 2 },
//	{ 2, 3 },
	{ 4, 4 },
	{ 4, 3 },
//	{ 3, 4 },
	{ 4, 2 },
//	{ 2, 4 },
	{ 4, 1 },
//	{ 1, 4 },
};
#define SMALL_POSITIONS	(sizeof(StartSmallPositions) / sizeof(StartSmallPositions[0]))

int StartMediumPositions[][2] =
{
	{ 0, 0 },
	{ 1, 1 },
	{ 2, 2 },
	{ 2, 1 },
//	{ 1, 2 },
	{ 3, 3 },
	{ 3, 2 },
//	{ 2, 3 },
	{ 3, 1 },
	{ 4, 4 },
	{ 4, 3 },
//	{ 3, 4 },
	{ 4, 2 },
//	{ 2, 4 },
	{ 4, 1 },
	{ 7, 7 },
	{ 7, 6 },
	{ 7, 5 },
	{ 7, 4 },
	{ 7, 3 },
	{ 7, 2 },
	{ 7, 1 },
};
#define MEDIUM_POSITIONS	(sizeof(StartMediumPositions) / sizeof(StartMediumPositions[0]))

#define R	MAX_WIDTH - 1
#define B	MAX_HEIGHT - 1
int StartExpertPositions[][2] =
{
	{ 0, 0 },
	{ 1, 1 },
//	{ 1, B - 1 },
//	{ R - 1, 1 },
//	{ R - 1, B - 1 },
	{ 2, 2 },
//	{ 2, B - 2 },
//	{ R - 2, B - 2 },
//	{ R - 2, 2 },
	{ 2, 1 },
//	{ 2, B - 1 },
//	{ R - 2, B - 1 },
//	{ R - 2, 1 },
//	{ 1, 2 },
//	{ 1, B - 2 },
//	{ R - 1, B - 2 },
//	{ R - 1, 2 },
	{ 3, 3 },
//	{ 3, B - 3 },
//	{ R - 3, B - 3 },
//	{ R - 3, 3 },
	{ 3, 2 },
//	{ 3, B - 2 },
//	{ R - 3, B - 2 },
//	{ R - 3, 2 },
//	{ 2, 3 },
//	{ 2, B - 3 },
//	{ R - 2, B - 3 },
//	{ R - 2, 3 },
	{ 3, 1 },
//	{ 3, B - 1 },
//	{ R - 3, B - 1 },
//	{ R - 3, 1 },
//	{ 1, 3 },
//	{ 1, B - 3 },
//	{ R - 1, B - 3 },
//	{ R - 1, 3 },
	{ 4, 4 },
//	{ 4, B - 4 },
//	{ R - 4, B - 4 },
//	{ R - 4, 4 },
	{ 4, 3 },
//	{ 4, B - 3 },
//	{ R - 4, B - 3 },
//	{ R - 4, 3 },
//	{ 3, 4 },
//	{ 3, B - 4 },
//	{ R - 3, B - 4 },
//	{ R - 3, 4 },
	{ 4, 2 },
//	{ 4, B - 2 },
//	{ R - 4, B - 2 },
//	{ R - 4, 2 },
//	{ 2, 4 },
//	{ 2, B - 4 },
//	{ R - 2, B - 4 },
//	{ R - 2, 4 },
	{ 4, 1 },
//	{ 4, B - 1 },
//	{ R - 4, B - 1 },
//	{ R - 4, 1 },
//	{ 1, 4 },
//	{ 1, B - 4 },
//	{ R - 1, B - 4 },
//	{ R - 1, 4 },
//	{ 0, 7 },
//	{ R - 0, 7 },
//	{ 7, 0 },
//	{ 7, B - 0 },
//	{ 1, 7 },
//	{ R - 1, 7 },
	{ 7, 1 },
//	{ 7, B - 1 },
//	{ 2, 7 },
//	{ R - 2, 7 },
	{ 7, 2 },
//	{ 7, B - 2 },
//	{ 3, 7 },
//	{ R - 3, 7 },
	{ 7, 3 },
//	{ 7, B - 3 },
//	{ 4, 7 },
//	{ R - 4, 7 },
	{ 7, 4 },
//	{ 7, B - 4 },
	{ 14, 7 },
};
#define EXPERT_POSITIONS	(sizeof(StartExpertPositions) / sizeof(StartExpertPositions[0]))


void SetTestingFiles(int wmId)
{
	time_t now;
	struct tm * date;

	prefixString[0] = '.';
	prefixString[1] = 0;
	if (wmId == IDM_TEST200x100
	||	wmId == IDM_TEST1000x100
	||	wmId == IDM_TESTSTARTINGPOS
	||	wmId == IDM_TESTENPLAYS
	||	wmId == IDM_TESTGUESS1OFFSETS)
	{
		TimeStartOverall();
		startSeed = seed;
		seedsUsed = 0;
		winAllTotal = 0;
		if (startX != START_X
		|| startY != START_Y)
			sprintf(startString, "start=%d,%d ", startX, startY);
		else
			startString[0] = 0;
		if (endPlayCount != END_PLAY_COUNT)
			sprintf(endPlayString, "endPlay=%d ", endPlayCount);
		else
			endPlayString[0] = 0;
		if (guess1Offset != GUESS1_OFFSET)
			sprintf(offsetString, "offset=%3.2f ", guess1Offset);
		else
			offsetString[0] = 0;
		now = time(NULL);
		date = localtime(&now);
		if (wmId == IDM_TEST200x100
		|| wmId == IDM_TEST1000x100)
		{
			testingMax = 100;						// number of puzzles to test
			if (seed == 0)							// use default seed if one is not provided
			{
				startSeed = seed = 1;
				RandomSeed(seed);
			}
			if (wmId == IDM_TEST200x100)
			{
				lastSeed = seed + 200;				// stop when we get to this seed
				testingAllTotal = 200 * testingMax;
				sprintf(prefixString, ".\\Test 200x100 %s%s%s%d.%d.%d %d.%d.%d",
					startString, endPlayString, offsetString,
					1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			}
			else
			{
				lastSeed = seed + 1000;				// stop when we get to this seed
				testingAllTotal = 1000 * testingMax;
				sprintf(prefixString, ".\\Test 1000x100 %s%s%s%d.%d.%d %d.%d.%d",
					startString, endPlayString, offsetString,
					1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			}
//			lastSeed = seed + 2;					// stop when we get to this seed
			CreateDirectory(prefixString, NULL);	// create test directory
			sprintf(testAllFileName, "%s\\results.txt", prefixString);
			testAllResults = fopen(testAllFileName, "w");
		}
		else if (wmId == IDM_TESTSTARTINGPOS)
		{
			if (seed == 0)							// use default seed if one is not provided
			{
				startSeed = seed = 1;
				RandomSeed(seed);
			}
			testStart = 0;
			if (width == 9)
			{
				startPositions = SMALL_POSITIONS;
				startX = StartSmallPositions[0][0];
				startY = StartSmallPositions[0][1];
			}
			else if (width == 16)
			{
				startPositions = MEDIUM_POSITIONS;
				startX = StartMediumPositions[0][0];
				startY = StartMediumPositions[0][1];
			}
			else 
			{
				startPositions = EXPERT_POSITIONS;
				startX = StartExpertPositions[0][0];
				startY = StartExpertPositions[0][1];
			}
			testingMax = TEST_SAMPLES;				// number of puzzles to test
			testingAllTotal = MAX_SEEDS * testingMax;
			sprintf(prefixString, ".\\Test StartPos %s%s%d.%d.%d %d.%d.%d",
				endPlayString, offsetString,
				1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			CreateDirectory(prefixString, NULL);	// create test directory
			sprintf(testAllFileName, "%s\\results.txt", prefixString);
			testAllResults = fopen(testAllFileName, "w");
		}
		else if (wmId == IDM_TESTENPLAYS)
		{
			if (seed == 0)							// use default seed if one is not provided
			{
				startSeed = seed = 1;
				RandomSeed(seed);
			}
			testEndPlay = 0;
			endPlayCount = EndPlayValues[0];
			testingMax = TEST_SAMPLES;				// number of puzzles to test
			testingAllTotal = MAX_SEEDS * testingMax;
			sprintf(prefixString, ".\\Test EndPlay %s%s%d.%d.%d %d.%d.%d",
				startString, offsetString,
				1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			CreateDirectory(prefixString, NULL);	// create test directory
			sprintf(testAllFileName, "%s\\results.txt", prefixString);
			testAllResults = fopen(testAllFileName, "w");
		}
		else if (wmId == IDM_TESTGUESS1OFFSETS)
		{
			if (seed == 0)							// use default seed if one is not provided
			{
				startSeed = seed = 1;
				RandomSeed(seed);
			}
			RandomSeed(seed);
			testOffset = 0;
			guess1Offset = Guess1Offsets[0];
			testingMax = TEST_SAMPLES;				// number of puzzles to test
			testingAllTotal = MAX_SEEDS * testingMax;
			sprintf(prefixString, ".\\Test Guess1Offset %s%s%d.%d.%d %d.%d.%d",
				startString, endPlayString,
				1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
			CreateDirectory(prefixString, NULL);	// create test directory
			sprintf(testAllFileName, "%s\\results.txt", prefixString);
			testAllResults = fopen(testAllFileName, "w");
		}
		if (testAllResults != NULL)
		{
			if (wmId != IDM_TESTSTARTINGPOS)
				fprintf(testAllResults, "start=%d,%d\n", startX, startY);
			if (wmId != IDM_TESTENPLAYS)
				fprintf(testAllResults, "endPlay=%d\n", endPlayCount);
			if (wmId != IDM_TESTGUESS1OFFSETS)
				fprintf(testAllResults, "Guess1Offset=%3.2f\n", guess1Offset);
			if (testStart < 0
			&& testEndPlay < 0
			&& testOffset < 0)
			{
				fprintf(testAllResults, "seed       wins\n---------------\n");
			}
			else
			{
				fprintf(testAllResults, "seed\t");
				for (int i = 0; i < MAX_SEEDS; i++)
					fprintf(testAllResults, "%d\t", startSeed + i);
				fprintf(testAllResults, "Totals\tPercent\tStd.Dev.\n");
				fprintf(testAllResults, "--------------------------------------------------------------------------------------------------------------\n");
				if (testStart >= 0)
					fprintf(testAllResults, "%d,%d\t", startX, startY);
				else if (testEndPlay >= 0)
					fprintf(testAllResults, "%d\t", endPlayCount);
				else if (testOffset >= 0)
					fprintf(testAllResults, "%3.2f\t", guess1Offset);
			}
		}
	}
	else if (wmId == IDM_TEST10)
		testingMax = 10;						// number of puzzles to test
	else if (wmId == IDM_TEST100)
		testingMax = 98;						// number of puzzles to test
	else if (wmId == IDM_TEST1000)
		testingMax = 1000;						// number of puzzles to test
	else
		testingMax = 10000;						// number of puzzles to test
}


void StartTest()
{
	time_t now;
	struct tm * date;

	saveSeed = seed;							// used later in the results file
	seed = 0;									// clear seed for next time
	testing = 1;								// start with this
	for (int i = 0; i < MAX_RULES; i++)
		rulesCountTotal[i] = 0;
	winTotal = 0;
	guess1Failed = 0;
	guess2Failed = 0;
	winProb = 0;
	lossProb = 0;
	guess1Prob = 0;
	guess2Prob = 0;
	guessesPerWin = 0;
	guessesPerLoss = 0;
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
		winsWithGuesses[i] = 0;
	if (saveSeed != 0)
		sprintf(seedString, "seed=%d ", saveSeed);
	else
		seedString[0] = 0;
	if (startX != START_X
	|| startY != START_Y)
		sprintf(startString, "start=%d,%d ", startX, startY);
	else
		startString[0] = 0;
	if (endPlayCount != END_PLAY_COUNT)
		sprintf(endPlayString, "endPlay=%d ", endPlayCount);
	else
		endPlayString[0] = 0;
	if (guess1Offset != GUESS1_OFFSET)
		sprintf(offsetString, "offset=%3.2f ", guess1Offset);
	else
		offsetString[0] = 0;
	now = time(NULL);
	date = localtime(&now);
	sprintf(testFilePath, "%s\\Test %s%s%s%s%d.%d.%d %d.%d.%d",
		prefixString, seedString, startString, endPlayString, offsetString,
		1900 + date->tm_year, date->tm_mon + 1, date->tm_mday, date->tm_hour, date->tm_min, date->tm_sec);
	CreateDirectory(testFilePath, NULL);		// create test directory
	sprintf(testFileName, "%s\\results.txt", testFilePath);
	testResults = fopen(testFileName, "w");
}


bool	GetNextTest()
{
	if (testAllResults == NULL)
		return false;
	if (testStart < 0
	&&	testEndPlay < 0
	&&	testOffset < 0)
	{
		++seedsUsed;
		seed = ++saveSeed;
		if (seed == lastSeed)					// we are done
		{
			seed = 0;							// clear seed we just set
			SaveTotalResults(testAllResults);	// display all stats
			ClearTotalStats();
			return false;
		}
	}
	else if (++seedsUsed == MAX_SEEDS)
	{
		char statsFileName[MAX_PATH];
		FILE * statsFile;
		sprintf(statsFileName, "%s\\stats start=%d,%d endPlay=%d guess1Offset=%.2f.txt",
			prefixString, startX, startY, endPlayCount, guess1Offset);
		statsFile = fopen(statsFileName, "w");
		if (statsFile != NULL)
		{
			SaveTotalResults(statsFile);
			ClearTotalStats();
			fclose(statsFile);
		}
		seedsUsed = 0;
		seed = startSeed;
		winAllTotal = 0;
		if (testStart >= 0)
		{
			if (++testStart >= startPositions)
			{
				testStart = -1;
				return false;
			}
			if (width == 9)
			{
				startX = StartSmallPositions[testStart][0];
				startY = StartSmallPositions[testStart][1];
			}
			else if (width == 16)
			{
				startX = StartMediumPositions[testStart][0];
				startY = StartMediumPositions[testStart][1];
			}
			else
			{
				startX = StartExpertPositions[testStart][0];
				startY = StartExpertPositions[testStart][1];
			}
			fprintf(testAllResults, "%d,%d\t", startX, startY);
		}
		else if (testEndPlay >= 0)
		{
			if (++testEndPlay >= END_PLAY_VALUES)
			{
				testEndPlay = -1;
				return false;
			}
			endPlayCount = EndPlayValues[testEndPlay];
			fprintf(testAllResults, "%d\t", endPlayCount);
		}
		else if (testOffset >= 0)
		{
			if (++testOffset >= GUESS1_OFFSETS)
			{
				testOffset = -1;
				return false;
			}
			guess1Offset = Guess1Offsets[testOffset];
			fprintf(testAllResults, "%3.2f\t", guess1Offset);
		}
	}
	else
		seed = ++saveSeed;
	RandomSeed(seed);
	StartTest();
	return true;
}


void Draw(HWND hWnd, HDC hdc, PAINTSTRUCT ps)
{
	HDC hdcMem = CreateCompatibleDC(hdc);
	HBITMAP hbmOld;
	int offset;

	// draw background
	if (width == MAX_WIDTH)
	{
		offset = WIDTH_OFFSET;
		hbmOld = (HBITMAP)SelectObject(hdcMem, hBackgndExpert);		// select new object and save the old
	}
	else if (width == 9)
	{
		offset = WIDTH_OFFSET + 10 * TILE_WIDTH;
		hbmOld = (HBITMAP)SelectObject(hdcMem, hBackgndSmall);		// select new object and save the old
	}
	else
	{
		offset = WIDTH_OFFSET + 7 * TILE_WIDTH;
		hbmOld = (HBITMAP)SelectObject(hdcMem, hBackgndMedium);		// select new object and save the old
	}
	if (ps.rcPaint.left == 0										// redraw the whole screen
	|| ps.rcPaint.right == BACKGND_WIDTH
	|| ps.rcPaint.bottom == BACKGND_HEIGHT)
	{
		BitBlt(hdc, 0, 0, BACKGND_WIDTH, BACKGND_HEIGHT, hdcMem, 0, 0, SRCCOPY);
	}
	else if (ps.rcPaint.left != 0									// not drawing the whole screen
	&& ps.rcPaint.left != Interior.left)							// or the interior
	{
		BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, ps.rcPaint.right - ps.rcPaint.left, ps.rcPaint.bottom - ps.rcPaint.top, hdcMem, ps.rcPaint.left, ps.rcPaint.top, SRCCOPY);
		SelectObject(hdcMem, hbmOld);								// restore the old object
		DeleteDC(hdcMem);											// delete 
		return;
	}
	else															// erase the rule selection
	{
		BitBlt(hdc, RuleRect.left, RuleRect.top, RuleRect.right - RuleRect.left, RuleRect.bottom - RuleRect.top, hdcMem, RuleRect.left, RuleRect.top, SRCCOPY);
	}

	// draw mines
	SelectObject(hdcMem, hMine);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (exposed[y][x] == EXPOSE_EXPODE)
				BitBlt(hdc, offset + x * TILE_WIDTH, HEIGHT_OFFSET + y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, hdcMem, TILE_WIDTH, 0, SRCCOPY);
			else if (exposed[y][x] == EXPOSE_MINE)
			{
				if ((gameOver || cheating)
				&& puzzle[y][x] == 0)			// mine should not be there
					BitBlt(hdc, offset + x * TILE_WIDTH, HEIGHT_OFFSET + y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, hdcMem, 2*TILE_WIDTH, 0, SRCCOPY);
				else
					BitBlt(hdc, offset + x * TILE_WIDTH, HEIGHT_OFFSET + y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, hdcMem, 0, 0, SRCCOPY);
			}
			else if (cheating					// show mines when cheating
			&&  exposed[y][x] < 0)
			{
				if (puzzle[y][x] != 0)			// mine should not be there
					BitBlt(hdc, offset + x * TILE_WIDTH, HEIGHT_OFFSET + y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, hdcMem, 0, 0, SRCCOPY);
			}
		}
	}

	// draw numbers
	SelectObject(hdcMem, hNumbers);
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (exposed[y][x] >= 0
			&& exposed[y][x] <= 8)
				BitBlt(hdc, offset + x * TILE_WIDTH, HEIGHT_OFFSET + y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT, hdcMem, exposed[y][x] * TILE_WIDTH, 0, SRCCOPY);
		}
	}

	// draw number of mines remaining
	SelectObject(hdcMem, hBackgndExpert);
	BitBlt(hdc, CountRect.left, CountRect.top, CountRect.right - CountRect.left, CountRect.bottom - CountRect.top, hdcMem, CountRect.left, CountRect.top, SRCCOPY);
	BitBlt(hdc, ProbRect.left, ProbRect.top, ProbRect.right - ProbRect.left, ProbRect.bottom - ProbRect.top, hdcMem, ProbRect.left, ProbRect.top, SRCCOPY);
	HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT logfont;
	GetObject(hFont, sizeof(LOGFONT), &logfont);
	logfont.lfHeight = 16;
	logfont.lfWeight = 1000;		// 400=normal 700=bold 1000=max
	HFONT hNewFont = CreateFontIndirect(&logfont);
	HFONT hOldFont = (HFONT)SelectObject(hdc, hNewFont);
	SetBkMode(hdc, TRANSPARENT);
	SetTextColor(hdc, RGB(255, 255, 255));
	char string[20];
//	if (cheating)
//		sprintf(string, "%d", remainingCount);
//	else
		sprintf(string, "%d", mineCount);
	DrawText(hdc, string, -1, &CountRect, DT_CENTER | DT_TOP);

	// draw probability 
	sprintf(string, "%d%%", (int)(100 * prob));
	DrawText(hdc, string, -1, &ProbRect, DT_CENTER | DT_TOP);
	SelectObject(hdc, hOldFont);										// reset font

	// show game over
	SelectObject(hdcMem, hBackgndExpert);								// clear game over area in case it was used last frame
	BitBlt(hdc, GameOverRect.left, GameOverRect.top, GameOverRect.right - GameOverRect.left, GameOverRect.bottom - GameOverRect.top, hdcMem, GameOverRect.left, GameOverRect.top, SRCCOPY);
	if (gameOver)
	{
		hOldFont = (HFONT)SelectObject(hdc, hNewFont);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));
		DrawText(hdc, "Game Over", -1, &GameOverRect, DT_CENTER | DT_TOP);
		SelectObject(hdc, hOldFont);									// reset font
	}

	// show last rule used
	if (lastRuleUsed >= 0)
	{
		hOldFont = (HFONT)SelectObject(hdc, hNewFont);
		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(0, 0, 0));
		DrawText(hdc, RuleText[lastRuleUsed], -1, &RuleRect, DT_CENTER | DT_TOP);
		SelectObject(hdc, hOldFont);									// reset font
	}

	SelectObject(hdcMem, hbmOld);										// restore the old object
	DeleteObject(hNewFont);
	DeleteDC(hdcMem);													// delete 												
}


bool FindPosition(HWND hWnd, int h, int v, int & x, int & y)
{
	int offset;
	if (width == MAX_WIDTH)
		offset = WIDTH_OFFSET;
	else if (width == 9)
		offset = WIDTH_OFFSET + 10 * TILE_WIDTH;
	else 
		offset = WIDTH_OFFSET + 7 * TILE_WIDTH;
	x = (h - WIDTH_OFFSET) / TILE_WIDTH;
	y = (v - HEIGHT_OFFSET) / TILE_HEIGHT;
	if (0 <= x && x < width
	&&	0 <= y && y < height)
	{
		return true;
	}
	return false;
}


// Message handler for getting a random seed
INT_PTR CALLBACK Statistics(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hwndOwner;
	RECT rcOwner, rcWindow;
	int width, widthOwner;
	HWND msg;
	char string[128];

	switch (message)
	{
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
			hwndOwner = GetDesktopWindow();
		GetWindowRect(hwndOwner, &rcOwner);
		widthOwner = rcOwner.right - rcOwner.left;
		GetWindowRect(hDlg, &rcWindow);
		width = rcWindow.right - rcWindow.left;
		SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (widthOwner - width) / 2, rcOwner.top + 50, 0, 0, SWP_NOSIZE);
		msg = GetDlgItem(hDlg, IDC_STATS0);
		sprintf(string, "%d", totalTests);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS1);
		sprintf(string, "%d", totalWins);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS2);
		if (totalTests != 0)
			sprintf(string, "%.2f%%", 100.0 * totalWins / totalTests);
		else
			sprintf(string, "0.0%%");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS4);
		sprintf(string, "%d", totalGuess1);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS5);
		sprintf(string, "%d", totalGuess2);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS6);
		if (totalGuess1 != 0)
			sprintf(string, "%3.1f%%", (100.0 * totalGuess1Failed) / totalGuess1);
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS7);
		if (totalGuess2 != 0)
			sprintf(string, "%3.1f%%", (100.0 * totalGuess2Failed) / totalGuess2);
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS8);
		if (totalWins != 0)
			sprintf(string, "%4.2f", (double)totalGuessesPerWin / totalWins);
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS9);
		if ((totalTests - totalWins) != 0)
			sprintf(string, "%4.2f", (double)totalGuessesPerLoss / (totalTests - totalWins));
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS10);
		if (totalWins > 0)
			sprintf(string, "%4.1f%%", 100.0 * totalWinProb / totalWins);
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS11);
		if ((totalTests - totalWins) > 0)
			sprintf(string, "%4.1f%%", 100.0 * totalLossProb / (totalTests - totalWins));
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS12);
		if (totalGuess1 != 0)
			sprintf(string, "%4.1f%%", (100.0 * totalGuess1Prob) / totalGuess1);
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATS13);
		if (totalGuess2 != 0)
			sprintf(string, "%4.1f%%", (100.0 * totalGuess2Prob) / totalGuess2);
		else
			sprintf(string, "0.0");
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS0);
		sprintf(string, "0 : %-4d", totalWinsWithGuesses[0]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS1);
		sprintf(string, "1 : %-4d", totalWinsWithGuesses[1]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS2);
		sprintf(string, "2 : %-4d", totalWinsWithGuesses[2]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS3);
		sprintf(string, "3 : %-4d", totalWinsWithGuesses[3]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS4);
		sprintf(string, "4 : %-4d", totalWinsWithGuesses[4]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS5);
		sprintf(string, "5 : %-4d", totalWinsWithGuesses[5]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS6);
		sprintf(string, "6 : %-4d", totalWinsWithGuesses[6]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS7);
		sprintf(string, "7 : %-4d", totalWinsWithGuesses[7]);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STATSWINS8);
		sprintf(string, "8 : %-4d", totalWinsWithGuesses[8]);
		SetWindowText(msg, string);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			ClearTotalStats();
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for getting a random seed
INT_PTR CALLBACK GetRandomSeed(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hwndOwner;
	RECT rcOwner, rcWindow;
	int width, widthOwner;
	HWND msg;
	char string[16];

	switch (message)
	{
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
			hwndOwner = GetDesktopWindow();
		GetWindowRect(hwndOwner, &rcOwner);
		widthOwner = rcOwner.right - rcOwner.left;
		GetWindowRect(hDlg, &rcWindow);
		width = rcWindow.right - rcWindow.left;
		SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (widthOwner - width) / 2, rcOwner.top + 50, 0, 0, SWP_NOSIZE);
		msg = GetDlgItem(hDlg, IDC_RANDOMSEED);
		SetWindowText(msg, "");
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			msg = GetDlgItem(hDlg, IDC_RANDOMSEED);
			GetWindowText(msg, string, sizeof(string));
			seed = (U32)atoi(string);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for getting the starting position
INT_PTR CALLBACK GetStartingPos(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hwndOwner;
	RECT rcOwner, rcWindow;
	int widthW, widthOwner;
	HWND msg;
	char string[16];

	switch (message)
	{
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
			hwndOwner = GetDesktopWindow();
		GetWindowRect(hwndOwner, &rcOwner);
		widthOwner = rcOwner.right - rcOwner.left;
		GetWindowRect(hDlg, &rcWindow);
		widthW = rcWindow.right - rcWindow.left;
		SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (widthOwner - widthW) / 2, rcOwner.top + 50, 0, 0, SWP_NOSIZE);
		msg = GetDlgItem(hDlg, IDC_STARTX);
		sprintf(string, "%d", startX);
		SetWindowText(msg, string);
		msg = GetDlgItem(hDlg, IDC_STARTY);
		sprintf(string, "%d", startY);
		SetWindowText(msg, string);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			msg = GetDlgItem(hDlg, IDC_STARTX);
			GetWindowText(msg, string, sizeof(string));
			startX = (U32)atoi(string);
			if (startX < 0)
				startX = 0;
			else if (startX >= width)
				startX = width - 1;
			msg = GetDlgItem(hDlg, IDC_STARTY);
			GetWindowText(msg, string, sizeof(string));
			startY = (U32)atoi(string);
			if (startY < 0)
				startY = 0;
			else if (startY >= height)
				startY = height - 1;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for getting endPlayCount
INT_PTR CALLBACK GetEndPlayCount(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hwndOwner;
	RECT rcOwner, rcWindow;
	int width, widthOwner;
	HWND msg;
	char string[16];

	switch (message)
	{
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
			hwndOwner = GetDesktopWindow();
		GetWindowRect(hwndOwner, &rcOwner);
		widthOwner = rcOwner.right - rcOwner.left;
		GetWindowRect(hDlg, &rcWindow);
		width = rcWindow.right - rcWindow.left;
		SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (widthOwner - width) / 2, rcOwner.top + 50, 0, 0, SWP_NOSIZE);
		msg = GetDlgItem(hDlg, IDC_SETENDPLAY);
		sprintf(string, "%d", endPlayCount);
		SetWindowText(msg, string);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			msg = GetDlgItem(hDlg, IDC_SETENDPLAY);
			GetWindowText(msg, string, sizeof(string));
			endPlayCount = (U32)atoi(string);
			if (endPlayCount < 6)
				endPlayCount = 6;
			else if (endPlayCount > 30)
				endPlayCount = 30;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for getting guess1Offset
INT_PTR CALLBACK GetGuess1Offset(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hwndOwner;
	RECT rcOwner, rcWindow;
	int width, widthOwner;
	HWND msg;
	char string[16];

	switch (message)
	{
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
			hwndOwner = GetDesktopWindow();
		GetWindowRect(hwndOwner, &rcOwner);
		widthOwner = rcOwner.right - rcOwner.left;
		GetWindowRect(hDlg, &rcWindow);
		width = rcWindow.right - rcWindow.left;
		SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (widthOwner - width) / 2, rcOwner.top + 50, 0, 0, SWP_NOSIZE);
		msg = GetDlgItem(hDlg, IDC_SETGUESS1OFFSET);
		sprintf(string, "%d", (int)(100 * guess1Offset));
		SetWindowText(msg, string);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			msg = GetDlgItem(hDlg, IDC_SETGUESS1OFFSET);
			GetWindowText(msg, string, sizeof(string));
			guess1Offset = atoi(string) / 100.0;
			if (guess1Offset < -.20)
				guess1Offset = -.20;
			else if (guess1Offset > .20)
				guess1Offset = .20;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	HWND hwndOwner;
	RECT rcOwner, rcWindow;
	int width, widthOwner;

	switch (message)
	{
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hDlg)) == NULL)
			hwndOwner = GetDesktopWindow();
		GetWindowRect(hwndOwner, &rcOwner);
		widthOwner = rcOwner.right - rcOwner.left;
		GetWindowRect(hDlg, &rcWindow);
		width = rcWindow.right - rcWindow.left;
		SetWindowPos(hDlg, HWND_TOP, rcOwner.left + (widthOwner - width) / 2, rcOwner.top + 50, 0, 0, SWP_NOSIZE);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	char fname[MAX_PATH];
	HMENU hMenu = GetMenu(hWnd);							
	HMENU hSubMenu;

	switch (message)
	{
	case WM_CREATE:
		hBackgndExpert = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BACKGNDEXPERT));
		hBackgndMedium = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BACKGNDMEDIUM));
		hBackgndSmall = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_BACKGNDSMALL));
		hMine = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_MINE));
		hNumbers = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_NUMBERS));
		if (hBackgndExpert == NULL
		|| hBackgndMedium == NULL
		|| hBackgndSmall == NULL
		|| hMine == NULL
		|| hNumbers == NULL)
			MessageBox(hWnd, "Could not load art!", "Error", MB_OK | MB_ICONEXCLAMATION);
		RandomInit();										// init random numbers
		hSubMenu = GetSubMenu(hMenu, SUBMENU_PLAY);
//		for (int i = 0; i < MAX_RULES; i++)					// for all rules
		for (int i = RULE_1; i < MAX_RULES; i++)
		{
			rules[i] = true;
//			CheckMenuItem(hSubMenu, SUBMENU_RULES + i, MF_BYPOSITION | MF_CHECKED);	// for all rules
			if (i >= GUESS_1)
				CheckMenuItem(hSubMenu, SUBMENU_RULES + i - GUESS_1, MF_BYPOSITION | MF_CHECKED);
		}
		TimeInit();
		guess1Offset = GUESS1_OFFSET;
		endPlayCount = END_PLAY_COUNT;
		ClearTotalStats();
		testing = 0;
		savePuzzles = false;
		testStart = -1;
		testEndPlay = -1;
		testOffset = -1;
		seed = 0;											// no seed set
		cheating = false;
		startX = START_X;
		startY = START_Y;
		CheckMenuItem(hSubMenu, SUBMENU_CHEAT, MF_BYPOSITION | MF_UNCHECKED);
		backup = false;
		EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
		started = false;
		width = MAX_WIDTH;
		height = MAX_HEIGHT;
		minesStart = 99;
		if (LoadGameFile() == false)						// load previous game if possible
			NewGame(startX, startY);						// start new game
		if (remainingCount == 0)
		{
			gameOver = true;
			EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
			EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
		}
		else
		{
			EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
			EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
		}
		break;
	case WM_TIMER:
		switch (wParam)
		{
		case IDT_TESTTIMER:
			if (solving)									// not done wait until we are
				break;
			KillTimer(hWnd, IDT_TESTTIMER);
			NewGame(startX, startY);						// start next puzzle
			solving = true;
			while (solving && gameOver == false)
			{
				backup = false;
				MakeMove();									// make some move until done
				if (backup == false)						// no move found 
					break;
			}
			solving = false;
			SaveResults(testResults);						// save results of the puzzle including errors
			for (int i = 0; i < MAX_RULES; i++)
				rulesCountTotal[i] += rulesCount[i];		// save totals
			if (gameOver && remainingCount != 0)			// we encountered an error
			{
				BackUp();									// backup past error
			}
			if (savePuzzles)
			{
				sprintf(testFileName, "%s\\puzzle%d.txt", testFilePath, testing);
				SavePuzzle(testFileName);
			}
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			if (++testing <= testingMax)
			{
				SetTimer(hWnd, IDT_TESTTIMER, SPEED, (TIMERPROC)NULL);
			}
			else
			{
				ShowTotalResults(testResults, testingMax, saveSeed);	// save stats 
				fclose(testResults);
				testResults = NULL;
				wins[seedsUsed] = winTotal;					// save wins for standard deviation calculation
				if (testStart < 0
				&& testEndPlay < 0
				&& testOffset < 0)
					SaveAllResults(testAllResults, saveSeed, seedsUsed, 0, saveSeed + 1 == lastSeed, (seedsUsed + 1) * 100);
				else
					SaveAllResults(testAllResults, saveSeed, -1, MAX_SEEDS * testingMax, seedsUsed == MAX_SEEDS - 1, testingAllTotal);
				if (GetNextTest())							// set up for next test
				{
					SetTimer(hWnd, IDT_TESTTIMER, SPEED, (TIMERPROC)NULL);
					break;
				}
				testing = 0;								// end testing
				seed = saveSeed = 0;						// discard seed
				startX = saveStartX;
				startY = saveStartY;
				guess1Offset = saveGuess1Offset;
				endPlayCount = saveEndPlayCount;
				if (testAllResults != NULL)
					fclose(testAllResults);
				testAllResults = NULL;
				if (gameOver)
				{
					EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
				}
				else
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
				}
				if (backup)
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_ENABLED);
				else
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
			}
			break;
		case IDT_TIMER:
			MakeMove();										// make some move
			if (solving == false)
			{
				KillTimer(hWnd, IDT_TIMER);
				hSubMenu = GetSubMenu(hMenu, SUBMENU_PLAY);
				CheckMenuItem(hSubMenu, SUBMENU_SOLVE, MF_BYPOSITION | MF_UNCHECKED);
				if (gameOver)
				{
					EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
				}
				else
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
				}
				if (backup)
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_ENABLED);
				else
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
			}
			RedrawWindow(hWnd, &Interior, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}
		break;
	case WM_LBUTTONDOWN:
		if (gameOver == false)
		{
			int x, y;
			if (FindPosition(hWnd, (int)(lParam & 0xffff), (int)(lParam >> 16), x, y))
			{
				if (exposed[y][x] == EXPOSE_MINE)		// remove mine
				{
					mineCount++;
					exposed[y][x] = -1;
				}
				else if (exposed[y][x] >= 0)			// do nothing if already exposed
					break;
				lastRuleUsed = -1;
				lastProb = prob;
				SaveBackUp();							// backup before exposing tile
				Expose(x, y);							// expose it now
				if (gameOver)
				{
					EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
				}
				else
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
				}
				EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_ENABLED);
				RedrawWindow(hWnd, &Interior, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			}
		}
		break;
	case WM_RBUTTONDOWN:
		if (gameOver == false)
		{
			int x, y;
			if (FindPosition(hWnd, (int)(lParam & 0xffff), (int)(lParam >> 16), x, y))
			{
				if (exposed[y][x] == EXPOSE_MINE)		// remove mine
				{
					lastRuleUsed = -1;
					SaveBackUp();						// backup before exposing tile
					mineCount++;
					exposed[y][x] = -1;
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				else if (exposed[y][x] < 0)
				{
					lastRuleUsed = -1;
					lastProb = prob;
					SaveBackUp();						// backup before exposing tile
					mineCount--;
					exposed[y][x] = EXPOSE_MINE;
					RedrawWindow(hWnd, &Interior, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_ENABLED);
			}
		}
		break;
	case WM_COMMAND:
	{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
			case IDM_NEW_EXPERT:
				if (testing)
					break;
				width = MAX_WIDTH;
				height = MAX_HEIGHT;
				minesStart = 99;
				goto NEW;
			case IDM_NEW_MEDIUM:
				if (testing)
					break;
				width = 16;
				height = 16;
				minesStart = 40;
				goto NEW;
			case IDM_NEW_SMALL:
				if (testing)
					break;
				width = 9;
				height = 9;
				minesStart = 10;
NEW:			if (startX >= width							// reset starting position if now invalid
				|| startY >= height)
				{
					startX = START_X;
					startY = START_Y;
				}
				NewGame(startX, startY);					// start new game
				EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_ENABLED);		// enable save game now
LOAD:			lastRuleUsed = -1;
				seed = 0;									// clear seed
				if (remainingCount == 0)
				{
					gameOver = true;
					EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
				}
				else
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
				}
				EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			case IDM_LOAD:
				if (testing)
					break;
				solving = false;
				if (LoadFileName(hWnd, fname, "Text file", "txt"))
				{
					if (LoadPuzzle(fname))
						goto LOAD;
					else
					{
						MessageBox(hWnd, "Could not load puzzle!", "Data Error", MB_OK | MB_ICONEXCLAMATION);
						goto NEW;
					}
				}
				break;
			case IDM_SAVE:
				if (testing)
					break;
				if (started == false
				|| gameOver == true)
					break;
				fname[0] = 0;
				if (SaveFileName(hWnd, fname, "Text file", "txt"))
				{
					SavePuzzle(fname);
				}
				break;
			case IDM_SOLVE:
				if (testing)
					break;
				if (solving)
				{
					solving = false;
					KillTimer(hWnd, IDT_TIMER);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
					hSubMenu = GetSubMenu(hMenu, SUBMENU_PLAY);
					CheckMenuItem(hSubMenu, SUBMENU_SOLVE, MF_BYPOSITION | MF_UNCHECKED);
					if (backup)
						EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_ENABLED);
					else
						EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
				}
				else
				{
					solving = true;
					SetTimer(hWnd, IDT_TIMER, SPEED, (TIMERPROC)NULL);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
					hSubMenu = GetSubMenu(hMenu, SUBMENU_PLAY);
					CheckMenuItem(hSubMenu, SUBMENU_SOLVE, MF_BYPOSITION | MF_CHECKED);
				}
				break;
			case IDM_NEXTMOVE:
				if (testing)
					break;
				if (started == false						// just in case
				|| gameOver)
					break;
				solving = true;								// stop game if we make a mistake
				MakeMove();									// make some move
				solving = false;
				if (gameOver)
				{
					EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_GRAYED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_GRAYED);
				}
				else
				{
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
				}
				if (backup)
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_ENABLED);
				else
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
				RedrawWindow(hWnd, &Interior, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			case IDM_STATS:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_STATSBOX), hWnd, Statistics);
				break;
			case IDM_SETSEED:
				seed = 0;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_RANDOMSEEDBOX), hWnd, GetRandomSeed);
				if (seed != 0)
					RandomSeed(seed);
				break;
			case IDM_STARTINGPOS:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_STARTINGPOSBOX), hWnd, GetStartingPos);
				break;
			case IDM_SETENDPLAY:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SETENDPLAYBOX), hWnd, GetEndPlayCount);
				break;
			case IDM_SETGUESS1OFFSET:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SETGUESS1OFFSETBOX), hWnd, GetGuess1Offset);
				break;
			case IDM_SAVEPUZZLES:
				savePuzzles = !savePuzzles;
				hSubMenu = GetSubMenu(hMenu, SUBMENU_TEST);
				if (savePuzzles)
					CheckMenuItem(hSubMenu, SUBMENU_SAVEPUZZLES, MF_BYPOSITION | MF_CHECKED);
				else
					CheckMenuItem(hSubMenu, SUBMENU_SAVEPUZZLES, MF_BYPOSITION | MF_UNCHECKED);
				break;
			case IDM_TESTSTARTINGPOS:
			case IDM_TESTENPLAYS:
			case IDM_TESTGUESS1OFFSETS:
			case IDM_TEST10:
			case IDM_TEST100:
			case IDM_TEST1000:
			case IDM_TEST200x100:
			case IDM_TEST1000x100:
				if (testing != 0)							// skip if testing progress	
				{
					testingMax = testing;					// stop current test
					break;
				}
				saveStartX = startX;
				saveStartY = startY;
				saveGuess1Offset = guess1Offset;
				saveEndPlayCount = endPlayCount;
				solving = false;							// turn off any solving
				SetTestingFiles(wmId);						// set up test files and paths
				if (testAllResults == NULL					// file open failed
				&& (wmId == IDM_TEST200x100 || wmId == IDM_TEST1000x100))
				{
					MessageBox(hWnd, "Could not open results file!", "Error", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				StartTest();								// set up test files and start test
				if (testResults == NULL)					// file open failed
				{
					MessageBox(hWnd, "Could not open results file!", "Error", MB_OK | MB_ICONEXCLAMATION);
					break;
				}
				SetTimer(hWnd, IDT_TESTTIMER, SPEED, (TIMERPROC)NULL);
				break;
			case IDM_BACKUP:
				if (testing)
					break;
				if (BackUp())								// backup occurred
				{
					EnableMenuItem(hMenu, IDM_SAVE, MF_BYCOMMAND | MF_ENABLED);		// game over could be cleared
					EnableMenuItem(hMenu, IDM_SOLVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_NEXTMOVE, MF_BYCOMMAND | MF_ENABLED);
					EnableMenuItem(hMenu, IDM_BACKUP, MF_BYCOMMAND | MF_GRAYED);
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				}
				break;
			case IDM_HFLIP:
				if (testing)
					break;
				for (int y = 0; y < height; y++)
				{
					for (int x = 0; x < width/2; x++)
					{
						int temp = puzzle[y][width - 1 - x];
						puzzle[y][width - 1 - x] = puzzle[y][x];
						puzzle[y][x] = temp;
						temp = exposed[y][width - 1 - x];
						exposed[y][width - 1 - x] = exposed[y][x];
						exposed[y][x] = temp;
						temp = lastExposed[y][width - 1 - x];
						lastExposed[y][width - 1 - x] = lastExposed[y][x];
						lastExposed[y][x] = temp;
					}
				}
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			case IDM_VFLIP:
				if (testing)
					break;
				for (int y = 0; y < height/2; y++)
				{
					for (int x = 0; x < width; x++)
					{
						int temp = puzzle[height - 1 - y][x];
						puzzle[height - 1 - y][x] = puzzle[y][x];
						puzzle[y][x] = temp;
						temp = exposed[height - 1 - y][x];
						exposed[height - 1 - y][x] = exposed[y][x];
						exposed[y][x] = temp;
						temp = lastExposed[height - 1 - y][x];
						lastExposed[height - 1 - y][x] = lastExposed[y][x];
						lastExposed[y][x] = temp;
					}
				}
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			case IDM_CHEAT:
				cheating = !cheating;						// turn on/off cheats
				hSubMenu = GetSubMenu(hMenu, SUBMENU_PLAY);
				if (cheating)
					CheckMenuItem(hSubMenu, SUBMENU_CHEAT, MF_BYPOSITION | MF_CHECKED);
				else
					CheckMenuItem(hSubMenu, SUBMENU_CHEAT, MF_BYPOSITION | MF_UNCHECKED);
				RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				break;
			case IDM_RULE1:
			case IDM_RULE2:
			case IDM_RULE3:
			case IDM_RULE4:
			case IDM_RULE5:
			case IDM_RULE6:
			case IDM_RULE7:
			case IDM_RULE8:
			case IDM_RULE9:
			case IDM_RULE10:
			case IDM_GUESS1:
			case IDM_GUESS2:
				rules[wmId - IDM_RULE1] = !rules[wmId - IDM_RULE1];
				hSubMenu = GetSubMenu(hMenu, SUBMENU_PLAY);
				if (rules[wmId - IDM_RULE1])
//					CheckMenuItem(hSubMenu, SUBMENU_RULES + wmId - IDM_RULE1, MF_BYPOSITION | MF_CHECKED);
					CheckMenuItem(hSubMenu, SUBMENU_RULES + wmId - IDM_GUESS1, MF_BYPOSITION | MF_CHECKED);
				else
//					CheckMenuItem(hSubMenu, SUBMENU_RULES + wmId - IDM_RULE1, MF_BYPOSITION | MF_UNCHECKED);
					CheckMenuItem(hSubMenu, SUBMENU_RULES + wmId - IDM_GUESS1, MF_BYPOSITION | MF_UNCHECKED);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}
		}
		break;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			Draw(hWnd, hdc, ps);
			EndPaint(hWnd, &ps);
			DeleteDC(hdc);
		}
		break;
	case WM_DESTROY:
		if (testResults != NULL)
			fclose(testResults);
		testResults = NULL;
		if (testAllResults != NULL)
			fclose(testAllResults);
		testAllResults = NULL;
		if (gameOver == false)
			SaveGameFile();
		DeleteObject(hBackgndExpert);
		DeleteObject(hBackgndMedium);
		DeleteObject(hBackgndSmall);
		DeleteObject(hMine);
		DeleteObject(hNumbers);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MINESWEEPER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_MINESWEEPER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}


//
//	 Center the window in relation to the desktop 
//
void CenterWindow(HWND CW_h2)
{
	RECT CW_rect1, CW_rect2;
	int CW_midx, CW_midy, CW_wx, CW_wy;
	HWND CW_h1;

	CW_h1 = GetDesktopWindow();
	GetWindowRect(CW_h1, &CW_rect1);
	GetWindowRect(CW_h2, &CW_rect2);
	CW_midx = (CW_rect1.right + CW_rect1.left) >> 1;
	CW_midy = (CW_rect1.bottom + CW_rect1.top) >> 1;
	CW_wx = CW_rect2.right - CW_rect2.left;
	CW_wy = CW_rect2.bottom - CW_rect2.top;
	MoveWindow(CW_h2, CW_midx - (CW_wx >> 1), CW_midy - (CW_wy >> 1), CW_wx, CW_wy, false);
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_SYSMENU,
		0, 0, BACKGND_WIDTH + 16, BACKGND_HEIGHT + 59, NULL, NULL, hInstance, NULL);
	if (!hWnd)
	{
		return FALSE;
	}
	CenterWindow(hWnd);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_MINESWEEPER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MINESWEEPER));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
