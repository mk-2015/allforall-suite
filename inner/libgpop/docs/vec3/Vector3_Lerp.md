# Vector3_Lerp

```c
err Vector3_Lerp(Vector3* a, Vector3* b, double t, Vector3* out, err* Err);
```

Performs linear interpolation between vector `a` and vector `b` by factor `t`.

### Parameters
- `a`: Pointer to the starting `Vector3`.
- `b`: Pointer to the ending `Vector3`.
- `t`: Interpolation factor, typically between 0.0 and 1.0.
- `out`: Pointer to the `Vector3` where the interpolated components will be stored.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
`EV3_SUCCESS` on success, or an error code if calculation fails.
