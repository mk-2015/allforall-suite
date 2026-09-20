# Vector2::Normalize

```cpp
err Normalize(int64_t* out_x, int64_t* out_y) const;
```

Normalizes the current vector to a unit vector of length 1, scaled by `EV2_NORM_SCALE` (1000).

### Parameters
- `out_x`: Pointer to the integer where the normalized X component (scaled by 1000) will be stored.
- `out_y`: Pointer to the integer where the normalized Y component (scaled by 1000) will be stored.

### Returns
- `EV2_SUCCESS` on success.
- `EV2_UB` if coordinates contains `INT64_MIN` or if the length of the vector is 0 (cannot normalize a zero vector).
