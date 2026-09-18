# Vector2_Add

```c
err Vector2_Add(Vector2* a, Vector2* b);
```

Adds vector `b` to vector `a` (`a = a + b`), with full overflow and underflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector2`.
- `b`: Pointer to the source/second operand `Vector2`.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_NULLVEC` if either vector is `NULL`.
- `EV2_OVERFLOW` if addition would cause an integer overflow/underflow.
