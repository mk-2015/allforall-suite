# Vector3_LengthSquared

```c
double Vector3_LengthSquared(Vector3* a, err* Err);
```

Calculates the squared geometric length (magnitude squared) of the vector `a`. This function is more performant than `Vector3_Length` as it avoids a square root calculation.

### Parameters
- `a`: Pointer to the `Vector3`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV3_SUCCESS` on success, `EV3_NULLVEC` if `a` is `NULL`, or `EV3_OVERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The squared length of the vector as a `double`, or a negative value on error.
