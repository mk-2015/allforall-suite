# Vector4_DistanceSquared

```c
double Vector4_DistanceSquared(Vector4* a, Vector4* b, err* Err);
```

Calculates the squared Euclidean distance between vector `a` and vector `b` in 4D space.

### Parameters
- `a`: Pointer to the first `Vector4`.
- `b`: Pointer to the second `Vector4`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV4_SUCCESS` on success, `EV4_NULLVEC` if either vector is `NULL`, or `EV4_UB` if calculation steps overflow or hit undefined boundaries.

### Returns
The squared distance between the two vectors as a `double`, or a negative value on error.
