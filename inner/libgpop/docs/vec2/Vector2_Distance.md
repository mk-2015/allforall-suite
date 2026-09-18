# Vector2_Distance

```c
double Vector2_Distance(Vector2* a, Vector2* b, err* Err);
```

Calculates the Euclidean distance between vector `a` and vector `b`.

### Parameters
- `a`: Pointer to the first `Vector2`.
- `b`: Pointer to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success, `EV2_NULLVEC` if either vector is `NULL`, or `EV2_UB` if calculation steps overflow or hit undefined boundaries.

### Returns
The distance between the two vectors as a `double`, or a negative value on error.
