This game will create and try to solve a Minesweeper puzzles.
It will always start at the specified location.  This location has been 
freed of all neighboring mines to give the player a free starting area.
We use a 64 bit pseudo random number generator to create the puzzles.
The layout is somewhat different than the normal Windowds MineSweeper.
Besides the number of mines remaining a percentage is listed instead of 
the usual time.  This percentage is the probability of success.  If no 
guess is involved the percentage is 100%.  Otherwise the probability of 
each guess is multiplied by the current probability.

There ar three sizes of puzzles:
Beginner or small: 9x9 with 10 mines - one mine per 8.1 entries
Intermediate or medium: 16x16 with 40 mines - one mine per 6.4 entries
Expert: 30x16 with 99 mines - one mine per 4.85 entries

File Menu:
1. You can select the size of the puzzle as well as load and save puzzles
2. Note the last puzzle is saved whatever you exit.  It is reloaded when
you restart the game.

Game Menu:
1. Solve will solve the puzzle the best it can, even guessing when it has to.
2. Next move allows you to see one move at a time what it is doing
3. Backup allows a one move backup, especially useful when undoing the last 
fatal move.
4. H and V Flip are provided to test some of the rules.
5. Cheat shows or hides all the mines
6. The two types of guesses can be enabled or disabled as desired.

Test Menu:
1. You can select a number of puzzles to create and solve with the current
settings.  It will create a folder name using the current date and any changed 
puzzle defaults (i.e. starting position).  In this folder will be a results 
file showing all the test results.
2. You can enable saving the puzzles too but I do not recommend this for
large tests because it creates a file for each puzzle.
3. You can set the random seed used for the test you are about to select.  
This is very useful to repeat the same puzzles with different paramters.
If no random seed is used, seed=1 is assumed.
4. You can change some solving parameters such as the starting location, the
number of free entries before end play is used, and an offset to determine
when guess1 is used instead of guess2.  
5. You can run a set of tests using 10 random seeds with different parameters.
This is useful to get a rough best guess on the best parameters before using 
test200x100 (20,000 puzzles) or test1000x100 (100,000 puzzles) to get a more 
refined result.  Please note that the basic tests for starting position, end play, 
and guess1 offset only create 5000 puzzles and this is not enough to get any 
reliable results.  For example, using the starting position of 4,3 we get a result 
of 47.2% win rate but 100,000 puzzles generates an estime of 46.648% with a 
standard deviation of .16%.

WARNING: when running some tests it is possible that the game will appear to hang.
It really has not.  What has happened is the game runs into a situation where the
number of possible solutions is very large so it will take some time to compute.
When this occurs you will notice a warning in the results file when too much time
has been taken.

The algorithm used to solve puzzles is:
Rule 1: If a square has the right number of mines as neighbors, expose any neighbors
Rule 2: if the number of empty squares is equal to the number of missing mines, add them.
The following rules are just simple cases of more advaced strategies but make it
easier to solve without a lot of compute power.  Besides they are useful for
the casual player to see and use.
Rule 3: If a sqaure must be a mine due to neighbors, add them:
	XXX
	X21
	?   this must be a mine
Rule 4: Similar to Rule 3, if a neighbor cannot be a mine fill it in:
	XXXX
	X11X
	?  X    expose ? because it cannot be a mine
Rule 5: 
	X**?	* may or may not be exposed
	Xnm?	n and m need the same number of mines
	X**?	expose all unexposed ?
Rule 6: 
	X**? 
	Xnm?	where m needs mines at unexposed ? to match n
	X**?	place mines at any unexpose ?
Rule 7: 
	Xa-		mines needed[a] = mines needed[b] = 1
	bm-		if mines needed[m] = 3 the put a mine at ?
	--?		if unexposed[a] and unexposed[b] = 2 then expose ?
Rule 8:
	XX*?? 
	Xm*n?	mines needed[m] = mines needed[n] 
	XX*??	expose all unexposed ?
