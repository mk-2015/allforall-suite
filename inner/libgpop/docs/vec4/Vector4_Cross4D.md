# Vector4_Cross4D

```c
err Vector4_Cross4D(Vector4* a, Vector4* b, Vector4* c, Vector4* out);
```

Calculates the 4D cross product (wedge product of three vectors) and stores the result in `out`.

### Parameters
- `a`, `b`, `c`: Pointers to the input `Vector4` operands.
- `out`: Pointer to the `Vector4` where the result will be stored.

### Returns
`EV4_SUCCESS` on success, or `EV4_NULLVEC` if any pointer is `NULL`.
