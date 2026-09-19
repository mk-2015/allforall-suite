# Vector4_Create

```c
Vector4* Vector4_Create(int64_t x, int64_t y, int64_t z, int64_t w, err* Err);
```

Allocates and initializes a new 4D vector with the specified coordinates.

### Parameters
- `x`, `y`, `z`, `w`: Initial coordinates.
- `Err`: Pointer to an error code (`err`) destination. It will be populated with `EV4_SUCCESS` on success or `EV4_CRTMALLOC` if allocation fails.

### Returns
A pointer to the newly allocated `Vector4`, or `NULL` if allocation fails.
