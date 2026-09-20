# Vector4::Subtract

```cpp
err Subtract(const Vector4& b);
```

Subtracts vector `b` from the current vector instance (`this = this - b`), with full overflow and underflow protection.

### Parameters
- `b`: Const reference to the source `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_UNDERFLOW` if subtraction causes an integer overflow/underflow.
