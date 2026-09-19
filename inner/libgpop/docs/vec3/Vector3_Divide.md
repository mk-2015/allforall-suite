# Vector3_Divide

```c
err Vector3_Divide(Vector3* a, Vector3* b);
```

Divides vector `a` components by vector `b` components (`a.x = a.x / b.x`, `a.y = a.y / b.y`, `a.z = a.z / b.z`), with division by zero safety.

### Parameters
- `a`: Pointer to the destination/first operand `Vector3`.
- `b`: Pointer to the source/second operand `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if either vector is `NULL`.
- `EV3_DIVBY0` if any coordinate of `b` is `0` or if the division would overflow.
