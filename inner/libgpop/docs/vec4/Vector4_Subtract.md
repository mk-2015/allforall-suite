# Vector4_Subtract

```c
err Vector4_Subtract(Vector4* a, Vector4* b);
```

Subtracts vector `b` from vector `a` (`a = a - b`), with overflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector4`.
- `b`: Pointer to the source/second operand `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if either vector is `NULL`.
- `EV4_UNDERFLOW` if subtraction causes an integer overflow/underflow.
