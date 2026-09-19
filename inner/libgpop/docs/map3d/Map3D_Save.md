# Map3D_Save

```c
int Map3D_Save(const char* filename, Map3D* map);
```

Saves the 3D map data to a file.

### Parameters
- `filename`: Path to the file where data should be saved.
- `map`: Pointer to the `Map3D` structure to save.

### Returns
Returns `0` on success, or a non-zero error code if saving fails.
