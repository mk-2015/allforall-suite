# Vector2_Multiply

```c
err Vector2_Multiply(Vector2* a, Vector2* b);
```

Multiplies vector `a` components by vector `b` components (`a.x = a.x * b.x`, `a.y = a.y * b.y`), with integer overflow safety.

### Parameters
- `a`: Pointer to the destination/first operand `Vector2`.
- `b`: Pointer to the source/second operand `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_NULLVEC` if either vector is `NULL`.
- `EV2_OVERFLOW` if the multiplication causes an overflow or underflow.
