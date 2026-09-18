# Map3D_Resize

```c
void Map3D_Resize(Map3D* Map, pos x, pos y, pos z);
```

Resizes the existing 3D Map grid to the new dimensions. If the new dimensions are smaller, the data is truncated.

### Parameters
- `Map`: Pointer to the `Map3D` structure.
- `x`: New number of columns.
- `y`: New number of rows.
- `z`: New number of layers.

### Errors
If resizing fails (e.g., memory allocation error), the map's error state will be set to `MAP3D_MALLOCFAIL`.
