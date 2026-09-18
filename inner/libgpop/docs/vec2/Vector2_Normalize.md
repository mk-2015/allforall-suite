# Vector2_Normalize

```c
err Vector2_Normalize(Vector2* a, int64_t* out_x, int64_t* out_y);
```

Normalizes the vector `a` to a unit vector of length 1, scaled by `EV2_NORM_SCALE` (1000).

### Parameters
- `a`: Pointer to the target `Vector2`.
- `out_x`: Pointer to the integer where the normalized X component (scaled by 1000) will be stored.
- `out_y`: Pointer to the integer where the normalized Y component (scaled by 1000) will be stored.

### Returns
- `EV2_SUCCESS` on success.
- `-1` if the vector `a` or destination pointers are `NULL`.
- `-2` if coordinates contains `INT64_MIN` (causing undefined behavior `EV2_UB`).
- `-3` if the length of the vector is 0 (cannot normalize a zero vector).
