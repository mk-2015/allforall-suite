# Vector3_Distance

```c
double Vector3_Distance(Vector3* a, Vector3* b, err* Err);
```

Calculates the Euclidean distance between vector `a` and vector `b` in 3D space.

### Parameters
- `a`: Pointer to the first `Vector3`.
- `b`: Pointer to the second `Vector3`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV3_SUCCESS` on success, `EV3_NULLVEC` if either vector is `NULL`, or `EV3_UB` if calculation steps overflow or hit undefined boundaries.

### Returns
The distance between the two vectors as a `double`, or a negative value on error.
