# Vector3_Length

```c
double Vector3_Length(Vector3* a, err* Err);
```

Calculates the geometric length (magnitude) of the vector `a`.

### Parameters
- `a`: Pointer to the `Vector3`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV3_SUCCESS` on success, `EV3_NULLVEC` if `a` is `NULL`, or `EV3_UNDERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The length of the vector as a `double`, or a negative value on error.
