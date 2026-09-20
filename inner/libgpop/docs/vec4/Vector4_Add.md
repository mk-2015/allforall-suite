# Vector4::Add

```cpp
err Add(const Vector4& b);
```

Adds vector `b` to the current vector instance (`this = this + b`), with full overflow and underflow protection.

### Parameters
- `b`: Const reference to the source `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_OVERFLOW` if addition causes an integer overflow/underflow.
