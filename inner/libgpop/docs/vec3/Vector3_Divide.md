# Vector3::Divide

```cpp
err Divide(const Vector3& b);
```

Divides current vector components by vector `b` components (`this.x /= b.x`, `this.y /= b.y`, `this.z /= b.z`), with division by zero safety.

### Parameters
- `b`: Const reference to the source `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_DIVBY0` if any coordinate of `b` is `0`.
