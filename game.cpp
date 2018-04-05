#include "stdafx.h"

int		puzzle[MAX_HEIGHT][MAX_WIDTH];			// 1 for mine and 0 otherwise
int		exposed[MAX_HEIGHT][MAX_WIDTH];			// -1 if not exposed, 0-8 for numbers, EXPOSE_MINE for mine...
int		lastExposed[MAX_HEIGHT][MAX_WIDTH];		// used for backup
bool	rules[MAX_RULES];						// what rules to follow to solve puzzle
int		rulesCount[MAX_RULES];					// count of rules used
int		rulesCountTotal[MAX_RULES];				// total count of rules used for all puzzles tested
int		guesses;								// number of guesses so far this puzzle
int		lastRuleUsed;							// last rule used
bool	started;								// puzzle started
bool	solving;								// puzzle solving in progress
bool	gameOver;								// true when mine has been hit
bool	backup;									// if backup available
bool	cheating;								// show mines when cheating
int		mineCount;								// mines left to find
int		remainingCount;							// number left to expose
int		lastMineCount;							// mines left to find on backup
int		lastRemainingCount;						// number left to expose on backup
int		endPlayCount;							// default value for end play 
int		endGameCount;							// number before use just one tank list
double	guess1Offset;							// amount to offset the guess1 probability (negative is more guess1)

double	prob;									// current probability of getting this right
double	lastProb;								// last value of prob

int		unexposed[MAX_HEIGHT][MAX_WIDTH];
int		mines[MAX_HEIGHT][MAX_WIDTH];
int		needed[MAX_HEIGHT][MAX_WIDTH];
int		neighbors[MAX_HEIGHT][MAX_WIDTH];


// get the high resolution counter's accuracy	
static	LARGE_INTEGER	ticksPerSecond;
static	LARGE_INTEGER	tickStart;				// start time
static	LARGE_INTEGER	tickStartOverall;		// start overall time
																					
void	TimeInit()
{
	QueryPerformanceFrequency(&ticksPerSecond);
}

void	TimeStart()
{
	QueryPerformanceCounter(&tickStart);		// start timer
}

void	TimeStartOverall()
{
	QueryPerformanceCounter(&tickStartOverall);	// start timer
}

double	TimeEnd()
{
	LARGE_INTEGER	tickEnd;					// end time
	QueryPerformanceCounter(&tickEnd);			// end timer
	return (double)(tickEnd.QuadPart - tickStart.QuadPart) / (double)ticksPerSecond.QuadPart;
}

double	TimeEndOverall()
{
	LARGE_INTEGER	tickEnd;					// end time
	QueryPerformanceCounter(&tickEnd);			// end timer
	return (double)(tickEnd.QuadPart - tickStartOverall.QuadPart) / (double)ticksPerSecond.QuadPart;
}


void SaveBackUp()
{
	// save current board
	backup = true;
	lastMineCount = mineCount;
	lastRemainingCount = remainingCount;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			lastExposed[y][x] = exposed[y][x];
		}
	}
}


bool BackUp()
{
	if (backup == false)
		return false;
	solving = false;
	gameOver = false;
	backup = false;
	mineCount = lastMineCount;
	remainingCount = lastRemainingCount;
	lastRuleUsed = -1;
	prob = lastProb;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			exposed[y][x] = lastExposed[y][x];
		}
	}
	return true;
}


void BackUpChanged()
{
#if _DEBUG
	if (lastMineCount == mineCount
	&&	lastRemainingCount == remainingCount)
		NULL;
#endif
}


#if 0
void ClearStats(STATS & stats)
{
	stats.games = 0;
	stats.won = 0;
	stats.winStreak = 0;
	stats.loseStreak = 0;
	stats.streak = 0;
}


void UpdateStats(STATS & stats, bool win)
{
	stats.games++;
	if (win)
	{
		stats.won++;
		if (stats.streak <= 0)
			stats.streak = 1;
		else
			stats.streak++;
		if (stats.winStreak < stats.streak)
			stats.winStreak = stats.streak;
	}
	else
	{
		if (stats.streak >= 0)
			stats.streak = -1;
		else
			stats.streak--;
		if (stats.loseStreak > stats.streak)
			stats.loseStreak = stats.streak;
	}
}
#endif


int CountMines(int x, int y)
{
	if (exposed[y][x] >= 0)
		return exposed[y][x];
	int count = 0;
	count += (x > 0 && y > 0 && puzzle[y - 1][x - 1] != 0 ? 1 : 0);
	count += (y > 0 && puzzle[y - 1][x] != 0 ? 1 : 0);
	count += (x < width-1 && y > 0 && puzzle[y - 1][x + 1] != 0 ? 1 : 0);
	count += (x > 0 && puzzle[y][x - 1] != 0 ? 1 : 0);
	count += (puzzle[y][x] != 0 ? 1 : 0);
	count += (x < width - 1 && puzzle[y][x + 1] != 0 ? 1 : 0);
	count += (x > 0 && y < height - 1 && puzzle[y + 1][x - 1] != 0 ? 1 : 0);
	count += (y < height - 1 && puzzle[y + 1][x] != 0 ? 1 : 0);
	count += (x < width - 1 && y < height - 1 && puzzle[y + 1][x + 1] != 0 ? 1 : 0);
	return count;
}


