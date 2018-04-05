#include "stdafx.h"


int		winTotal;								// total number of wins
int		winAllTotal;							// wins across all seeds
int		guess1Failed;
int		guess2Failed;
int		winsWithGuesses[MAX_WINS_WITH_GUESSES];
double	winProb;								// sum of prob for all wins
double	lossProb;
double	guess1Prob;								// sum of all guess1 prob 
double	guess2Prob;								// sum of all guess2 prob 
int		guessesPerWin;							// number of guesses per win
int		guessesPerLoss;
int		totalWins;								// number of wins and tests
int		totalTests;
int		totalGuess1;							// total guess1 and guess2 used
int		totalGuess2;
int		totalGuess1Failed;						// number of failed guesses
int		totalGuess2Failed;
int		totalGuessesPerWin;						// number of guesses per win
int		totalGuessesPerLoss;
int		totalWinsWithGuesses[MAX_WINS_WITH_GUESSES];
double	totalWinProb;							// sum of prob for all wins
double	totalLossProb;
double	totalGuess1Prob;						// sum of all guess1 prob 
double	totalGuess2Prob;						// sum of all guess2 prob 

static char * RuleText[MAX_RULES] = 
{
	"***** Rule 1 *****",
	"***** Rule 2 *****",
	"***** Rule 3 *****",
	"***** Rule 4 *****",
	"***** Rule 5 *****",
	"***** Rule 6 *****",
	"***** Rule 7 *****",
	"***** Rule 8 *****",
	"***** Rule 9 *****",
	"***** Rule 10 *****",
	"Guess 1",
	"Guess 2",
};


void ClearTotalStats()
{
	totalWins = 0;
	totalTests = 0;
	totalGuess1 = 0;
	totalGuess2 = 0;
	totalGuess1Failed = 0;
	totalGuess2Failed = 0;
	totalGuessesPerWin = 0;
	totalGuessesPerLoss = 0;
	totalWinProb = 0.0;
	totalLossProb = 0.0;
	totalGuess1Prob = 0.0;
	totalGuess2Prob = 0.0;
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
		totalWinsWithGuesses[i] = 0;
}


void SaveTotalResults(FILE * file)
{
	if (totalTests == 0)
		return;
	if (file != testAllResults)
	{
		fprintf(file, "start=%d,%d endPlay=%d guess1Offset=%.2f\n", startX, startY, endPlayCount, guess1Offset);
		fprintf(file, "Samples=%d\nWon=%d\n", totalTests, totalWins);
		double win = (double)totalWins / totalTests;
		fprintf(file, "Percent=%.2f%%\n", 100.0 * win);
		double sum = 0;
		for (int i = 0; i < MAX_SEEDS; i++)
		{
			double x = ((double)wins[i] / TEST_SAMPLES - win);
			sum += x * x;
		}
		double deviation = sqrt(sum) / MAX_SEEDS;
		fprintf(file, "Deviation: %.2f%%\n", 100.0 * deviation);
	}
	fprintf(file, "Guess1 Used: %d\n", totalGuess1);
	fprintf(file, "Guess2 Used: %d\n", totalGuess2);
	if (totalGuess1 != 0)
		fprintf(file, "Guess1 Failed: %.1f%%\n", (100.0 * totalGuess1Failed) / totalGuess1);
	if (totalGuess2 != 0)
		fprintf(file, "Guess2 Failed: %.1f%%\n", (100.0 * totalGuess2Failed) / totalGuess2);
	if (totalWins != 0)
		fprintf(file, "Average guess per win: %.2f\n", (double)totalGuessesPerWin / totalWins);
	if ((totalTests - totalWins) != 0)
		fprintf(file, "Average guess per loss: %.2f\n", (double)totalGuessesPerLoss / (totalTests - totalWins));
	if (totalWins != 0)
		fprintf(file, "Average win prob: %.2f\n", 100.0 * totalWinProb / totalWins);
	if ((totalTests - totalWins) > 0)
		fprintf(file, "Average loss prob: %.2f\n", 100.0 * totalLossProb / (totalTests - totalWins));
	if (totalGuess1 != 0)
		fprintf(file, "Average guess1 prob: %.2f\n", (100.0 * totalGuess1Prob) / totalGuess1);
	if (totalGuess2 != 0)
		fprintf(file, "Average guess2 prob: %.2f\n", (100.0 * totalGuess2Prob) / totalGuess2);
	fprintf(file, "Wins with guesses:\n     0     1     2     3     4     5     6     7     8+\n");
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
	{
		fprintf(file, "%6d", totalWinsWithGuesses[i]);
	}
	fprintf(file, "\n");
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
	{
		fprintf(file, " %4.1f%%", 100.0 * totalWinsWithGuesses[i] / totalTests);
	}
	fprintf(file, "\n");
	double time = TimeEndOverall();
	fprintf(file, "\ntime=%.3f seconds\n", time);
}


