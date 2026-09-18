# Vector2_Subtract

```c
err Vector2_Subtract(Vector2* a, Vector2* b);
```

Subtracts vector `b` from vector `a` (`a = a - b`), with full overflow and underflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector2`.
- `b`: Pointer to the source/second operand `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_NULLVEC` if either vector is `NULL`.
- `EV2_UNDERFLOW` if subtraction would cause an integer overflow/underflow.