void ExposeAll(int x, int y)
{
	if (x > 0 && y > 0 && exposed[y - 1][x - 1] < 0)
	{
		remainingCount--;
		exposed[y - 1][x - 1] = CountMines(x - 1, y - 1);
		if (exposed[y - 1][x - 1] == 0)
			ExposeAll(x - 1, y - 1);			// expose all neighbors too
	}
	if (y > 0 && exposed[y - 1][x] < 0)
	{
		remainingCount--;
		exposed[y - 1][x] = CountMines(x, y - 1);
		if (exposed[y - 1][x] == 0)
			ExposeAll(x, y - 1);				// expose all neighbors too
	}
	if (x < width - 1 && y > 0 && exposed[y - 1][x + 1] < 0)
	{
		remainingCount--;
		exposed[y - 1][x + 1] = CountMines(x + 1, y - 1);
		if (exposed[y - 1][x + 1] == 0)
			ExposeAll(x + 1, y - 1);			// expose all neighbors too
	}
	if (x > 0 && exposed[y][x - 1] < 0)
	{
		remainingCount--;
		exposed[y][x - 1] = CountMines(x - 1, y);
		if (exposed[y][x - 1] == 0)
			ExposeAll(x - 1, y);				// expose all neighbors too
	}
	if (x < width - 1 && exposed[y][x + 1] < 0)
	{
		remainingCount--;
		exposed[y][x + 1] = CountMines(x + 1, y);
		if (exposed[y][x + 1] == 0)
			ExposeAll(x + 1, y);				// expose all neighbors too
	}
	if (x > 0 && y < height - 1 && exposed[y + 1][x - 1] < 0)
	{
		remainingCount--;
		exposed[y + 1][x - 1] = CountMines(x - 1, y + 1);
		if (exposed[y + 1][x - 1] == 0)
			ExposeAll(x - 1, y + 1);			// expose all neighbors too
	}
	if (y < height - 1 && exposed[y + 1][x] < 0)
	{
		remainingCount--;
		exposed[y + 1][x] = CountMines(x, y + 1);
		if (exposed[y + 1][x] == 0)
			ExposeAll(x, y + 1);				// expose all neighbors too
	}
	if (x < width - 1 && y < height - 1 && exposed[y + 1][x + 1] < 0)
	{
		remainingCount--;
		exposed[y + 1][x + 1] = CountMines(x + 1, y + 1);
		if (exposed[y + 1][x + 1] == 0)
			ExposeAll(x + 1, y + 1);			// expose all neighbors too
	}
}


void Expose(int x, int y)
{
	if (exposed[y][x] >= 0)						// already exposed do nothing
		return;
	if (puzzle[y][x] != 0)						// we hit a mine
	{
		exposed[y][x] = EXPOSE_EXPODE;
		solving = false;						
		gameOver = true;
		MessageBeep(MB_ICONWARNING);			// sound end of solving
		return;
	}
	remainingCount--;
	exposed[y][x] = CountMines(x, y);			// count mines 	
	if (exposed[y][x] == 0)						// we need to expose all neighbors too
		ExposeAll(x, y);
	if (remainingCount == 0)
	{
		solving = false;
		gameOver = true;
		MessageBeep(MB_ICONWARNING);			// sound end of solving
	}
}


void NewGame(int startx, int starty)
{
	int x, y;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			exposed[y][x] = -1;
			puzzle[y][x] = 0;
		}
	}
	for (int i = 0; i < minesStart; i++)
	{
		while (true)
		{
			y = (int)Random(height);
			x = (int)Random(width);
			if (puzzle[y][x] != 0)					// do not put mines on top of another
				continue;
			if (y < starty - 1 || y > starty + 1	// stay away from starting area
			|| x < startx - 1 || x > startx + 1)
				break;
		} 
		puzzle[y][x] = 1;
	}
	started = true;
	backup = false;
	solving = false;
	gameOver = false;
	mineCount = minesStart;
	remainingCount = width * height - minesStart;
	for (int i = 0; i < MAX_RULES; i++)
		rulesCount[i] = 0;
	lastRuleUsed = -1;
	prob = 1.0;
	guesses = 0;
	Expose(startx, starty);							// expose starting area
}


