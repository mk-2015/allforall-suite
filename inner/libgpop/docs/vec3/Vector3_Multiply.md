# Vector3_Multiply

```c
err Vector3_Multiply(Vector3* a, Vector3* b);
```

Multiplies vector `a` components by vector `b` components (`a.x = a.x * b.x`, `a.y = a.y * b.y`, `a.z = a.z * b.z`), with integer overflow safety.

### Parameters
- `a`: Pointer to the destination/first operand `Vector3`.
- `b`: Pointer to the source/second operand `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if either vector is `NULL`.
- `EV3_OVERFLOW` if the multiplication causes an overflow or underflow.
