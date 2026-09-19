# Vector4_Multiply

```c
err Vector4_Multiply(Vector4* a, Vector4* b);
```

Multiplies vector `a` components by vector `b` components (`a = a * b`), with overflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector4`.
- `b`: Pointer to the source/second operand `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if either vector is `NULL`.
- `EV4_OVERFLOW` if multiplication causes an overflow or underflow.
