#include <data/map/3dmap.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fits_size(pos v)
{
    return v <= (pos)SIZE_MAX;
}

static void error_clear(Map3D* Map)
{
    if (Map)
        Map->Error = MAP3D_SUCCESS;
}

static void grid_free(Map3D_Grid grid, pos z_layers, pos y_rows)
{
    pos z, y;

    if (!grid)
        return;

    for (z = 0; z < z_layers; z++)
    {
        if (grid[z])
        {
            for (y = 0; y < y_rows; y++)
            {
                free(grid[z][y]);
            }
            free(grid[z]);
        }
    }

    free(grid);
}

static Map3D_Grid grid_alloc(pos x, pos y, pos z)
{
    Map3D_Grid grid;
    pos layer_idx, row_idx;

    if (x == 0 || y == 0 || z == 0)
        return NULL;
    if (!fits_size(x) || !fits_size(y) || !fits_size(z))
        return NULL;

    if ((size_t)z > SIZE_MAX / sizeof(uint8_t**))
        return NULL;
    if ((size_t)y > SIZE_MAX / sizeof(uint8_t*))
        return NULL;

    grid = (Map3D_Grid)calloc((size_t)z, sizeof(uint8_t**));
    if (!grid)
        return NULL;

    for (layer_idx = 0; layer_idx < z; layer_idx++)
    {
        grid[layer_idx] = (uint8_t**)calloc((size_t)y, sizeof(uint8_t*));
        if (!grid[layer_idx])
        {
            grid_free(grid, layer_idx, y);
            return NULL;
        }

        for (row_idx = 0; row_idx < y; row_idx++)
        {
            grid[layer_idx][row_idx] = (uint8_t*)calloc((size_t)x, sizeof(uint8_t));
            if (!grid[layer_idx][row_idx])
            {
                grid_free(grid, layer_idx + 1, row_idx);
                return NULL;
            }
        }
    }

    return grid;
}

static uint8_t* selected_byte(Map3D* Map)
{
    if (!Map || !Map->Map)
        return NULL;
    if (Map->selected_byte_z >= Map->z ||
        Map->selected_byte_y >= Map->y ||
        Map->selected_byte_x >= Map->x)
        return NULL;

    return &Map->Map[Map->selected_byte_z][Map->selected_byte_y][Map->selected_byte_x];
}

static int valid_bit(int8_t n)
{
    return n >= 0 && n <= 7;
}

Map3D* Map3D_Init(pos x, pos y, pos z)
{
    Map3D* Map;
    Map3D_Grid grid;

    grid = grid_alloc(x, y, z);
    if (!grid)
        return NULL;

    Map = (Map3D*)calloc(1, sizeof(Map3D));
    if (!Map)
    {
        grid_free(grid, z, y);
        return NULL;
    }

    Map->Map = grid;
    Map->Error = MAP3D_SUCCESS;
    Map->x = x;
    Map->y = y;
    Map->z = z;
    Map->selected_byte_x = 0;
    Map->selected_byte_y = 0;
    Map->selected_byte_z = 0;

    return Map;
}

void Map3D_Deinit(Map3D* Map)
{
    if (!Map)
        return;

    grid_free(Map->Map, Map->z, Map->y);

    Map->Map = NULL;
    Map->Error = MAP3D_SUCCESS;
    Map->x = 0;
    Map->y = 0;
    Map->z = 0;
    Map->selected_byte_x = 0;
    Map->selected_byte_y = 0;
    Map->selected_byte_z = 0;

    free(Map);
}

err Map3D_GetLastError(Map3D* Map)
{
    if (!Map)
        return MAP3D_SUCCESS;

    return Map->Error;
}

void Map3D_Resize(Map3D* Map, pos x, pos y, pos z)
{
    Map3D_Grid grid;
    pos copy_x, copy_y, copy_z;
    pos layer_idx, row_idx;

    if (!Map)
        return;

    error_clear(Map);

    if (x == Map->x && y == Map->y && z == Map->z)
        return;

    grid = grid_alloc(x, y, z);
    if (!grid)
    {
        Map->Error = MAP3D_MALLOCFAIL;
        return;
    }

    if (Map->Map)
    {
        copy_x = (x < Map->x) ? x : Map->x;
        copy_y = (y < Map->y) ? y : Map->y;
        copy_z = (z < Map->z) ? z : Map->z;

        for (layer_idx = 0; layer_idx < copy_z; layer_idx++)
        {
            for (row_idx = 0; row_idx < copy_y; row_idx++)
            {
                memcpy(grid[layer_idx][row_idx],
                       Map->Map[layer_idx][row_idx],
                       (size_t)copy_x);
            }
        }
    }

    grid_free(Map->Map, Map->z, Map->y);

    Map->Map = grid;
    Map->x = x;
    Map->y = y;
    Map->z = z;

    if (Map->selected_byte_x >= Map->x)
        Map->selected_byte_x = Map->x - 1;
    if (Map->selected_byte_y >= Map->y)
        Map->selected_byte_y = Map->y - 1;
    if (Map->selected_byte_z >= Map->z)
        Map->selected_byte_z = Map->z - 1;
}

