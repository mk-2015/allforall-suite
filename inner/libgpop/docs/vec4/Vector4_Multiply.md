# Vector4::Multiply

```cpp
err Multiply(const Vector4& b);
```

Multiplies current vector components by vector `b` components (`this.x *= b.x`, `this.y *= b.y`, `this.z *= b.z`, `this.w *= b.w`), with integer overflow safety.

### Parameters
- `b`: Const reference to the source `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_OVERFLOW` if multiplication causes an overflow or underflow.
