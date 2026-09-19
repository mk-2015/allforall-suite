# Vector3_Dot

```c
double Vector3_Dot(Vector3* a, Vector3* b, err* Err);
```

Calculates the dot product of vector `a` and vector `b` (`a.x * b.x + a.y * b.y + a.z * b.z`).

### Parameters
- `a`: Pointer to the first `Vector3`.
- `b`: Pointer to the second `Vector3`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV3_SUCCESS` on success, `EV3_NULLVEC` if either vector is `NULL`, or `EV3_UB` if calculations hit overflow or undefined boundaries.

### Returns
The dot product as a `double`, or a negative error code representation.
