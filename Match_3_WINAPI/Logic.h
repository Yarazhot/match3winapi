#pragma once
#include <vector>
typedef struct TCell
{
	int row;
	int col;
};
typedef std::vector<std::vector<int>> TIntMx;
typedef std::vector<TCell> TCells;
typedef std::vector<POINT> POINTSS;


void CreateMx(TIntMx&, int, int);
bool Check(TIntMx&);
void Falling(TIntMx&);
void AddGems(TIntMx&);
void Swap(TIntMx&, POINT, POINT);
bool CanMove(TIntMx);
void ShowMx(TIntMx);