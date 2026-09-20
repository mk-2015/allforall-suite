# Vector4::Divide

```cpp
err Divide(const Vector4& b);
```

Divides current vector components by vector `b` components (`this.x /= b.x`, `this.y /= b.y`, `this.z /= b.z`, `this.w /= b.w`), with division by zero safety.

### Parameters
- `b`: Const reference to the source `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_DIVBY0` if any coordinate of `b` is `0`.
