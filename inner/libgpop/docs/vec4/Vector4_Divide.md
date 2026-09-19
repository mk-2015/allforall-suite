# Vector4_Divide

```c
err Vector4_Divide(Vector4* a, Vector4* b);
```

Divides vector `a` components by vector `b` components (`a = a / b`), with division by zero safety.

### Parameters
- `a`: Pointer to the destination/first operand `Vector4`.
- `b`: Pointer to the source/second operand `Vector4`.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if either vector is `NULL`.
- `EV4_DIVBY0` if any coordinate of `b` is `0` or division would overflow.
