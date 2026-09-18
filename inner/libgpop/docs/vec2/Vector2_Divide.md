# Vector2_Divide

```c
err Vector2_Divide(Vector2* a, Vector2* b);
```

Divides vector `a` components by vector `b` components (`a.x = a.x / b.x`, `a.y = a.y / b.y`), with division by zero safety.

### Parameters
- `a`: Pointer to the destination/first operand `Vector2`.
- `b`: Pointer to the source/second operand `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_NULLVEC` if either vector is `NULL`.
- `EV2_DIVBY0` if either coordinate of `b` is `0` or if the division would overflow.
