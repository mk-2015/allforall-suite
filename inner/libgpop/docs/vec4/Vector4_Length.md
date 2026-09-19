# Vector4_Length

```c
double Vector4_Length(Vector4* a, err* Err);
```

Calculates the geometric length (magnitude) of the vector `a`.

### Parameters
- `a`: Pointer to the `Vector4`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV4_SUCCESS` on success, `EV4_NULLVEC` if `a` is `NULL`, or `EV4_UNDERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The length of the vector as a `double`, or a negative value on error.
