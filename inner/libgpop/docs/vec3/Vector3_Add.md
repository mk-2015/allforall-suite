# Vector3_Add

```c
err Vector3_Add(Vector3* a, Vector3* b);
```

Adds vector `b` to vector `a` (`a = a + b`), with full overflow and underflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector3`.
- `b`: Pointer to the source/second operand `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if either vector is `NULL`.
- `EV3_OVERFLOW` if addition would cause an integer overflow/underflow.
