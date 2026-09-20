# Vector2::Lerp

```cpp
Vector2 Lerp(const Vector2& b, double t, err* Err) const;
```

Linearly interpolates between the current vector and vector `b` by factor `t` (0.0 to 1.0).

### Parameters
- `b`: Const reference to the target `Vector2`.
- `t`: Interpolation factor (0.0 returns this, 1.0 returns `b`).
- `Err`: Pointer to an error code (`err`) destination. It will be set to `EV2_SUCCESS` on success or an error code on failure.

### Returns
The interpolated `Vector2`.
