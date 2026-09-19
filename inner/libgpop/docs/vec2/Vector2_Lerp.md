# Vector2_Lerp

```c
err Vector2_Lerp(Vector2* a, Vector2* b, double t, int64_t* out_x, int64_t* out_y, err* Err);
```

Performs linear interpolation between vector `a` and vector `b` by factor `t`.

### Parameters
- `a`: Pointer to the starting `Vector2`.
- `b`: Pointer to the ending `Vector2`.
- `t`: Interpolation factor, typically between 0.0 and 1.0.
- `out_x`: Pointer to the integer where the interpolated X component will be stored.
- `out_y`: Pointer to the integer where the interpolated Y component will be stored.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
`EV2_SUCCESS` on success, or an error code if calculation fails.
