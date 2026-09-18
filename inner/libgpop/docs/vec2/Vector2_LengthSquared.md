# Vector2_LengthSquared

```c
double Vector2_LengthSquared(Vector2* a, err* Err);
```

Calculates the squared geometric length (magnitude squared) of the vector `a`. This function is more performant than `Vector2_Length` as it avoids a square root calculation.

### Parameters
- `a`: Pointer to the `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success, `EV2_NULLVEC` if `a` is `NULL`, or `EV2_OVERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The squared length of the vector as a `double`, or a negative value on error.
