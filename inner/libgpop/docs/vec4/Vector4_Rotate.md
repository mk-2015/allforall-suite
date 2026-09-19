# Vector4_Rotate

```c
err Vector4_Rotate(Vector4* a, double radians, Vector4* out, err* Err);
```

Rotates vector `a` by the given angle in radians.

### Parameters
- `a`: Pointer to the target `Vector4`.
- `radians`: Angle of rotation in radians.
- `out`: Pointer to the `Vector4` where the rotated components will be stored.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
`EV4_SUCCESS` on success, or an error code if rotation fails.
