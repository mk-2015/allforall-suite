# Vector2::Add

```cpp
err Add(const Vector2& b);
```

Adds vector `b` to the current vector instance (`this = this + b`), with full overflow and underflow protection.

### Parameters
- `b`: Const reference to the source `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_OVERFLOW` if addition would cause an integer overflow/underflow.
