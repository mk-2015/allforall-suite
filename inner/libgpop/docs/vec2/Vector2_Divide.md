# Vector2::Divide

```cpp
err Divide(const Vector2& b);
```

Divides current vector components by vector `b` components (`this.x = this.x / b.x`, `this.y = this.y / b.y`), with division by zero safety.

### Parameters
- `b`: Const reference to the source `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_DIVBY0` if either coordinate of `b` is `0` or if the division would overflow.