void SaveResults(FILE * file)
{
	if (gameOver)
	{
		if (remainingCount == 0)
		{
			int guesses = rulesCount[GUESS_1] + rulesCount[GUESS_2];
			if (guesses >= MAX_WINS_WITH_GUESSES)
				guesses = MAX_WINS_WITH_GUESSES - 1;
			winsWithGuesses[guesses]++;
			winProb += prob;
			winTotal++;
			guessesPerWin += guesses;
			fprintf(file, "Puzzle%d prob=%3d%% Win!\n", testing, (int)(100 * prob));
		}
		else
		{
			lossProb += prob;
			guessesPerLoss += guesses;
			if (lastRuleUsed == GUESS_1)
				guess1Failed++;
			else if (lastRuleUsed == GUESS_2)
				guess2Failed++;
			fprintf(file, "Puzzle%d prob=%3d%% %s Failed\n", testing, (int)(100 * prob), RuleText[lastRuleUsed]);
		}
	}
	else 
		fprintf(file, "Puzzle%d prob=%3d%% ***** Missing Rule *****\n", testing, (int)(100 * prob));
	fprintf(file, "remaining=%d mines=%d\n", remainingCount, mineCount);
	fprintf(file, "Rules   1      2      3      4      5      6      7      8      9     10 Guess1 Guess2\n  ");
	for (int i = 0; i < MAX_RULES; i++)
	{
		fprintf(file, "%7d", rulesCount[i]);
	}
	fprintf(file, "\n\n");
}


void SaveAllResults(FILE * file, U32 seed, int index, int count, bool done, int total)
{
	if (file == NULL)
		return;
	winAllTotal += winTotal;
	if (index >= 0)
	{
		fprintf(file, "%-11d %d\n", seed, winTotal);
		index++;
		if (done)
		{
			double win = (double)winAllTotal / total;
			fprintf(file, "---------------\nSamples: %d\nWins: %d\nPercent: %.2f%%\n", 100 * index, winAllTotal, 100.0 * win);
			double sum = 0;
			for (int i = 0; i < index; i++)
			{
				double x = ((double)wins[i] / 100 - win);
				sum += x * x;
			}
			double deviation = sqrt(sum) / index;
			fprintf(file, "Deviation: %.2f%%\n", 100.0 * deviation);
		}
	}
	else
	{
		fprintf(file, "%d\t", winTotal);
		if (done)
		{
			double win = (double)winAllTotal / total;
			fprintf(file, "%d\t%3.1f%%\t", winAllTotal, 100.0 * win);
			double sum = 0;
			for (int i = 0; i < MAX_SEEDS; i++)
			{
				double x = ((double)wins[i] / TEST_SAMPLES - win);
				sum += x * x;
			}
			double deviation = sqrt(sum) / MAX_SEEDS;
			fprintf(file, "%.2f%%\n", 100.0 * deviation);
		}
	}
}


