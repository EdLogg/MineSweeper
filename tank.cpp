#include "stdafx.h"


// See https://luckytoilet.wordpress.com/2012/12/23/2125/
// which describes the "tank" algorithm
// first we must create independent lists of tiles that are connected
// then we can run all possible combinations for each list
// to get a count of a mine for each entry
// If we have an entry with 0% probability we can expose this entry
// If we have an entry with 100% probability we can add a mine there
// Otherwise we can pick the lowest proabability for our choice
// For the end game include all entries in one list 
// to find entries with no possible mine due to the mine quota

struct Neighbor
{
	int		x, y;							// position
	int		needed;							// number of mines needed
	int		unexposed;						// number of unexposed tiles around
};

struct Free
{
	int		x, y;							// position
	int		exposedCount;					// count for exposed entries for our listArray neighbors
	int		unexposed;						// number of unexposed tiles around
	int		listEntries;					// number of neighbors in listArray
};

struct List 
{
	int		x, y;							// position
	int		count;							// number of possible choices with mines
	int		tried;							// < 0 if not tried yet, 0 if exposed, 1 if mine
	int		num;							// number of neighbors below
	double	prob;							// probability of mine here (count / number of solutions)
	Neighbor * neighbors[8];				// pointer into neighbor array
};

int		neighborNext;						// next neighbor entry to use
int		neighborStart[MAX_LISTS];			// starting neighbor for this list
int		neighborEnd[MAX_LISTS];				// last neighbor for this list
Neighbor neighborArray[MAX_HEIGHT * MAX_WIDTH];		// list of all neighbors
int		freeNext;							// number of entries in freeArray
Free	freeArray[MAX_HEIGHT * MAX_WIDTH];	// number of non-neighbors we can play in with guess 1 rule
List	listArray[MAX_HEIGHT * MAX_WIDTH];	// list entries
int		listUsed[MAX_HEIGHT][MAX_WIDTH];	// which list is this entry part of (-1 if none)
int		listNext;							// next entry to use
int		lists;								// number of lists found
int		minesUsed;							// current count of mines used
int		listStart[MAX_LISTS];				// starting index for a list 
int		listEnd[MAX_LISTS];					// last entry for a list
int		listSolutions[MAX_LISTS];			// number of possible choices for each list
int		listMinMines[MAX_LISTS];			// min number of mines used
int		listMaxMines[MAX_LISTS];			// max number of mines used


void CreateList(int x, int y);
void CreateSolutions(int index, int maxCount);


void AddNeighbor(List * list, int x, int y)
{
	Neighbor * next = &neighborArray[lists];
	for (; next < &neighborArray[neighborNext]; next++)
	{
		if (next->x == x && next->y == y)
			goto FOUND;									// already on the list
	}
	neighborNext++;
	next->x = x;
	next->y = y;
	next->unexposed = unexposed[y][x];
	next->needed = needed[y][x];
	listUsed[y][x] = lists;								// mark this exposed entry on this list
FOUND:
	list->neighbors[list->num] = next;
	list->num++;
}


void CheckNeighbors(int x, int y)
{
	if (x > 0 && y > 0
	&& exposed[y - 1][x - 1] < 0							// unexposed
	&& listUsed[y - 1][x - 1] < 0)							// not on some list
		CreateList(x - 1, y - 1);
	if (y > 0
	&& exposed[y - 1][x] < 0								// unexposed
	&& listUsed[y - 1][x] < 0)								// not on some list
		CreateList(x, y - 1);
	if (x < width - 1 && y > 0
	&& exposed[y - 1][x + 1] < 0							// unexposed
	&& listUsed[y - 1][x + 1] < 0)							// not on some list
		CreateList(x + 1, y - 1);
	if (x > 0
	&& exposed[y][x - 1] < 0								// unexposed
	&& listUsed[y][x - 1] < 0)								// not on some list
		CreateList( x - 1, y);
	if (x < width - 1
	&& exposed[y][x + 1] < 0								// unexposed
	&& listUsed[y][x + 1] < 0)								// not on some list
		CreateList(x + 1, y);
	if (x > 0 && y < height - 1
	&& exposed[y + 1][x - 1] < 0							// unexposed
	&& listUsed[y + 1][x - 1] < 0)							// not on some list
		CreateList(x - 1, y + 1);
	if (y < height - 1
	&& exposed[y + 1][x] < 0								// unexposed
	&& listUsed[y + 1][x] < 0)								// not on some list
		CreateList(x, y + 1);
	if (x < width - 1 && y < height - 1
	&& exposed[y + 1][x + 1] < 0							// unexposed
	&& listUsed[y + 1][x + 1] < 0)							// not on some list
		CreateList(x + 1, y + 1);
}