Rule 9:
	XX*?? 
	Xm*n?	mines needed[n] - mines needed[m] = unexposed[n] - unexposed[m]	
	XX*??	place mines at all unexposed ?
	
Rule 10 is a general solutuion similar to rules 3-9 above.
See https://luckytoilet.wordpress.com/2012/12/23/2125/ for a complete description.
We find all sequences of entries that are connected (i.e. those that affect the 
others).  Next we test all possible placements of mines in a connected sequence.
The probability of a mine being this location is the count from all sequences
found above divided by the number of solutions.  This probability is used 
to pick the best guess by picking the lowest proability entry with the best 
chance of getting more information to solve the sequence.  I do not use the
max probability of the neighbors because this is wrong:
		1-2?
		2-??
		3B--
		M---
		probability at each ? is really 33% not 40% 
I have made an extension to the algorithm above.  First of all I check to make sure
the number of mines never exceeds the remaining number.  For example, if there
are 6 mines left and 3 sequences, the max allowed is 4 for each of the sequences.
So we can discard any possibilities that do not match these conditions.
Second, if the total maximums for the sequences exceed the remaining mines,
I redo the sequences reducing the max allowed.  For example, suppose
there are 3 sequences with min and max number of mines of 1-2, 3-4, and 2-2 
with 6 mines remaining.  Now we know the second sequences can have only 3 mines 
and the first sequence only 1 mine.  So we redo the checks again with these new limits.
Rule 10 says if there are any entries where mines cannot occur, expose it.
If there are entries where a mine must always occur, place a mine there.
If neither of the cases above occur we must guess.

Guess 1:
If the remaining open spaces is better than guess 2 then use this method.
The goal here is two fold.  If we can pick a place next to an adjacent sequence
to help us solve the sequence, but be sure not we do not pick next to orphans.
Orphans are single cells not connected to others horizontally or vertically.
If we expose a number large (that will not allow us solve the sequence) at 
least we improve the odds for picking a second time in the open areas.
The second use is to pick a place with no neighboring mines so we can start a 
second area to work on.  The corners are the most likely since they have only
three neighbors that could be mines.  We do try to avoid filling an empty area 
surrounded by all mines unless the probability of success is 100%.  The algorithm 
used for the first use is to give a score for all neighbors to a sequence.  The
score is calculated based on the following situation (using all four directions).
We give a score of 4 for being vertical or horizontal to an exposed entry
assuming there is an unexposed space in between, and two for each adjacent 
exposed entry which is not a mine.  If we are on the edge count the entries 
marked as 2 as if they were exposed.  I.e look for these cases:
	2
	4-?
	2
We must have a minimum count of 8 before we consider this valid.  So the 
highest score is the best.  In case of ties use the entry with the highest 
number of neighbors that are part of a sequence.  If this ties use the entry 
with the fewest number of unexposed neighbors.  If we have no choise  for 
supporting a sequence we use the best score for the second use.

Guess 2:
Otherwise we pick the first lowest odds play based on the max of the odds of a 
mine for each of the neighbors.  We give a little edge to those entries horizontal
or vertical to other exposed entries in the hope of increasing the chances of getting
a rule to continue to solve the puzzle.  We again try to avoid orphans because 
these will not help us find solutions but just increase the sequence.

See the files "Results Start Position.txt", "Results End Play.txt", and "Results
Guess1 Offset.txt" for the results of the tests using seed=1.  These tests were
used to set the defaults for the game paramters.  

The default value for end play (30) was chosen using the data in the results file
"Results End Play.txt".  As one would expect the results get better with the high 
end play values.  I stopped at 30 because of CPU constraints.  It will take longer
to calculate all solutions with unconnected sequences of length greated than this.

If we offset the odds of using guess1 over guess2 we get different results.
Negative offsets means we use more guess1 and less guess2. The default value of
-.02% was chosen using the data from the results shown in "Results Guess1 Offset.txt".

