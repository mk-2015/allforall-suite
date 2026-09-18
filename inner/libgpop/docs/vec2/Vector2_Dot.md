# Vector2_Dot

```c
double Vector2_Dot(Vector2* a, Vector2* b, err* Err);
```

Calculates the dot product of vector `a` and vector `b` (`a.x * b.x + a.y * b.y`).

### Parameters
- `a`: Pointer to the first `Vector2`.
- `b`: Pointer to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success, `EV2_NULLVEC` if either vector is `NULL`, or `EV2_UB` if calculations hit overflow or undefined boundaries.

### Returns
The dot product as a `double`, or a negative error code representation.
