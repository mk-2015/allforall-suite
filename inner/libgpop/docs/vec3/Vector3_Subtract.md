# Vector3_Subtract

```c
err Vector3_Subtract(Vector3* a, Vector3* b);
```

Subtracts vector `b` from vector `a` (`a = a - b`), with full overflow and underflow protection.

### Parameters
- `a`: Pointer to the destination/first operand `Vector3`.
- `b`: Pointer to the source/second operand `Vector3`.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if either vector is `NULL`.
- `EV3_UNDERFLOW` if subtraction would cause an integer overflow/underflow.