void CreateList(int x, int y)
{
	listUsed[y][x] = lists;
	List * list = &listArray[listNext];
	list->x = x;
	list->y = y;
	list->count = 0;
	list->num = 0;
	list->tried = -1;
	listNext++;
	if (x > 0 && y > 0
	&&  exposed[y - 1][x - 1] > 0
	&&  exposed[y - 1][x - 1] < EXPOSE_MINE)
	{
		AddNeighbor(list, x - 1, y - 1);
		CheckNeighbors(x - 1, y - 1);
	}
	if (y > 0
	&& exposed[y - 1][x] > 0
	&& exposed[y - 1][x] < EXPOSE_MINE)
	{
		AddNeighbor(list, x, y - 1);
		CheckNeighbors(x, y - 1);
	}
	if (x < width - 1 && y > 0
	&& exposed[y - 1][x + 1] > 0
	&& exposed[y - 1][x + 1] < EXPOSE_MINE)
	{
		AddNeighbor(list, x + 1, y - 1);
		CheckNeighbors(x + 1, y - 1);
	}
	if (x > 0
	&& exposed[y][x - 1] > 0
	&& exposed[y][x - 1] < EXPOSE_MINE)
	{
		AddNeighbor(list, x - 1, y);
		CheckNeighbors(x - 1, y);
	}
	if (x < width - 1
	&& exposed[y][x + 1] > 0
	&& exposed[y][x + 1] < EXPOSE_MINE)
	{
		AddNeighbor(list, x + 1, y);
		CheckNeighbors(x + 1, y);
	}
	if (x > 0 && y < height - 1
	&& exposed[y + 1][x - 1] > 0
	&& exposed[y + 1][x - 1] < EXPOSE_MINE)
	{
		AddNeighbor(list, x - 1, y + 1);
		CheckNeighbors(x - 1, y + 1);
	}
	if (y < height - 1
	&& exposed[y + 1][x] > 0
	&& exposed[y + 1][x] < EXPOSE_MINE)
	{
		AddNeighbor(list, x, y + 1);
		CheckNeighbors(x, y+ 1);
	}
	if (x < width - 1 && y < height - 1
	&& exposed[y + 1][x + 1] > 0
	&& exposed[y + 1][x + 1] < EXPOSE_MINE)
	{
		AddNeighbor(list, x + 1, y + 1);
		CheckNeighbors(x + 1, y + 1);
	}
}


void CreateTankLists()
{
	listNext = 0;
	neighborNext = 0;
	lists = 0;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			listUsed[y][x] = -1;
		}
	}
	// end game check all unexposed entries
	if (remainingCount + mineCount <= endGameCount)
	{
		listStart[lists] = 0;
		neighborStart[lists] = neighborNext;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (exposed[y][x] < 0					// find an unexposed tile
				&& listUsed[y][x] < 0)					// not on the list already
				{
					CreateList(x, y);
				}
			}
		}
		listEnd[lists] = listNext - 1;
		neighborEnd[lists] = neighborNext - 1;
		lists++;
		return;
	}
	// else segregate lists
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (listUsed[y][x] >= 0					// already on some list
			||  exposed[y][x] >= 0					// find an unexposed tile
			||  neighbors[y][x] == 0)				// we need real neighbors (not just mines)
				continue;
			listStart[lists] = listNext;
			neighborStart[lists] = neighborNext;
			CreateList(x, y);
			listEnd[lists] = listNext - 1;
			neighborEnd[lists] = neighborNext - 1;
			lists++;
		}
	}
}


bool Conflict(int list, int maxCount)
{
	if (minesUsed > maxCount)							// do not exceed the number of mines we have
		return true;
	for (int n = neighborStart[list]; n <= neighborEnd[list]; n++)
	{
		if (neighborArray[n].needed < 0)
			return true;
		if (neighborArray[n].needed > 0
		&& neighborArray[n].needed > neighborArray[n].unexposed)
			return true;
	}
	return false;
}


