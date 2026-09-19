# Vector2_Cross

```c
double Vector2_Cross(Vector2* a, Vector2* b, err* Err);
```

Calculates the 2D cross product of vector `a` and vector `b` (`a.x * b.y - a.y * b.x`).

### Parameters
- `a`: Pointer to the first `Vector2`.
- `b`: Pointer to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success, `EV2_NULLVEC` if either vector is `NULL`, or `EV2_UB` if calculation steps overflow or hit undefined boundaries.

### Returns
The 2D cross product as a `double`, or a negative error code representation.
