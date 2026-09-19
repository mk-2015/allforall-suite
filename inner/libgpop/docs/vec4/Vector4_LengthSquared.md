# Vector4_LengthSquared

```c
double Vector4_LengthSquared(Vector4* a, err* Err);
```

Calculates the squared geometric length (magnitude squared) of the vector `a`.

### Parameters
- `a`: Pointer to the `Vector4`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV4_SUCCESS` on success, `EV4_NULLVEC` if `a` is `NULL`, or `EV4_OVERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The squared length of the vector as a `double`, or a negative value on error.