void Trymine(int list, int index, int maxCount)
{
	listArray[index].tried = 1;							// place mine
	minesUsed++;
	for (int n = 0; n < listArray[index].num; n++)		// reflect that in our neighbors
	{
		listArray[index].neighbors[n]->needed--;
		listArray[index].neighbors[n]->unexposed--;
	}
	if (Conflict(list, maxCount) == false)				// we have no conflict so far
	{
		CreateSolutions(list, maxCount);				// continue until we are done
	}
	listArray[index].tried = -1;				
	minesUsed--;
	for (int n = 0; n < listArray[index].num; n++)		// undo mine
	{
		listArray[index].neighbors[n]->needed++;
		listArray[index].neighbors[n]->unexposed++;
	}
}


void TryExpose(int list, int index, int maxCount)
{
	listArray[index].tried = 0;							// expose this
	for (int n = 0; n < listArray[index].num; n++)		// reflect that in our neighbors
	{
		listArray[index].neighbors[n]->unexposed--;
	}
	if (Conflict(list, maxCount) == false)
	{
		CreateSolutions(list, maxCount);				// continue until we are done
	}
	listArray[index].tried = -1;
	for (int n = 0; n < listArray[index].num; n++)		// undo expose
	{
		listArray[index].neighbors[n]->unexposed++;
	}
}


void CreateSolutions(int list, int maxCount)
{
	for (int index = listStart[list]; index <= listEnd[list]; index++)
	{
		if (listArray[index].tried >= 0)				// already filled this entry
			continue;
		Trymine(list, index, maxCount);					// try a mine here	
		TryExpose(list, index, maxCount);				// else try expose here
		return;
	}
	if (remainingCount + mineCount <= endGameCount
	&&  minesUsed != maxCount)							// must match mine count exactly
		return;
	// count completed solution
	listSolutions[list]++;
	int mines = 0;
	for (int i = listStart[list]; i <= listEnd[list]; i++)
	{
		if (listArray[i].tried > 0)
		{
			mines++;
			listArray[i].count++;						// count if mine was used
		}
	}
	if (listMinMines[list] > mines)						// save min and max mines used
		listMinMines[list] = mines;
	if (listMaxMines[list] < mines)
		listMaxMines[list] = mines;
}


//	Create all possible solutions for each list
//	We do this by adding a mine in the next position
//	and if this causes a conflict return false 
//	and if this exposes an entry then see if this causes a conflist too
//	If there are no conflicts, repeat the process on the next unused entry on the list
//	When done mark those list entries with a mine and update the count of solutions
//	Repeat for all lists
bool CreateTankSolutions()
{
	bool ret = true;
	for (int i = 0; i < lists; i++)
	{
		listSolutions[i] = 0;
		listMaxMines[i] = 0;
		listMinMines[i] = MAX_MINES;
	}
	for (int list = 0; list < lists; list++)
	{
		int len = listEnd[list] - listStart[list] + 1;
		if (len > MAX_SEQUENCE)							// add this to mark very long lists
		{
			if (testResults != NULL)
				fprintf(testResults, "\n***** WARNING: Puzzle=%d Very long squence (len=%d)\n", testing, len);
		}
		minesUsed = 0;
		CreateSolutions(list, mineCount - lists + 1);	// create solutions
	}
	int max = 0;
	int min = 0;
	int solutions = 0;
	for (int i = 0; i < lists; i++)
	{
		min += listMinMines[i];
		max += listMaxMines[i];
		solutions += listSolutions[i];
	}
	// if there the max solutions exceed our mineCount
	// then reduce the max range to a value that keeps the limits right
	// this means the max of one plus the min of the others <= mineCount
	if (max > mineCount)
	{
		int play = mineCount - min;						// max amount of play for our solutions
		max = mineCount;
		for (int i = 0; i < lists; i++)
		{
			if (listMaxMines[i] - listMinMines[i] > play)
			{
				ret = false;							// let the caller know we checked further
				for (int n = listStart[i]; n <= listEnd[i]; n++)
				{
					listArray[n].count = 0;				// remove old count
				}
				int limit = listMinMines[i] + play;
				minesUsed = 0;
				listSolutions[i] = 0;
				listMaxMines[i] = 0;
				listMinMines[i] = MAX_MINES;
				CreateSolutions(i, limit);				// create solutions with new limits
			}
		}
	}
	// Also if the sequences takes up all available entries
	// but we are not in end play
	// then make sure the sequences uses exactly mineCount mines
	// we do this by combining all lists into one
	// provided there are not too many entries (time issue)
	// I know it would be best to exclude any lists where min == max
	// but this is too hard with all the neighbor lists
	// but do this only when min != max
	// see examples/puzzle250 rule10 failed.txt
	// see examples/puzzle55 rule10 failed.txt
	if (endGameCount != mineCount + remainingCount
	&&	listNext == mineCount + remainingCount
	&&  lists > 1
	&&  min != max)
	{
#if 0
		if (testing > 0)
		{
			char testFileName[MAX_PATH];
			sprintf(testFileName, "%s\\check%d.txt", testFilePath, testing);
			SavePuzzle(testFileName);
		}
#endif
		if (listNext > MAX_COMBINE_LISTS)				// add this to prevent excess time to solve
		{
			if (testResults != NULL)
				fprintf(testResults, "\n***** WARNING: Puzzle=%d Unable to combine lists (len=%d)\n", testing, listNext);
			return ret;
		}
		endGameCount = mineCount + remainingCount;
		lists = 1;
		listEnd[0] = listNext - 1;
		neighborEnd[0] = neighborNext - 1;				// last neighbor for this list
		ret = false;									// let the caller know we checked further
		for (int n = 0; n < listNext; n++)
			listArray[n].count = 0;						// remove old count
		for (int y = 0; y < height; y++)				// all lists use list 0
			for (int x = 0; x < width; x++)
				if (listUsed[y][x] > 0)
					listUsed[y][x] = 0;
		minesUsed = 0;
		listSolutions[0] = 0;
		listMaxMines[0] = 0;
		listMinMines[0] = MAX_MINES;
		CreateSolutions(0, mineCount);					// create solutions with new limits
	}
	return ret;
}


