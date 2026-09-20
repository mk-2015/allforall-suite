# Vector3::Lerp

```cpp
err Lerp(const Vector3& b, double t, Vector3& out, err* Err) const;
```

Linearly interpolates between this vector and `b`.

### Parameters
- `b`: Target `Vector3`.
- `t`: Factor (0.0 to 1.0).
- `out`: Destination `Vector3`.
- `Err`: Pointer to error code.

### Returns
- `EV3_SUCCESS` on success.
