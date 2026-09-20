# Vector3::Add

```cpp
err Add(const Vector3& b);
```

Adds vector `b` to the current vector instance (`this = this + b`), with full overflow and underflow protection.

### Parameters
- `b`: Const reference to the source `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_OVERFLOW` if addition causes an integer overflow/underflow.