void CalculateProb()
{
	int list = 0;
	for (int i = 0; i < listNext; i++)
	{
		listArray[i].prob = (double)listArray[i].count / listSolutions[list];
		if (i == listEnd[list])
			list++;										// use next list
	}
}


bool FindEasyTankMove()
{
	for (int i = 0; i < listNext; i++)
	{
		if (listArray[i].count == 0)
		{
			SaveBackUp();								// save current board
			Expose(listArray[i].x, listArray[i].y);
			BackUpChanged();							// check to see if anything changed
			return true;
		}
		if (listArray[i].count == listSolutions[listUsed[listArray[i].y][listArray[i].x]])
		{
			SaveBackUp();								// save current board
			AddMine(listArray[i].x, listArray[i].y);
			BackUpChanged();							// check to see if anything changed
			return true;
		}
	}
	return false;
}


int CountExposedNeighbors(int x, int y)						
{
	int count = 0;
	if (x > 0 && y > 0
	&& exposed[y - 1][x - 1] > 0)						// exposed
		count += 1;
	if (y > 0
	&& exposed[y - 1][x] > 0)							// exposed
		count += 2;
	if (x < width - 1 && y > 0
	&& exposed[y - 1][x + 1] > 0)						// exposed
		count += 1;
	if (x > 0
	&& exposed[y][x - 1] > 0)							// exposed
		count += 2;
	if (x < width - 1
	&& exposed[y][x + 1] > 0)							// exposed
		count += 2;
	if (x > 0 && y < height - 1
	&& exposed[y + 1][x - 1] > 0)						// exposed
		count += 1;
	if (y < height - 1
	&& exposed[y + 1][x] > 0)							// exposed
		count += 2;
	if (x < width - 1 && y < height - 1
	&& exposed[y + 1][x + 1] > 0)						// exposed
		count += 1;
	return count;
}


// find the tank move with the best odds
// but also consider those with more neighbors better
double FindBestMove(int & list, int & index)
{
	int curList = 0;
	int bestCount = CountExposedNeighbors(listArray[0].x, listArray[0].y);
	double prob = listArray[0].prob;
	int count;
	list = 0;
	index = 0;
	for (int i = 1; i < listNext; i++)
	{
		if (i == listStart[curList + 1])				// starting next list
			curList++;
		count = CountExposedNeighbors(listArray[i].x, listArray[i].y);
		if (listArray[i].prob < prob
		|| (listArray[i].prob == prob && count > bestCount))
		{
			bestCount = count;
			prob = listArray[i].prob;
			index = i;
			list = curList;
		}
	}
	return prob;
}


