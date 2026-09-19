# Vector2_Rotate

```c
err Vector2_Rotate(Vector2* a, double radians, int64_t* out_x, int64_t* out_y, err* Err);
```

Rotates vector `a` by the given angle in radians around the origin (0,0).

### Parameters
- `a`: Pointer to the target `Vector2`.
- `radians`: Angle of rotation in radians.
- `out_x`: Pointer to the integer where the rotated X component will be stored.
- `out_y`: Pointer to the integer where the rotated Y component will be stored.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
`EV2_SUCCESS` on success, or an error code if rotation fails.
