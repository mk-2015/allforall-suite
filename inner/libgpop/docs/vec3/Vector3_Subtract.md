# Vector3::Subtract

```cpp
err Subtract(const Vector3& b);
```

Subtracts vector `b` from the current vector instance (`this = this - b`), with full overflow and underflow protection.

### Parameters
- `b`: Const reference to the source `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_UNDERFLOW` if subtraction causes an integer overflow/underflow.