void FindListEntry(int x, int y, int list, int & count)
{
	for (int n = listStart[list]; n <= listEnd[list]; n++)
	{
		if (listArray[n].x == x
		&&  listArray[n].y == y)
		{
			count++;
			return;
		}
	}
}


int CheckExposed(int x, int y, bool offedge = false)
{
	if (x < 0
	|| y < 0
	|| x >= width
	|| y >= height)
	{
		if (offedge)
			return 1;
		return 0;
	}
	if (listUsed[y][x] >= 0
	&&  exposed[y][x] > 0
	&&  exposed[y][x] < EXPOSE_MINE)
		return 1;											// exposed part of some sequence
	return 0;
}


void CountListEntries(int x, int y, int & count, int & exposedCount)
{
	count = 0;
	exposedCount = 0;
	if (x > 0 && y > 0
	&& listUsed[y - 1][x - 1] >= 0)							// list entry
		FindListEntry(x - 1, y - 1, listUsed[y - 1][x - 1], count);
	if (y > 0
	&& listUsed[y - 1][x] >= 0)								// list entry
		FindListEntry(x, y - 1, listUsed[y - 1][x], count);
	if (x < width - 1 && y > 0
	&& listUsed[y - 1][x + 1] >= 0)							// list entry
		FindListEntry(x + 1, y - 1, listUsed[y - 1][x + 1], count);
	if (x > 0
	&& listUsed[y][x - 1] >= 0)								// list entry
		FindListEntry(x - 1, y, listUsed[y][x - 1], count);
	if (x < width - 1
	&& listUsed[y][x + 1] >= 0)								// list entry
		FindListEntry(x + 1, y, listUsed[y][x + 1], count);
	if (x > 0 && y < height - 1
	&& listUsed[y + 1][x - 1] >= 0)							// list entry
		FindListEntry(x - 1, y + 1, listUsed[y + 1][x - 1], count);
	if (y < height - 1
	&& listUsed[y + 1][x] >= 0)								// list entry
		FindListEntry(x, y + 1, listUsed[y + 1][x], count);
	if (x < width - 1 && y < height - 1
	&& listUsed[y + 1][x + 1] >= 0)							// list entry
		FindListEntry(x + 1, y + 1, listUsed[y + 1][x + 1], count);
	if (CheckExposed(x - 2, y) != 0
	&&  exposed[y][x - 1] < 0)
	{
		if (CheckExposed(x - 2, y - 1, true) != 0)
			exposedCount += 6 + 2 * CheckExposed(x - 2, y + 1, true);
		else if (CheckExposed(x - 2, y + 1, true) != 0)
			exposedCount += 6;
		else
			exposedCount += 1;
	}
	if (CheckExposed(x + 2, y) != 0
	&& exposed[y][x + 1] < 0)
	{
		if (CheckExposed(x + 2, y - 1, true) != 0)
			exposedCount += 6 + 2 * CheckExposed(x + 2, y + 1, true);
		else if (CheckExposed(x + 2, y + 1, true) != 0)
			exposedCount += 6;
		else
			exposedCount += 1;
	}
	if (CheckExposed(x, y - 2) != 0
	&& exposed[y - 1][x] < 0)
	{
		if (CheckExposed(x - 1, y - 2, true) != 0)
			exposedCount += 6 + 2 * CheckExposed(x + 1, y - 2, true);
		else if (CheckExposed(x + 1, y - 2, true) != 0)
			exposedCount += 6;
		else
			exposedCount += 1;
	}
	if (CheckExposed(x, y + 2) != 0
	&& exposed[y + 1][x] < 0)
	{
		if (CheckExposed(x - 1, y + 2, true) != 0)
			exposedCount += 6 + 2 * CheckExposed(x + 1, y + 2, true);
		else if (CheckExposed(x + 1, y + 2, true) != 0)
			exposedCount += 6;
		else
			exposedCount += 1;
	}
//	exposedCount += CheckExposed(x - 2, y - 2);
//	exposedCount += CheckExposed(x - 2, y + 2);
//	exposedCount += CheckExposed(x + 2, y - 2);
//	exposedCount += CheckExposed(x + 2, y + 2);
}


