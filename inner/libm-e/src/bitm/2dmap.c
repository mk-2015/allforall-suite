#include <bitm/2dmap.h>

#include <stdlib.h>
#include <string.h>

static int fits_size(unsigned long long v)
{
	return v <= (unsigned long long)SIZE_MAX;
}

static void error_clear(Map2D* Map)
{
	if (Map)
		Map->Error = (err)0;
}

static void grid_free(Map2D_Grid grid, unsigned long long rows)
{
	unsigned long long r;

	if (!grid)
		return;

	for (r = 0; r < rows; r++)
		free(grid[r]);

	free(grid);
}

static Map2D_Grid grid_alloc(unsigned long long x, unsigned long long y)
{
	Map2D_Grid grid;
	unsigned long long r;

	if (x == 0 || y == 0)
		return NULL;
	if (!fits_size(x) || !fits_size(y))
		return NULL;
	if ((size_t)y > SIZE_MAX / sizeof(uint8_t*))
		return NULL;

	grid = (Map2D_Grid)calloc((size_t)y, sizeof(uint8_t*));
	if (!grid)
		return NULL;

	for (r = 0; r < y; r++)
	{
		grid[r] = (uint8_t*)calloc((size_t)x, sizeof(uint8_t));
		if (!grid[r])
		{
			grid_free(grid, r);
			return NULL;
		}
	}

	return grid;
}

static uint8_t* selected_byte(Map2D* Map)
{
	if (!Map || !Map->Map)
		return NULL;
	if (Map->selected_byte_y >= Map->y || Map->selected_byte_x >= Map->x)
		return NULL;

	return &Map->Map[Map->selected_byte_y][Map->selected_byte_x];
}

static int valid_bit(int8_t n)
{
	return n >= 0 && n <= 7;
}

Map2D* Map2D_Init(unsigned long long x, unsigned long long y)
{
	Map2D* Map;
	Map2D_Grid grid;

	grid = grid_alloc(x, y);
	if (!grid)
		return NULL;

	Map = (Map2D*)calloc(1, sizeof(Map2D));
	if (!Map)
	{
		grid_free(grid, y);
		return NULL;
	}

	Map->Map = grid;
	Map->Error = (err)0;
	Map->x = x;
	Map->y = y;
	Map->selected_byte_x = 0;
	Map->selected_byte_y = 0;

	return Map;
}

void Map2D_Deinit(Map2D* Map)
{
	if (!Map)
		return;

	grid_free(Map->Map, Map->y);

	Map->Map = NULL;
	Map->Error = (err)0;
	Map->x = 0;
	Map->y = 0;
	Map->selected_byte_x = 0;
	Map->selected_byte_y = 0;

	free(Map);
}

err Map2D_GetLastError(Map2D* Map)
{
	if (!Map)
		return (err)0;

	return Map->Error;
}

void Map2D_Resize(Map2D* Map, unsigned long long x, unsigned long long y)
{
	Map2D_Grid grid;
	unsigned long long copy_x, copy_y, r;

	if (!Map)
		return;

	error_clear(Map);

	if (x == Map->x && y == Map->y)
		return;

	grid = grid_alloc(x, y);
	if (!grid)
	{
		Map->Error = MAP2D_MALLOCFAIL;
		return;
	}

	if (Map->Map)
	{
		copy_x = (x < Map->x) ? x : Map->x;
		copy_y = (y < Map->y) ? y : Map->y;

		for (r = 0; r < copy_y; r++)
			memcpy(grid[r], Map->Map[r], (size_t)copy_x);
	}

	grid_free(Map->Map, Map->y);

	Map->Map = grid;
	Map->x = x;
	Map->y = y;

	if (Map->selected_byte_x >= Map->x)
		Map->selected_byte_x = Map->x - 1;
	if (Map->selected_byte_y >= Map->y)
		Map->selected_byte_y = Map->y - 1;
}

void Map2D_Zero(Map2D* Map)
{
	unsigned long long r;

	if (!Map || !Map->Map)
		return;

	error_clear(Map);

	for (r = 0; r < Map->y; r++)
		memset(Map->Map[r], 0, (size_t)Map->x);
}

void Map2D_SelectByte(Map2D* Map, unsigned long long x, unsigned long long y)
{
	if (!Map)
		return;

	error_clear(Map);

	if (x >= Map->x || y >= Map->y)
		return;

	Map->selected_byte_x = x;
	Map->selected_byte_y = y;
}

void Map2D_ReplaceByte(Map2D* Map, uint8_t byte)
{
	uint8_t* p = selected_byte(Map);

	error_clear(Map);

	if (p)
		*p = byte;
}

void Map2D_SetByte(Map2D* Map, uint8_t byte)
{
	uint8_t* p = selected_byte(Map);

	error_clear(Map);

	if (!p)
		return;

	if (*p != 0)
	{
		Map->Error = MAP2D_BYTENOTZERO;
		return;
	}

	*p = byte;
}

void Map2D_ZeroByte(Map2D* Map)
{
	uint8_t* p = selected_byte(Map);

	error_clear(Map);

	if (p)
		*p = 0;
}

void Map2D_GetByte(Map2D* Map, uint8_t* out)
{
	uint8_t* p = selected_byte(Map);

	error_clear(Map);

	if (!p || !out)
		return;

	*out = *p;
}

void Map2D_BitSet(Map2D* Map, int8_t n)
{
	uint8_t* p;
	uint8_t mask;

	if (!Map)
		return;

	error_clear(Map);

	if (!valid_bit(n))
		return;

	p = selected_byte(Map);
	if (!p)
		return;

	mask = (uint8_t)(1u << n);

	if ((*p & mask) != 0)
	{
		Map->Error = MAP2D_BITNOTZERO;
		return;
	}

	*p |= mask;
}

void Map2D_BitClear(Map2D* Map, int8_t n)
{
	uint8_t* p;

	if (!Map)
		return;

	error_clear(Map);

	if (!valid_bit(n))
		return;

	p = selected_byte(Map);
	if (p)
		*p &= (uint8_t)~(1u << n);
}

void Map2D_BitToggle(Map2D* Map, int8_t n)
{
	uint8_t* p;

	if (!Map)
		return;

	error_clear(Map);

	if (!valid_bit(n))
		return;

	p = selected_byte(Map);
	if (p)
		*p ^= (uint8_t)(1u << n);
}

bool Map2D_BitGet(Map2D* Map, int8_t n)
{
	uint8_t* p;

	if (!Map)
		return false;

	error_clear(Map);

	if (!valid_bit(n))
		return false;

	p = selected_byte(Map);
	if (!p)
		return false;

	return (*p & (uint8_t)(1u << n)) != 0;
}