void ShowTotalResults(FILE * file, int tests, int seed)
{
	int total = 0, totals[MAX_RULES];
	for (int i = 0; i < MAX_RULES; i++)
		totals[i] = 0;
	fprintf(file, "starting at (%d,%d)\n", startX, startY);
	if (guess1Offset != 0.0)
		fprintf(file, "guess1Offset=%3.1f%%\n", 100 * guess1Offset);
	fprintf(file, "endPlayCount=%d\n", endPlayCount);
	if (seed != 0)
		fprintf(file, "seed=%d\n", seed);

	fprintf(file, "Rules   1      2      3      4      5      6      7      8      9     10 Guess1 Guess2\n  ");
	for (int i = 0; i < MAX_RULES; i++)
	{
		totals[i] += rulesCountTotal[i];
		total += totals[i];
		fprintf(file, "%7d", rulesCountTotal[i]);
	}
	fprintf(file, "\n   ");
	for (int i = 0; i < MAX_RULES; i++)
	{
		fprintf(file, "%5.1f%% ", 100.0 * rulesCountTotal[i] / total);
	}
	fprintf(file, "\n\n");

	fprintf(file, "Samples=%d\nWins=%d\nPercent=%.1f%%\n", tests, winTotal, 100.0 * winTotal / tests);

	if (rulesCountTotal[GUESS_1] != 0)
		fprintf(file, "Guess 1 failed=%d%%\n", (100 * guess1Failed) / rulesCountTotal[GUESS_1]);
	if (rulesCountTotal[GUESS_2] != 0)
		fprintf(file, "Guess 2 failed=%d%%\n", (100 * guess2Failed) / rulesCountTotal[GUESS_2]);
	if (winTotal > 0)
		fprintf(file, "Average guesses per win=%4.2f\n", (double)guessesPerWin / winTotal);
	if (tests - winTotal > 0)
		fprintf(file, "Average guesses per loss=%4.2f\n", (double)guessesPerLoss / (tests - winTotal));
	if (winTotal > 0)
		fprintf(file, "Average Win Prob=%5.1f%%\n", 100.0 * winProb / winTotal);
	if (tests - winTotal > 0)
		fprintf(file, "Average Loss Prob=%5.1f%%\n", 100.0 * lossProb / (tests - winTotal));
	if (rulesCountTotal[GUESS_1] != 0)
		fprintf(file, "Average Guess 1 Prob=%5.1f%%\n", (100 * guess1Prob) / rulesCountTotal[GUESS_1]);
	if (rulesCountTotal[GUESS_2] != 0)
		fprintf(file, "Average Guess 2 Prob=%5.1f%%\n", (100 * guess2Prob) / rulesCountTotal[GUESS_2]);
	fprintf(file, "Wins with guesses\n     0     1     2     3     4     5     6     7     8+\n");
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
	{
		fprintf(file, "%6d", winsWithGuesses[i]);
	}
	fprintf(file, "\n");
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
	{
		fprintf(file, " %4.1f%%", 100.0 * winsWithGuesses[i] / tests);
	}
	fprintf(file, "\n\n");

	totalWins += winTotal;
	totalTests += tests;
	totalGuess1 += rulesCountTotal[GUESS_1];
	totalGuess2 += rulesCountTotal[GUESS_2];
	totalGuess1Failed += guess1Failed;
	totalGuess2Failed += guess2Failed;
	totalGuessesPerWin += guessesPerWin;
	totalGuessesPerLoss += guessesPerLoss;
	totalWinProb += winProb;
	totalLossProb += lossProb;
	totalGuess1Prob += guess1Prob;
	totalGuess2Prob += guess2Prob;
	for (int i = 0; i < MAX_WINS_WITH_GUESSES; i++)
		totalWinsWithGuesses[i] += winsWithGuesses[i];
}


