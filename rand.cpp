#include "stdafx.h"


//	Setting seeds:
//	Use of KISS or KISS() as a general 64 - bit RNG requires specifying
//	3 * 64 + 58 = 250 bits for seeds, 64 bits each for x, y, z and 58 for c,
//	resulting in a composite sequence with period around 2 ^ 250.
//	The actual period is
//	(2 ^ 250 + 2 ^ 192 + 2 ^ 64 - 2 ^ 186 - 2 ^ 129) / 6 ~= 2 ^ (247.42) or 10 ^ (74.48).
//	We "lose" 1 + 1.58 = 2.58 bits from maximum possible period, one bit
//	because b = 2 ^ 64, a square, cannot be a primitive root of p = ab - 1,
//	so the best possible order for b is (p - 1) / 2.
//	The periods of MWC and  XSH have gcd 3 = 2 ^ 1.58, so another  1.58
//	bits  are "lost" from the best possible period we could expect
//	from 250 seed bits.
//	
//	Some users may think 250 seed bits are an unreasonable requirement.
//	A good seeding procedure might be to assume the default seed
//	values then let the user choose none, one, two, ..., or all
//	of x, y, z, and c to be reseeded.
//
static U64 kiss64_x = 1234567890987654321ULL;
static U64 kiss64_c = 123456123456123456ULL;
static U64 kiss64_y = 362436362436362436ULL;
static U64 kiss64_z = 1066149217761810ULL;
static U64 kiss64_t = 0;

void RandomSeed(U64 seed)
{
	kiss64_x = seed | 1;
	kiss64_c = seed | 2;
	kiss64_y = seed | 4;
	kiss64_z = seed | 8;
	kiss64_t = 0;
}


//
//	Warning: Using the method below slightly biases the results near 0.
//	For example suppose U32 were only returning 0-31, then with in = 30
//	we would get two 0 and 1 but only 1 of the other results.
//	But with U64 we would get a slight bias in the range of 1 out of 2^59.
//
U64 Random(U32 in)
{
	// multiply with carry
	kiss64_t = (kiss64_x << 58) + kiss64_c;
	kiss64_c = (kiss64_x >> 6);
	kiss64_x += kiss64_t;
	kiss64_c += (kiss64_x < kiss64_t);
	// XOR shift
	kiss64_y ^= (kiss64_y << 13);
	kiss64_y ^= (kiss64_y >> 17);
	kiss64_y ^= (kiss64_y << 43);
	// Congruential
	kiss64_z = 6906969069LL * kiss64_z + 1234567;
	U64 rand = kiss64_x + kiss64_y + kiss64_z;
	if (in == 0)
		return rand;
	else
		return rand % in;
}


void	RandomInit(void)
{
	SYSTEMTIME C_time;
	__int64 C_z, C_aux;

	// Make it depend on the date and tick count
	GetSystemTime(&C_time);

	C_aux = C_time.wYear;
	C_z = C_aux << (22 + 6 + 6 + 5 + 5 + 4);

	C_aux = C_time.wMonth;
	C_z += C_aux << (22 + 6 + 6 + 5 + 5);

	C_aux = C_time.wDay;
	C_z += C_aux << (22 + 6 + 6 + 5);

	C_aux = C_time.wHour;
	C_z += C_aux << (22 + 6 + 6);

	C_aux = C_time.wMinute;
	C_z += C_aux << (22 + 6);

	C_aux = C_time.wSecond;
	C_z += C_aux << 22;

	C_aux = GetTickCount();
	C_z += C_aux;
	RandomSeed((U64)C_z);
}

