# Vector4_Add

```c
err Vector4_Add(Vector4* a, Vector4* b);
```

Adds vector `b` to vector `a` (`a = a + b`), with overflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector4`.
- `b`: Pointer to the source/second operand `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if either vector is `NULL`.
- `EV4_OVERFLOW` if addition causes an integer overflow/underflow.
