# Vector2::Multiply

```cpp
err Multiply(const Vector2& b);
```

Multiplies current vector components by vector `b` components (`this.x = this.x * b.x`, `this.y = this.y * b.y`), with integer overflow safety.

### Parameters
- `b`: Const reference to the source `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_OVERFLOW` if the multiplication causes an overflow or underflow.
