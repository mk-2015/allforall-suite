# Vector2::Subtract

```cpp
err Subtract(const Vector2& b);
```

Subtracts vector `b` from the current vector instance (`this = this - b`), with full overflow and underflow protection.

### Parameters
- `b`: Const reference to the source `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_UNDERFLOW` if subtraction would cause an integer overflow/underflow.