bool LoadPuzzle(char * fname)
{
	FILE * file;
	int x, y;

	file = fopen(fname, "r");
	if (file != NULL)
	{
		char string[3*MAX_WIDTH + 3];
		fgets(string, 3 * MAX_WIDTH + 2, file);
		int len = (int)strlen(string);
		if (len <= 9*2+1)
		{
			width = 9;
			height = 9;
			minesStart = 10;
		}
		else if (len <= 16*2+1)
		{
			width = 16;
			height = MAX_HEIGHT;
			minesStart = 40;
		}
		else
		{
			width = MAX_WIDTH;
			height = MAX_HEIGHT;
			minesStart = 99;
		}
		fseek(file, 0, SEEK_SET);						// restart at the beginning of the file
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				if (fscanf(file, "%d,", &puzzle[y][x]) == EOF)
				{
					fclose(file);
					return false;
				}
			}
		}
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				if (fscanf(file, "%d,", &exposed[y][x]) == EOF)
				{
					fclose(file);
					return false;
				}
			}
		}
		if (fscanf(file, "%lf", &prob) == EOF)			// if prob does not exist it is an old file 
			prob = 1.0;
		fclose(file);
		mineCount = minesStart;
		remainingCount = width * height - minesStart;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (exposed[y][x] == EXPOSE_MINE)
					mineCount--;
				else if (exposed[y][x] >= 0)
					remainingCount--;
			}
		}
		if (startX >= width							// reset starting position if now invalid
		|| startY >= height)
		{
			startX = START_X;
			startY = START_Y;
		}
		started = true;
		backup = false;
		solving = false;
		gameOver = false;
		for (int i = 0; i < MAX_RULES; i++)
			rulesCount[i] = 0;
		return true;
	}
	return false;
}


//
//
//
bool SavePuzzle(char * fname)
{
	FILE * file;

	file = fopen(fname, "w");
	if (file != NULL)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				fprintf(file, "%d,", puzzle[y][x]);
			}
			fprintf(file, "\n");
		}
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				fprintf(file, "%2d,", exposed[y][x]);
			}
			fprintf(file, "\n");
		}
		fprintf(file, "%f\n", prob);
		fclose(file);
		return true;
	}
	return false;
}


bool LoadGameFile()
{
	PWSTR	ppszPath;
	char	path[256];

	// get path to the system app data
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath);
	for (int i = 0; i < 256; i++)
	{
		path[i] = (char)ppszPath[i];
		if (path[i] == 0)
			break;
	}
	// this should be C:\Users\Ed Logg's\AppData\Roaming\MineSweeper\MineSweeper.txt
	strncat(path, "\\MineSweeper\\MineSweeper.txt", MAX_PATH-1);
	if (LoadPuzzle(path))
	{
//		DeleteFile(path);							// do not delete file in case game crashes
		started = true;
		return true;
	}
	return false;
}


bool SaveGameFile()
{
	PWSTR	ppszPath;
	char	path[256];
	// get path to the system app data
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &ppszPath);
	for (int i = 0; i < 256; i++)
	{
		path[i] = (char)ppszPath[i];
		if (path[i] == 0)
			break;
	}
	// create directory if it does not exist
	strncat(path, "\\MineSweeper", MAX_PATH-1);
	CreateDirectory(path, NULL);
	strncat(path, "\\MineSweeper.txt", MAX_PATH-1);
	return SavePuzzle(path);
}


//	Windows does not permit the following characters in file names:
//	<, >, :, ", /, |, \, ?, *
//
void ValidateFileName(char * fname)
{
	while (*fname != 0)
	{
		if (*fname < ' ')
			*fname = ' ';
		else if (*fname > '~')
			*fname = ' ';
		else if (*fname == '<')
			*fname = ' ';
		else if (*fname == '>')
			*fname = ' ';
		else if (*fname == ':')
			*fname = ' ';
		else if (*fname == '"')
			*fname = ' ';
		else if (*fname == '/')
			*fname = ' ';
		else if (*fname == '|')
			*fname = ' ';
		else if (*fname == '\\')
			*fname = ' ';
		else if (*fname == '?')
			*fname = ' ';
		else if (*fname == '*')
			*fname = ' ';
		++fname;
	}
}


bool SaveFileName(HWND hwnd, char * fname, char * filterString, char * defaultExt)
{
	OPENFILENAME ofn;

	ZeroMemory(&ofn, sizeof(ofn));
	ValidateFileName(fname);							// insure the file is valid

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filterString;
	ofn.lpstrFile = fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = defaultExt;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn))
		return true;
	return false;
}


bool LoadFileName(HWND hwnd, char * fname, char * filterString, char * defaultExt)
{
	OPENFILENAME ofn;
	fname[0] = 0;

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filterString;
	ofn.lpstrFile = fname;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = defaultExt;
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (GetOpenFileName(&ofn))
		return true;
	return false;
}
