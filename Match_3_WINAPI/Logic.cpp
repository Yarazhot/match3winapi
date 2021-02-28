//#include <iostream>
#include <windows.h>
#include <vector>
#include <ctime>
#include <math.h>
#include <cstdlib>
//#include "Logic.h"
/*typedef struct TCell
{
	int row;
	int col;
};*/
typedef std::vector<std::vector<int>> TIntMx;
//typedef std::vector<TCell> TCells;
typedef std::vector<POINT> POINTSS;

POINTSS CheckHor(TIntMx num_mx, POINT cur_cell);
POINTSS CheckVert(TIntMx num_mx, POINT cur_cell);
void DestroyGems(TIntMx& num_mx, POINTSS gem_line);

void CreateMx(TIntMx& num_mx, int N, int gem_var_num)
{
	num_mx.resize(N);
	for (int i = 0; i < N; i++)
		num_mx[i].resize(N);
	srand(time(NULL));
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			num_mx[i][j] = /*rand() % gem_var_num + */1;
}

void Swap(TIntMx& swap_mx, POINT c1, POINT c2)
{
	int buff = swap_mx[c1.y][c1.x];
	swap_mx[c1.y][c1.x] = swap_mx[c2.y][c2.x];
	swap_mx[c2.y][c2.x] = buff;
}

bool Check(TIntMx& num_mx)
{
	POINT cur_cell;
	POINTSS gem_line;
	bool founded = false;
	for (cur_cell.y = 0; cur_cell.y < num_mx.size(); cur_cell.y++)
	{
		for (cur_cell.x = 0; cur_cell.x < num_mx[0].size(); cur_cell.x++)
		{
			gem_line = CheckHor(num_mx, cur_cell);
			if (gem_line.size() > 2)
			{
				DestroyGems(num_mx, gem_line);
				founded = true;
			}
			gem_line = CheckVert(num_mx, cur_cell);
			if (gem_line.size() > 2)
			{
				DestroyGems(num_mx, gem_line);
				founded = true;
			}
		}
	}
	return founded;
}

POINTSS CheckHor(TIntMx num_mx, POINT cur_cell)
{
	POINTSS result;
	result.push_back(cur_cell);
	POINT buff;
	buff.y = cur_cell.y;
	for (int i = cur_cell.x + 1; i < num_mx[0].size(); i++)
		if (abs(num_mx[cur_cell.y][cur_cell.x]) == abs(num_mx[cur_cell.y][i]))
		{
			buff.x = i;
			result.push_back(buff);
		}
		else
			return result;
	return result;
}

POINTSS CheckVert(TIntMx num_mx, POINT cur_cell)
{
	POINTSS result;
	result.push_back(cur_cell);
	POINT buff;
	buff.x = cur_cell.x;
	for (int i = cur_cell.y + 1; i < num_mx.size(); i++)
		if (abs(num_mx[cur_cell.y][cur_cell.x]) == abs(num_mx[i][cur_cell.x]))
		{
			buff.y = i;
			result.push_back(buff);
		}
		else
			return result;
	return result;
}

void DestroyGems(TIntMx& num_mx, POINTSS gem_line)
{
	for (int i = 0; i < gem_line.size(); i++)
		num_mx[gem_line[i].y][gem_line[i].x] = -1 * abs(num_mx[gem_line[i].y][gem_line[i].x]);
}

void Falling(TIntMx& num_mx)
{
	int Buff;
	for (int j = 0; j < num_mx[0].size(); j++)
	{
		int n = 0;
		for (int k = num_mx.size() - 1; k > n; k--)
			if (num_mx[k][j] < 0)
			{
				for (int i = k; i > 0; i--)
				{
					Buff = num_mx[i][j];
					num_mx[i][j] = num_mx[i - 1][j];
					num_mx[i - 1][j] = Buff;
				}
				k++;
				n++;
			}
	}
}

void AddGems(TIntMx& num_mx)
{
	srand(time(NULL));
	for (int i = 0; i < num_mx.size(); i++)
		for (int j = 0; j < num_mx[i].size(); j++)
			if (num_mx[i][j] < 0)
				num_mx[i][j] = rand() % 5 + 1;
}

bool CanMove(TIntMx num_mx)
{
	bool result = false;
	TIntMx LocMx;
	POINT c1;
	POINT c2;
	int i = 0;
	int j;
	while (!result && (i < num_mx.size()))
	{
		j = 0;
		while (!result && (j < num_mx.size()))
		{
			if (j < num_mx.size() - 1)
			{
				LocMx = num_mx;
				c1.y = i;
				c1.x = j;
				c2.y = i;
				c2.x = j + 1;
				Swap(LocMx, c1, c2);
				result = Check(LocMx);
			}
			if (!result && i < num_mx.size() - 1)
			{
				LocMx = num_mx;
				c1.y = i;
				c1.x = j;
				c2.y = (i + 1);
				c2.x = j;
				Swap(LocMx, c1, c2);
				result = Check(LocMx);
			}
			j++;
		}
		i++;
	}
	return(result);
}