void Map3D_Zero(Map3D* Map)
{
    pos layer_idx, row_idx;

    if (!Map || !Map->Map)
        return;

    error_clear(Map);

    for (layer_idx = 0; layer_idx < Map->z; layer_idx++)
    {
        for (row_idx = 0; row_idx < Map->y; row_idx++)
        {
            memset(Map->Map[layer_idx][row_idx], 0, (size_t)Map->x);
        }
    }
}

void Map3D_SelectByte(Map3D* Map, pos x, pos y, pos z)
{
    if (!Map)
        return;

    error_clear(Map);

    if (x >= Map->x || y >= Map->y || z >= Map->z)
        return;

    Map->selected_byte_x = x;
    Map->selected_byte_y = y;
    Map->selected_byte_z = z;
}

void Map3D_ReplaceByte(Map3D* Map, uint8_t byte)
{
    uint8_t* p = selected_byte(Map);

    error_clear(Map);

    if (p)
        *p = byte;
}

void Map3D_SetByte(Map3D* Map, uint8_t byte)
{
    uint8_t* p = selected_byte(Map);

    error_clear(Map);

    if (!p)
        return;

    if (*p != 0)
    {
        Map->Error = MAP3D_BYTENOTZERO;
        return;
    }

    *p = byte;
}

void Map3D_ZeroByte(Map3D* Map)
{
    uint8_t* p = selected_byte(Map);

    error_clear(Map);

    if (p)
        *p = 0;
}

void Map3D_GetByte(Map3D* Map, uint8_t* out)
{
    uint8_t* p = selected_byte(Map);

    error_clear(Map);

    if (!p || !out)
        return;

    *out = *p;
}

void Map3D_BitSet(Map3D* Map, int8_t n)
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
        Map->Error = MAP3D_BITNOTZERO;
        return;
    }

    *p |= mask;
}

void Map3D_BitClear(Map3D* Map, int8_t n)
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

void Map3D_BitToggle(Map3D* Map, int8_t n)
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

bool Map3D_BitGet(Map3D* Map, int8_t n)
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

int Map3D_Save(const char* filename, Map3D* map)
{
    FILE* fp = NULL;
    uint64_t header[3];
    pos z_idx, y_idx;

    if (!filename || !map || !map->Map)
        return -1;

    if (map->x > UINT64_MAX || map->y > UINT64_MAX || map->z > UINT64_MAX)
    {
        map->Error = MAP3D_OVERFLOW;
        return -1;
    }

    fp = fopen(filename, "wb");
    if (!fp)
    {
        map->Error = MAP3D_FILEERROR;
        return -1;
    }

    header[0] = (uint64_t)map->x;
    header[1] = (uint64_t)map->y;
    header[2] = (uint64_t)map->z;

    if (fwrite(header, sizeof(uint64_t), 3, fp) != 3)
    {
        map->Error = MAP3D_FILEERROR;
        fclose(fp);
        return -1;
    }

    for (z_idx = 0; z_idx < map->z; z_idx++)
    {
        if (!map->Map[z_idx])
        {
            map->Error = MAP3D_PARSEERROR;
            fclose(fp);
            return -1;
        }

        for (y_idx = 0; y_idx < map->y; y_idx++)
        {
            uint8_t* row = map->Map[z_idx][y_idx];
            if (!row)
            {
                map->Error = MAP3D_PARSEERROR;
                fclose(fp);
                return -1;
            }

            if (fwrite(row, sizeof(uint8_t), (size_t)map->x, fp) != (size_t)map->x)
            {
                map->Error = MAP3D_FILEERROR;
                fclose(fp);
                return -1;
            }
        }
    }

    fclose(fp);
    map->Error = MAP3D_SUCCESS;
    return 0;
}

Map3D* Map3D_Load(const char* filename)
{
    FILE* fp = NULL;
    uint64_t header[3];
    pos x, y, z;
    pos z_idx, y_idx;
    Map3D* map = NULL;
    int ch;

    if (!filename)
        return NULL;

    fp = fopen(filename, "rb");
    if (!fp)
        return NULL;

    if (fread(header, sizeof(uint64_t), 3, fp) != 3)
    {
        fclose(fp);
        return NULL;
    }

    x = (pos)header[0];
    y = (pos)header[1];
    z = (pos)header[2];

    map = Map3D_Init(x, y, z);
    if (!map)
    {
        fclose(fp);
        return NULL;
    }

    for (z_idx = 0; z_idx < z; z_idx++)
    {
        for (y_idx = 0; y_idx < y; y_idx++)
        {
            uint8_t* row = map->Map[z_idx][y_idx];

            if (fread(row, sizeof(uint8_t), (size_t)x, fp) != (size_t)x)
            {
                map->Error = MAP3D_PARSEERROR;
                Map3D_Deinit(map);
                fclose(fp);
                return NULL;
            }
        }
    }

    ch = fgetc(fp);
    if (ch != EOF)
    {
        map->Error = MAP3D_PARSEERROR;
        Map3D_Deinit(map);
        fclose(fp);
        return NULL;
    }

    fclose(fp);
    map->Error = MAP3D_SUCCESS;
    return map;
}