void Count(int x, int y, int & unexposed, int & mines, int & needed, int & neighbors)
{
	unexposed = 0;
	mines = 0;
	needed = 0;
	neighbors = 0;
	if (x > 0 && y > 0)
	{
		if (exposed[y - 1][x - 1] == EXPOSE_MINE)
			mines++;
		else if (exposed[y - 1][x - 1] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (y > 0)
	{
		if (exposed[y - 1][x] == EXPOSE_MINE)
			mines++;
		else if (exposed[y - 1][x] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (x < width - 1 && y > 0)
	{
		if (exposed[y - 1][x + 1] == EXPOSE_MINE)
			mines++;
		else if (exposed[y - 1][x + 1] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (x > 0)
	{
		if (exposed[y][x - 1] == EXPOSE_MINE)
			mines++;
		else if (exposed[y][x - 1] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (x < width - 1)
	{
		if (exposed[y][x + 1] == EXPOSE_MINE)
			mines++;
		else if (exposed[y][x + 1] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (x > 0 && y < height - 1)
	{
		if (exposed[y + 1][x - 1] == EXPOSE_MINE)
			mines++;
		else if (exposed[y + 1][x - 1] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (y < height - 1)
	{
		if (exposed[y + 1][x] == EXPOSE_MINE)
			mines++;
		else if (exposed[y + 1][x] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (x < width - 1 && y < height - 1)
	{
		if (exposed[y + 1][x + 1] == EXPOSE_MINE)
			mines++;
		else if (exposed[y + 1][x + 1] < 0)
			unexposed++;
		else
			neighbors++;
	}
	if (exposed[y][x] > 0 && exposed[y][x] < EXPOSE_MINE)
		needed = exposed[y][x] - mines;
}


void AddMine(int x, int y)
{
	if (x < 0
	|| x >= width
	|| y < 0
	|| y >= height)
		return;
	if (exposed[y][x] < 0)
	{
		mineCount--;
		exposed[y][x] = EXPOSE_MINE;
		if (puzzle[y][x] == 0)				// mine should not be here
		{
			solving = false;
			gameOver = true;				// stop play when we do this
			MessageBeep(MB_ICONWARNING);	// sound end of solving
		}
	}
}

void ExposeTile(int x, int y)
{
	if (x < 0
	|| x >= width
	|| y < 0
	|| y >= height)
		return;
	Expose(x, y);							// expose this tile and neighbors
}


// Rule 3
//		XX?	or	--?	or	-nX  or Xn-	or	??? or	???	or	?XX	or	?--
//		nm?		nm?		-mX		Xm-		-mX		Xm-		?mn		?mn
//		--?		XX? 	???		???		-nX		Xn-		?--		?XX
// m is at [y][x]
// we need more then 2 unexposed tiles for m
// and more than one mine for m
// and we need only one more mine for n (hence not exposed or a mine)
// and the needed mines for m - 1 == unexposed tiles for m - 2
// and there are two unexposed common tiles between the two (n,m) as shown above
// then add mines in the unfilled spots next to n not common to the two (?)
//
bool Rule3(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (unexposed[y][x] <= 2									// we need more than 2 unexposed tiles for m
	||  needed[y][x] <= 1										// we need more than one mine for m
	||  needed[y][x] - 1 != unexposed[y][x] - 2)				// we require needed mines for m - 1 == unexposed tiles for m - 2
		return false;
	//		XX?	or	--?	or	-nX  or Xn-	
	//		nm?		nm?		-mX		Xm-		
	//		--?		XX? 	???		???	
	if (exposed[y + 1][x - 1] < 0 && exposed[y + 1][x] < 0		// case 1 above
	&& needed[y][x - 1] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x + 1, y - 1);
		AddMine(x + 1, y);
		AddMine(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (exposed[y - 1][x - 1] < 0 && exposed[y - 1][x] < 0		// case 2 above
	&& needed[y][x - 1] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x + 1, y - 1);
		AddMine(x + 1, y);
		AddMine(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (exposed[y - 1][x - 1] < 0 && exposed[y][x - 1] < 0		// case 3 above
	&& needed[y - 1][x] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x - 1, y + 1);
		AddMine(x, y + 1);
		AddMine(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (exposed[y - 1][x + 1] < 0 && exposed[y][x + 1] < 0		// case 4 above
	&& needed[y - 1][x] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x - 1, y + 1);
		AddMine(x, y + 1);
		AddMine(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	//		??? or	???	or	?XX	or	?--
	//		-mX		Xm-		?mn		?mn
	//		-nX		Xn-		?--		?XX
	if (exposed[y][x - 1] < 0 && exposed[y + 1][x - 1] < 0		// case 5 above
	&& needed[y + 1][x] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x - 1, y - 1);
		AddMine(x, y - 1);
		AddMine(x + 1, y - 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (exposed[y + 1][x + 1] < 0 && exposed[y][x + 1] < 0		// case 6 above
	&& needed[y + 1][x] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x - 1, y - 1);
		AddMine(x, y - 1);
		AddMine(x + 1, y - 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (exposed[y + 1][x] < 0 && exposed[y + 1][x + 1] < 0		// case 7 above
	&& needed[y][x + 1] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x - 1, y - 1);
		AddMine(x - 1, y);
		AddMine(x - 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (exposed[y - 1][x + 1] < 0 && exposed[y - 1][x] < 0		// case 8 above
	&& needed[y][x + 1] == 1)									// hence exposed and not a mine
	{
		SaveBackUp();											// save current board
		AddMine(x - 1, y - 1);
		AddMine(x - 1, y);
		AddMine(x - 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	return false;
}


// Rule 4
//		Xn- or	-nX	or	??? or	??? or	?XX or	?-- or	XX? or	--?
//		Xm-		-mX		-mX		Xm-		?mn		?mn		nm?		nm?
//		???		???		-nX		Xn-		?--		?XX		--?		XX?
// m is at [y][x]
// we need more then 2 unexposed tiles for m
// and only 2 unexposed tiles for n
// and there is only 1 needed mine for each n and m
// and there are two unexposed common tiles between the two as shown above
// then expose the unexposed neighbors of m shown by ?
//
bool Rule4(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (unexposed[y][x] <= 2									// we need more than 2 unexposed tiles for m
	||  needed[y][x] != 1)										// we need only one mine for m
		return false;
	//		Xn- or	-nX	or	??? or	??? 
	//		Xm-		-mX		-mX		Xm-	
	//		???		???		-nX		Xn-	
	if (needed[y - 1][x] == 1									// case 1 above
	&& unexposed[y - 1][x] == 2
	&& exposed[y - 1][x + 1] < 0 && exposed[y][x + 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x - 1, y + 1);
		ExposeTile(x, y + 1);
		ExposeTile(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (needed[y - 1][x] == 1									// case 2 above
	&& unexposed[y - 1][x] == 2
	&& exposed[y - 1][x - 1] < 0 && exposed[y][x - 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x - 1, y + 1);
		ExposeTile(x, y + 1);
		ExposeTile(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (needed[y + 1][x] == 1									// case 3 above
	&& unexposed[y + 1][x] == 2
	&& exposed[y + 1][x - 1] < 0 && exposed[y][x - 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x - 1, y - 1);
		ExposeTile(x, y - 1);
		ExposeTile(x + 1, y - 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (needed[y + 1][x] == 1									// case 4 above
	&& unexposed[y + 1][x] == 2
	&& exposed[y + 1][x + 1] < 0 && exposed[y][x + 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x - 1, y - 1);
		ExposeTile(x, y - 1);
		ExposeTile(x + 1, y - 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	//	?XX or	?-- or	XX? or	--?
	//	?mn		?mn		nm?		nm?
	//	?--		?XX		--?		XX?
	if (needed[y][x + 1] == 1									// case 5 above
	&& unexposed[y][x + 1] == 2
	&& exposed[y + 1][x] < 0 && exposed[y + 1][x + 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x - 1, y - 1);
		ExposeTile(x - 1, y);
		ExposeTile(x - 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (needed[y][x + 1] == 1									// case 6 above
	&& unexposed[y][x + 1] == 2
	&& exposed[y - 1][x] < 0 && exposed[y - 1][x + 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x - 1, y - 1);
		ExposeTile(x - 1, y);
		ExposeTile(x - 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (needed[y][x - 1] == 1									// case 7 above
	&& unexposed[y][x - 1] == 2
	&& exposed[y + 1][x] < 0 && exposed[y + 1][x - 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x + 1, y - 1);
		ExposeTile(x + 1, y);
		ExposeTile(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	if (needed[y][x - 1] == 1									// case 8 above
	&& unexposed[y][x - 1] == 2
	&& exposed[y - 1][x] < 0 && exposed[y - 1][x - 1] < 0)
	{
		SaveBackUp();											// save current board
		ExposeTile(x + 1, y - 1);
		ExposeTile(x + 1, y);
		ExposeTile(x + 1, y + 1);
		BackUpChanged();										// check to see if anything changed
		return true;
	}
	return false;
}


// Rule 5
//		X**? or	?**X or	??? or	XXX
//		Xnm?	?mnX	*m*		*n*
//		X**?	?**X	*n*		*m*
//						XXX		???
// m is at [y][x]
// * = exposed or not
// In the above situations n and m need the same number of mines
// and the number of unexposed for n is < number for m
// and by previous rules unexposed of n > mines needed by n
// then expose all ? unless they are already exposed
//
bool Rule5(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (needed[y][x - 1] == needed[y][x]
	&& unexposed[y][x - 1] < unexposed[y][x]
	&& (x == 1 ||  (exposed[y - 1][x - 2] >= 0 && exposed[y][x - 2] >= 0 && exposed[y + 1][x - 2] >= 0)))
	{
		SaveBackUp();										// save current board
		ExposeTile(x + 1, y - 1);
		ExposeTile(x + 1, y);
		ExposeTile(x + 1, y + 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (needed[y][x + 1] == needed[y][x]
	&& unexposed[y][x + 1] < unexposed[y][x]
	&& (x == width - 1 || (exposed[y - 1][x + 2] >= 0 && exposed[y][x + 2] >= 0 && exposed[y + 1][x + 2] >= 0)))
	{
		SaveBackUp();										// save current board
		ExposeTile(x - 1, y - 1);
		ExposeTile(x - 1, y);
		ExposeTile(x - 1, y + 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	//	??? or	XXX
	//	*m*		*n*
	//	*n*		*m*
	//	XXX		???
	if (needed[y + 1][x] == needed[y][x]
	&& unexposed[y + 1][x] < unexposed[y][x]
	&& (y == height - 1 || (exposed[y + 2][x - 1] >= 0 && exposed[y + 2][x] >= 0 && exposed[y + 2][x + 1] >= 0)))
	{
		SaveBackUp();										// save current board
		ExposeTile(x - 1, y - 1);
		ExposeTile(x, y - 1);
		ExposeTile(x + 1, y - 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (needed[y - 1][x] == needed[y][x]
	&& unexposed[y - 1][x] < unexposed[y][x]
	&& (y == 1 || (exposed[y - 2][x - 1] >= 0 && exposed[y - 2][x] >= 0 && exposed[y - 2][x + 1] >= 0)))
	{
		SaveBackUp();										// save current board
		ExposeTile(x - 1, y + 1);
		ExposeTile(x, y + 1);
		ExposeTile(x + 1, y + 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	return false;
}


// Rule 6 (extension of Rule 3)
//		X**? or	?**X or	??? or	XXX
//		Xnm?	?mnX	*m*		*n*
//		X**?	?**X	*n*		*m*
//						XXX		???
// m is at [y][x]
// * = exposed or not
// In the above situations needed[m] and needed[n] > 0 (i.e. not mines)
// the number of unexposed[n] is < unexposed[m]
// the number of mines needed[m] - needed[n] = unexposed[m] - unexposed[n] 
// then place mines for all ?
//
bool Rule6(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (needed[y][x] == 0)
		return false;
	if (unexposed[y][x - 1] < unexposed[y][x]
	&&  needed[y][x - 1] > 0
	&&  needed[y][x] - needed[y][x - 1] == unexposed[y][x] - unexposed[y][x - 1]
	&& (x == 1 || (exposed[y - 1][x - 2] >= 0 && exposed[y][x - 2] >= 0 && exposed[y + 1][x - 2] >= 0)))
	{
		SaveBackUp();										// save current board
		AddMine(x + 1, y - 1);
		AddMine(x + 1, y);
		AddMine(x + 1, y + 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (unexposed[y][x + 1] < unexposed[y][x]
	&& needed[y][x + 1] > 0
	&& needed[y][x] - needed[y][x + 1] == unexposed[y][x] - unexposed[y][x + 1]
	&& (x == width - 1 || (exposed[y - 1][x + 2] >= 0 && exposed[y][x + 2] >= 0 && exposed[y + 1][x + 2] >= 0)))
	{
		SaveBackUp();										// save current board
		AddMine(x - 1, y - 1);
		AddMine(x - 1, y);
		AddMine(x - 1, y + 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	//	??? or	XXX
	//	*m*		*n*
	//	*n*		*m*
	//	XXX		???
	if (unexposed[y + 1][x] < unexposed[y][x]
	&& needed[y + 1][x] > 0
	&& needed[y][x] - needed[y + 1][x] == unexposed[y][x] - unexposed[y + 1][x]
	&& (y == height - 1 || (exposed[y + 2][x - 1] >= 0 && exposed[y + 2][x] >= 0 && exposed[y + 2][x + 1] >= 0)))
	{
		SaveBackUp();										// save current board
		AddMine(x - 1, y - 1);
		AddMine(x, y - 1);
		AddMine(x + 1, y - 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (needed[y - 1][x] < needed[y][x]
	&& needed[y - 1][x] > 0
	&& needed[y][x] - needed[y - 1][x] == unexposed[y][x] - unexposed[y - 1][x]
	&& (y == 1 || (exposed[y - 2][x - 1] >= 0 && exposed[y - 2][x] >= 0 && exposed[y - 2][x + 1] >= 0)))
	{
		SaveBackUp();										// save current board
		AddMine(x - 1, y + 1);
		AddMine(x, y + 1);
		AddMine(x + 1, y + 1);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	return false;
}


// Rule 7
//		Xa- or	-aX or	--? or	?--
//		bm-		-mb		bm-		-mb
//		--?		?--		Xa-		-aX
// In the above situation unexposed[m] = 5
// and needed[a] = needed[b] = 1
// and unexposed[a] and unexposed[b] >= 2
// and X is exposed
// and ? is unexposed
// and if the needed[m]= 3 the put a mine at ?
// else if unexposed[a] and unexposed[b] = 2 then expose[m]
//
bool Rule7(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (unexposed[y][x] != 5
	|| needed[y][x] < 2
	|| needed[y][x] > 3)
		return false;
	if (needed[y][x - 1] == 1
	&& needed[y - 1][x] == 1
	&& unexposed[y][x - 1] >= 2
	&& unexposed[y - 1][x] >= 2
	&& exposed[y + 1][x + 1] < 0)
	{
		if (needed[y][x] == 3)								// add mine
		{
			SaveBackUp();									// save current board
			AddMine(x + 1, y + 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
		if (unexposed[y][x - 1] == 2
		&&  unexposed[y - 1][x] == 2)
		{
			SaveBackUp();									// save current board
			ExposeTile(x + 1, y + 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
	}
	if (needed[y][x + 1] == 1
	&& needed[y - 1][x] == 1
	&& unexposed[y][x + 1] >= 2
	&& unexposed[y - 1][x] >= 2
	&& exposed[y + 1][x - 1] < 0)
	{
		if (needed[y][x] == 3)								// add mine
		{
			SaveBackUp();									// save current board
			AddMine(x - 1, y + 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
		if (unexposed[y][x + 1] == 2
		&&  unexposed[y - 1][x] == 2)
		{
			SaveBackUp();									// save current board
			ExposeTile(x - 1, y + 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
	}
	//	--? or	?--
	//	bm-		-mb
	//	Xa-		-aX
	if (needed[y][x - 1] == 1
	&& needed[y + 1][x] == 1
	&& unexposed[y][x - 1] >= 2
	&& unexposed[y + 1][x] >= 2
	&& exposed[y - 1][x + 1] < 0)
	{
		if (needed[y][x] == 3)								// add mine
		{
			SaveBackUp();									// save current board
			AddMine(x + 1, y - 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
		if (unexposed[y][x - 1] == 2
		&&  unexposed[y + 1][x] == 2)
		{
			SaveBackUp();									// save current board
			ExposeTile(x + 1, y - 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
	}
	if (needed[y][x + 1] == 1
	&& needed[y + 1][x] == 1
	&& unexposed[y][x + 1] >= 2
	&& unexposed[y + 1][x] >= 2
	&& exposed[y - 1][x - 1] < 0)
	{
		if (needed[y][x] == 3)								// add mine
		{
			SaveBackUp();									// save current board
			AddMine(x - 1, y - 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
		if (unexposed[y][x + 1] == 2
		&&  unexposed[y + 1][x] == 2)
		{
			SaveBackUp();									// save current board
			ExposeTile(x - 1, y - 1);
			BackUpChanged();								// check to see if anything changed
			return true;
		}
	}
	return false;
}


//	New Rule 8 (extension of Rule 5)
//		XX*?? or	??*XX or	???			XXX
//		Xm*n?		?n*mX		?n?			XmX	
//		XX*??		??*XX		***			***
//								XmX			?n?
//								XXX			???
//	where needed[m] <= 2 
//  and needed[m] = needed[n] 
//	and unexposed[m] < unexposed[n]
//	X is exposed and * may not be exposed
//	so expose all ?
//		
bool Rule8(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (needed[y][x] > 2)
		return false;
	if (x <= width - 3
	&& needed[y][x + 2] == needed[y][x]
	&& unexposed[y][x + 2] > unexposed[y][x]
	&& exposed[y - 1][x - 1] >= 0
	&& exposed[y][x - 1] >= 0
	&& exposed[y + 1][x - 1] >= 0
	&& exposed[y - 1][x] >= 0
	&& exposed[y + 1][x] >= 0)
	{
		SaveBackUp();										// save current board
		ExposeTile(x + 2, y - 1);
		ExposeTile(x + 2, y + 1);
		if (x < width - 3)
		{
			ExposeTile(x + 3, y - 1);
			ExposeTile(x + 3, y);
			ExposeTile(x + 3, y + 1);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (x >= 2
	&& needed[y][x - 2] == needed[y][x]
	&& unexposed[y][x - 2] > unexposed[y][x]
	&& exposed[y - 1][x + 1] >= 0
	&& exposed[y][x + 1] >= 0
	&& exposed[y + 1][x + 1] >= 0
	&& exposed[y - 1][x] >= 0
	&& exposed[y + 1][x] >= 0)
	{
		SaveBackUp();										// save current board
		ExposeTile(x - 2, y - 1);
		ExposeTile(x - 2, y + 1);
		if (x >= 3)
		{
			ExposeTile(x - 3, y - 1);
			ExposeTile(x - 3, y);
			ExposeTile(x - 3, y + 1);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	//  or	???			XXX
	//		?n?			XmX
	//		***			***
	//		XmX			?n?
	//		XXX			???
	if (y >= 2
	&& needed[y - 2][x] == needed[y][x]
	&& unexposed[y - 2][x] > unexposed[y][x]
	&& exposed[y + 1][x - 1] >= 0
	&& exposed[y + 1][x] >= 0
	&& exposed[y + 1][x + 1] >= 0
	&& exposed[y][x - 1] >= 0
	&& exposed[y][x + 1] >= 0)
	{
		SaveBackUp();										// save current board
		ExposeTile(x - 1, y - 2);
		ExposeTile(x + 1, y - 2);
		if (y >= 3)
		{
			ExposeTile(x - 1, y - 3);
			ExposeTile(x, y - 3);
			ExposeTile(x + 1, y - 3);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (y <= height - 3
	&& needed[y + 2][x] == needed[y][x]
	&& unexposed[y + 2][x] > unexposed[y][x]
	&& exposed[y - 1][x - 1] >= 0
	&& exposed[y - 1][x] >= 0
	&& exposed[y - 1][x + 1] >= 0
	&& exposed[y][x - 1] >= 0
	&& exposed[y][x + 1] >= 0)
	{
		SaveBackUp();										// save current board
		ExposeTile(x - 1, y + 2);
		ExposeTile(x + 1, y + 2);
		if (y <= height - 4)
		{
			ExposeTile(x - 1, y + 3);
			ExposeTile(x, y + 3);
			ExposeTile(x + 1, y + 3);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	return false;
}


//	New Rule 9 (Similar to rule 8)
//		XX*?? or	??*XX or	???			XXX
//		Xm*n?		?n*mX		?n?			XmX	
//		XX*??		??*XX		***			***
//								XmX			?n?
//								XXX			???
//	where needed[m] <= 2 
//	and unexposed[m] >= 2 
//  and needed[n] > 0
//	and unexposed[m] > unexposed[n]
//  and needed[n] - needed[m] = unexposed[n] - unexposed[m]
//	X is exposed and * may not be exposed
//	so place mines at all ?
//
bool Rule9(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	if (needed[y][x] > 2
	||  needed[y][x] == 0
	||  unexposed[y][x] < 2)
		return false;
	if (x <= width - 3
	&& needed[y][x + 2] > 0
	&& unexposed[y][x + 2] > unexposed[y][x]
	&& needed[y][x + 2] - needed[y][x] == unexposed[y][x + 2] - unexposed[y][x]
	&& exposed[y - 1][x - 1] >= 0
	&& exposed[y][x - 1] >= 0
	&& exposed[y + 1][x - 1] >= 0
	&& exposed[y - 1][x] >= 0
	&& exposed[y + 1][x] >= 0)
	{
		SaveBackUp();										// save current board
		AddMine(x + 2, y - 1);
		AddMine(x + 2, y + 1);
		if (x < width - 3)
		{
			AddMine(x + 3, y - 1);
			AddMine(x + 3, y);
			AddMine(x + 3, y + 1);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (x >= 2
	&& needed[y][x - 2] > 0
	&& unexposed[y][x - 2] > unexposed[y][x]
	&& needed[y][x - 2] - needed[y][x] == unexposed[y][x - 2] - unexposed[y][x]
	&& exposed[y - 1][x + 1] >= 0
	&& exposed[y][x + 1] >= 0
	&& exposed[y + 1][x + 1] >= 0
	&& exposed[y - 1][x] >= 0
	&& exposed[y + 1][x] >= 0)
	{
		SaveBackUp();										// save current board
		AddMine(x - 2, y - 1);
		AddMine(x - 2, y + 1);
		if (x >= 3)
		{
			AddMine(x - 3, y - 1);
			AddMine(x - 3, y);
			AddMine(x - 3, y + 1);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	//  or	???			XXX
	//		?n?			XmX
	//		***			***
	//		XmX			?n?
	//		XXX			???
	if (y >= 2
	&& needed[y - 2][x] > 0
	&& unexposed[y - 2][x] > unexposed[y][x]
	&& needed[y - 2][x] - needed[y][x]== unexposed[y - 2][x] - unexposed[y][x] 
	&& exposed[y + 1][x - 1] >= 0
	&& exposed[y + 1][x] >= 0
	&& exposed[y + 1][x + 1] >= 0
	&& exposed[y][x - 1] >= 0
	&& exposed[y][x + 1] >= 0)
	{
		SaveBackUp();										// save current board
		AddMine(x - 1, y - 2);
		AddMine(x + 1, y - 2);
		if (y >= 3)
		{
			AddMine(x - 1, y - 3);
			AddMine(x, y - 3);
			AddMine(x + 1, y - 3);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	if (y <= height - 3
	&& needed[y + 2][x] > 0
	&& unexposed[y + 2][x] > unexposed[y][x]
	&& needed[y + 2][x] - needed[y][x] == unexposed[y + 2][x] - unexposed[y][x]
	&& exposed[y - 1][x - 1] >= 0
	&& exposed[y - 1][x] >= 0
	&& exposed[y - 1][x + 1] >= 0
	&& exposed[y][x - 1] >= 0
	&& exposed[y][x + 1] >= 0)
	{
		SaveBackUp();										// save current board
		AddMine(x - 1, y + 2);
		AddMine(x + 1, y + 2);
		if (y <= height - 4)
		{
			AddMine(x - 1, y + 3);
			AddMine(x, y + 3);
			AddMine(x + 1, y + 3);
		}
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	return false;
}


//	New Rule 10
//		?XX
//		-mXX
//		-npX
//		****
// In the above situation unexposed[m] = 3
// and needed[m] = 1
// and needed[n] > 1
// and X is exposed
// and n cannot fill its mines with *** using p
// then expose ?
//
bool Rule10(int x, int y, int unexposed[MAX_HEIGHT][MAX_WIDTH], int mines[MAX_HEIGHT][MAX_WIDTH], int needed[MAX_HEIGHT][MAX_WIDTH])
{
	return false;
}


bool MakeMove()
{
	// Rule 1 and 2 applied here
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Count(x, y, unexposed[y][x], mines[y][x], needed[y][x], neighbors[y][x]);
			// Rule 1 
			// find exposed square with N unexposed neighbors
			// and N remaining mines around this square
			// In this case add mines to all unexposed neighbors
			if (rules[RULE_1])
			{
				if (exposed[y][x] > 0
				&& unexposed[y][x] != 0
				&& exposed[y][x] == unexposed[y][x] + mines[y][x])
				{
					lastRuleUsed = RULE_1;
					rulesCount[0]++;
					SaveBackUp();								// save current board
					AddMine(x - 1, y - 1);
					AddMine(x, y - 1);
					AddMine(x + 1, y - 1);
					AddMine(x - 1, y);
					AddMine(x + 1, y);
					AddMine(x - 1, y + 1);
					AddMine(x, y + 1);
					AddMine(x + 1, y + 1);
					BackUpChanged();							// check to see if anything changed
					return true;
				}
			}
			// Rule 2
			// If an exposed square already has all the mines marked
			// fill all unexposed squares around it
			if (rules[RULE_2])
			{
				if (exposed[y][x] > 0
				&& unexposed[y][x] != 0
				&& exposed[y][x] == mines[y][x])
				{
					lastRuleUsed = RULE_2;
					rulesCount[1]++;
					SaveBackUp();								// save current board
					ExposeTile(x - 1, y - 1);
					ExposeTile(x, y - 1);
					ExposeTile(x + 1, y - 1);
					ExposeTile(x - 1, y);
					ExposeTile(x + 1, y);
					ExposeTile(x - 1, y + 1);
					ExposeTile(x, y + 1);
					ExposeTile(x + 1, y + 1);
					BackUpChanged();							// check to see if anything changed
					return true;
				}
			}
		}
	}

	// handle harder rules
	//
	//	where x,y > 0 and < max-1
	//
	for (int y = 1; y < height - 1; y++)
	{
		for (int x = 1; x < width - 1; x++)
		{
			if (needed[y][x] == 0)								// hence we are exposed and not a mine 
				continue;										// and needing some mines < unexposed 
			if (rules[RULE_3]
			&& Rule3(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_3;
				rulesCount[2]++;
				return true;
			}
			if (rules[RULE_4]
			&& Rule4(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_4;
				rulesCount[3]++;
				return true;
			}
			if (rules[RULE_5]
			&& Rule5(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_5;
				rulesCount[4]++;
				return true;
			}
			if (rules[RULE_6]
			&& Rule6(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_6;
				rulesCount[5]++;
				return true;
			}
			if (rules[RULE_7]
			&& Rule7(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_7;
				rulesCount[6]++;
				return true;
			}
			if (rules[RULE_8]
			&& Rule8(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_8;
				rulesCount[7]++;
				return true;
			}
			if (rules[RULE_9]
			&& Rule9(x, y, unexposed, mines, needed))
			{
				lastRuleUsed = RULE_9;
				rulesCount[8]++;
				return true;
			}
		}
	}

	// Run tank method 
	endGameCount = endPlayCount;
	CreateTankLists();										// create the lists
	TimeStart();
	if (CreateTankSolutions() == false)						// find all possible solutions on the lists	
	{
#if 0
		char	checkName[MAX_PATH];
		sprintf(checkName, ".\\puzzle%d.txt", testing);
		SavePuzzle(checkName);
#endif
	}
	double time = TimeEnd();
	if (time > 1.0)
	{
		if (testResults != NULL)
			fprintf(testResults, "\n***** WARNING: Puzzle=%d CreateTankSolutions time=%.2f)\n", testing, time);
	}
	CalculateProb();										// calculate proability for each entry in lists
	if (rules[RULE_10])
	{
		if (FindEasyTankMove())								// find obvious move if any
		{
			lastRuleUsed = RULE_10;
			rulesCount[9]++;
			return true;
		}
	}

	// Guess 1 - look at randomly picking a place if the mine/total ratio is lower
	// in the hopes of exposing a large area
	if (rules[GUESS_1])
	{
		if (PickOpenSpot(!rules[GUESS_2]))
		{
			lastRuleUsed = GUESS_1;
			rulesCount[10]++;
			return true;
		}
	}

	// guess 2 using tank rules
	if (rules[GUESS_2])
	{
		if (PickBestSpot())
		{
			lastRuleUsed = GUESS_2;
			rulesCount[11]++;
			return true;
		}
	}

	Beep(500, 80);
	solving = false;
	return false;
}