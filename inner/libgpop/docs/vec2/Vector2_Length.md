# Vector2_Length

```c
double Vector2_Length(Vector2* a, err* Err);
```

Calculates the geometric length (magnitude) of the vector `a`.

### Parameters
- `a`: Pointer to the `Vector2`.
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success, `EV2_NULLVEC` if `a` is `NULL`, or `EV2_UNDERFLOW` if any coordinate contains `INT64_MIN`.

### Returns
The length of the vector as a `double`, or a negative value on error.
