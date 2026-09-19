# Vector3_Normalize

```c
err Vector3_Normalize(Vector3* a, Vector3* out);
```

Normalizes the vector `a` to a unit vector of length 1.

### Parameters
- `a`: Pointer to the target `Vector3`.
- `out`: Pointer to the `Vector3` where the normalized components will be stored.

### Returns
- `EV3_SUCCESS` on success.
- `EV3_NULLVEC` if any pointer is `NULL`.
- `EV3_UB` if the vector length is 0 (cannot normalize a zero vector).
