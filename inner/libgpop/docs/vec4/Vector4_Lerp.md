# Vector4::Lerp

```cpp
err Lerp(const Vector4& b, double t, Vector4& out, err* Err) const;
```

Linearly interpolates between this vector and `b`.

### Parameters
- `b`: Target `Vector4`.
- `t`: Factor (0.0 to 1.0).
- `out`: Destination `Vector4`.
- `Err`: Pointer to error code.

### Returns
- `EV4_SUCCESS` on success.
