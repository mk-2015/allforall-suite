# Vector4_Normalize

```c
err Vector4_Normalize(Vector4* a, Vector4* out);
```

Normalizes the vector `a` to a unit vector of length 1.

### Parameters
- `a`: Pointer to the target `Vector4`.
- `out`: Pointer to the `Vector4` where the normalized components will be stored.

### Returns
- `EV4_SUCCESS` on success.
- `EV4_NULLVEC` if any pointer is `NULL`.
- `EV4_UB` if the vector length is 0 (cannot normalize a zero vector).
