# Vector3::Multiply

```cpp
err Multiply(const Vector3& b);
```

Multiplies current vector components by vector `b` components (`this.x *= b.x`, `this.y *= b.y`, `this.z *= b.z`), with integer overflow safety.

### Parameters
- `b`: Const reference to the source `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_OVERFLOW` if multiplication causes an overflow or underflow.
