# Vector3_Rotate

```c
err Vector3_Rotate(Vector3* a, double radians, Vector3* out, err* Err);
```

Rotates vector `a` by the given angle in radians.

### Parameters
- `a`: Pointer to the target `Vector3`.
- `radians`: Angle of rotation in radians.
- `out`: Pointer to the `Vector3` where the rotated components will be stored.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
`EV3_SUCCESS` on success, or an error code if rotation fails.