bool PickOpenSpot(bool always)
{
	int avail = remainingCount + mineCount - listNext;	// number we could pick from not neighbors the tank list	
	if (avail <= 0)										// there are no open places
		return false;
	int mines = 0;
	for (int i = 0; i < lists; i++)
	{
		mines += listMinMines[i];
		mines += listMaxMines[i];
	}
	mines = mineCount - (mines >>= 1);					// average number of mines likely in the open areas
	if (mines < 0)
		mines = 0;
	double open = (double)mines / avail;				// odds in the open area
	open += guess1Offset;
	int list, index;
	double odds = 1.0;
	if (lists > 0)										// nothing from the tank method
		odds = FindBestMove(list, index);				// best odds using the tank algorithm
	if (always
	||  open < odds)									// random placement is better than tank odds
	{
		freeNext = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				if (exposed[y][x] < 0					// not exposed
				&& listUsed[y][x] < 0)					// and not in any list
				{
					freeArray[freeNext].x = x;
					freeArray[freeNext].y = y;
					freeArray[freeNext].unexposed = unexposed[y][x];
					CountListEntries(x, y, freeArray[freeNext].listEntries, freeArray[freeNext].exposedCount);
					freeNext++;
				}
			}
		}
		// find best choice by looking for an entry with
		// 1. no unexposed neighbors but 100% chance of this being open
		// 2. ignore any entries with only 1 list neighbor
		//  because this would create an orphan which will not help us
		//	If we look for a good exposedCount (>= 7) we will get
		//	support for any listArray neighbors and avoid orphans.
		//	In case of ties get the highest listEntries
		//	If tied get the lowest unexposed count
		// 3. fewest neighbors and no list neighbors (corners are best)
		// the fewest unexposed entries
		// and in case of a tie the most number of list entry neighbors
		// if we get a large number we not hurt our chances of doing this again
		// but if we get the right value we will probably fill in a sequence of list entries
		int bestOpen = -1;									// best open area
		int bestOpenUnexposed = 99;
		int best = -1;										// best support for sequences
		int bestExposedCount = 0;							// get max of this
		int bestListEntries = 0;							// and max of this
		int bestUnexposed = 0;								// and min of this
		for (int i = 0; i < freeNext; i++)
		{
			// surrounded by mines 
			if (freeArray[i].unexposed == 0)
			{
				if (open == 0.0)							// we know it is open
				{
					bestOpen = best = i;
					break;
				}
				continue;									// ignore this entry
			}
			// totally open area
			if (freeArray[i].listEntries == 0)
			{
				if (bestOpen < 0
				|| bestOpenUnexposed > freeArray[i].unexposed)
				{
					bestOpen = i;
					bestOpenUnexposed = freeArray[i].unexposed;
				}
				continue;
			}
			// list entries nearby
			if (freeArray[i].listEntries == 1)				// avoid single list neighbors
				continue;
			if (freeArray[i].exposedCount < MIN_GUESS1_COUNT)	// look for a good count
				continue;
			if (best < 0
			|| bestExposedCount < freeArray[i].exposedCount)
			{
				best = i;
				bestExposedCount = freeArray[i].exposedCount;
				bestListEntries = freeArray[i].listEntries;
				bestUnexposed = freeArray[i].unexposed;
			}
			else if (bestExposedCount == freeArray[i].exposedCount
			&&  bestListEntries < freeArray[i].listEntries)
			{
				best = i;
				bestListEntries = freeArray[i].listEntries;
				bestUnexposed = freeArray[i].unexposed;
			}
			else if (bestExposedCount == freeArray[i].exposedCount
			&&  bestListEntries == freeArray[i].listEntries
			&&  bestUnexposed > freeArray[i].unexposed)
			{
				best = i;
				bestListEntries = freeArray[i].listEntries;
				bestUnexposed = freeArray[i].unexposed;
			}
		}
		if (best < 0)										// support sequences first
			best = bestOpen;
		if (best < 0)										// nothing meets our criteria
			return false;
		SaveBackUp();										// save current board
		guesses++;
		Expose(freeArray[best].x, freeArray[best].y);
		lastProb = prob;
		guess1Prob += (1.0 - open);
		prob *= (1.0 - open);								// probability
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	return false;
}


bool PickBestSpot()
{
	if (lists > 0)
	{
		int list, index;
		double odds = FindBestMove(list, index);			// best odds using the tank algorithm
		SaveBackUp();										// save current board
		lastProb = prob;
		guesses++;
		guess2Prob += (1.0 - listArray[index].prob);
		prob *= (1.0 - listArray[index].prob);				// probability
		Expose(listArray[index].x, listArray[index].y);
		BackUpChanged();									// check to see if anything changed
		return true;
	}
	return false;
}
