# Vector2_Create

```c
Vector2* Vector2_Create(int64_t x, int64_t y, err* Err);
```

Allocates and initializes a new 2D vector with the specified coordinates.

### Parameters
- `x`: Initial X-coordinate.
- `y`: Initial Y-coordinate.
- `Err`: Pointer to an error code (`err`) destination. It will be populated with `EV2_SUCCESS` on success or `EV2_CRTMALLOC` if allocation fails.

### Returns
A pointer to the newly allocated `Vector2`, or `NULL` if allocation fails.
