# Vector3_Cross

```c
err Vector3_Cross(Vector3* a, Vector3* b, Vector3* out);
```

Calculates the cross product of vector `a` and vector `b` and stores the result in `out`.

### Parameters
- `a`: Pointer to the first `Vector3`.
- `b`: Pointer to the second `Vector3`.
- `out`: Pointer to the `Vector3` where the result will be stored.

### Returns
`EV3_SUCCESS` on success, or `EV3_NULLVEC` if any pointer is `NULL`.