Different starting locations generate different win percentages.  The results can 
be found in the file "Results Start Position.txt".  Note, that the best starting 
position for the expert game is different for the small puzzles.  However, I used
the same starting position anyway because it is so close.  The best results using 
200,000 samples is:
Size	Pos		Percent	Estimated deviation
-------------------------------------------
small	2,2		96.955%	.04%	
small	2,1		96.919%	.04%
small	3,2		96.886%	.04%
small	3,3		96.671%	.04%
medium	3,2		87.683%	.08%
medium	2,2		87.618%	.08%
medium	3,3		87.541%	.08%
expert	3,3		46.916%	.11%
expert	3,2		46.833%	.11%


Using a new 64 bit random number generator we find the best winning percentage using 
Test1000x100 with seeds 1-1000.  For the expert game we get:
start=3,3
endPlay=30
Guess1Offset=-0.02
Samples: 100000
Wins: 47066
Percent: 47.07%
Deviation: 0.16%
Guess1 Used: 71350
Guess2 Used: 163179
Guess1 Failed: 18.4%
Guess2 Failed: 24.4%
Average guess per win: 1.81
Average guess per loss: 2.78
Average win prob: 70.88
Average loss prob: 55.91
Average guess1 prob: 84.20
Average guess2 prob: 77.47
Wins with guesses:
     0     1     2     3     4     5     6     7     8+
 16383 10599  6785  4614  3158  2050  1307   873  1297
 16.4% 10.6%  6.8%  4.6%  3.2%  2.0%  1.3%  0.9%  1.3%


For the medium sized puzzles we get:
start=3,2
endPlay=30
Guess1Offset=-0.02
Samples: 100000
Wins: 87650
Percent: 87.65%
Deviation: 0.10%
Guess1 Used: 11310
Guess2 Used: 31180
Guess1 Failed: 14.0%
Guess2 Failed: 34.5%
Average guess per win: 0.28
Average guess per loss: 1.48
Average win prob: 93.74
Average loss prob: 56.94
Average guess1 prob: 88.74
Average guess2 prob: 66.74
Wins with guesses:
     0     1     2     3     4     5     6     7     8+
 72257 10382  2871  1206   514   225   104    49    42
 72.3% 10.4%  2.9%  1.2%  0.5%  0.2%  0.1%  0.0%  0.0%



For the small sized puzzles we get:
starting at (2,2)
guess1Offset=-0.02
endPlayCount=30
Samples: 100000
Wins: 96981
Percent: 96.98%
Deviation: 0.05%
Guess1 Used: 3612
Guess2 Used: 6396
Guess1 Failed: 8.9%
Guess2 Failed: 42.2%
Average guess per win: 0.07
Average guess per loss: 1.12
Average win prob: 98.44
Average loss prob: 56.17
Average guess1 prob: 94.33
Average guess2 prob: 57.97
Wins with guesses:
     0     1     2     3     4     5     6     7     8+
 91835  4084   744   234    69    12     3     0     0
 91.8%  4.1%  0.7%  0.2%  0.1%  0.0%  0.0%  0.0%  0.0%


Most of the stats above are self explanatory, however, there are a few that 
need explanations or is of some interest.  
* Guess1 failed is less than Guess2 failed is always a good sign because it
means we are doing the right thing picking Guess1 first.
* Average Guess1 probability is higher than Guess2 also means we are making
the right choice to use Guess1 instead of Guess2.  Notice the high probability
of success for both Guess1 and Guess2.  This is good.
* The average number of guesses per win is close to two means we generally 
need to guess twice before winning.
* The average number of guesses per loss is often close to one more than
a win which is what I would expect.
* Notice we can win less 16% of games without guessing and only another 10%
guessing only once.  In general we win 33% of games guessing twice or less.
* The average win/loss probability is the product of all guesses so far up
until we win or lose.


To do:
* should I add guess1a and guess1b to rules to disable one or the other or add an offset like with guess1offset?
* should I try to place mine in guess2 instead of exposing if mine prob < expose prob????
* Should I do some lookahead in the above case?



