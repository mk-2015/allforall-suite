# Map2D_SaveCSV

```c
err Map2D_SaveCSV(Map2D* Map, const char* filename);
```

Saves the current `Map2D` grid to a CSV file.

### Parameters
- `Map`: Pointer to the `Map2D` structure.
- `filename`: Path to the file where the grid should be saved.

### Returns
Returns `MAP2D_SUCCESS` (0) on success, or an error code on failure.
