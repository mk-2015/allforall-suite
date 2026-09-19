# Vector4_Lerp

```c
err Vector4_Lerp(Vector4* a, Vector4* b, double t, Vector4* out, err* Err);
```

Performs linear interpolation between vector `a` and vector `b` by factor `t`.

### Parameters
- `a`: Pointer to the starting `Vector4`.
- `b`: Pointer to the ending `Vector4`.
- `t`: Interpolation factor, typically between 0.0 and 1.0.
- `out`: Pointer to the `Vector4` where the interpolated components will be stored.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
`EV4_SUCCESS` on success, or an error code if calculation fails.
