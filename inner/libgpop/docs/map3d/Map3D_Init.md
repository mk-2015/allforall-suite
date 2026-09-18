# Map3D_Init

```c
Map3D* Map3D_Init(pos x, pos y, pos z);
```

Initializes a new 3D Map grid of dimensions x by y by z.

### Parameters
- `x`: Number of columns.
- `y`: Number of rows.
- `z`: Number of layers.

### Returns
Returns a pointer to the initialized `Map3D` structure, or `NULL` if initialization fails (e.g., due to memory allocation failure).
