# Vector3_Create

```c
Vector3* Vector3_Create(int64_t x, int64_t y, int64_t z, err* Err);
```

Allocates and initializes a new 3D vector with the specified coordinates.

### Parameters
- `x`: Initial X-coordinate.
- `y`: Initial Y-coordinate.
- `z`: Initial Z-coordinate.
- `Err`: Pointer to an error code (`err`) destination. It will be populated with `EV3_SUCCESS` on success or `EV3_CRTMALLOC` if allocation fails.

### Returns
A pointer to the newly allocated `Vector3`, or `NULL` if allocation fails.
