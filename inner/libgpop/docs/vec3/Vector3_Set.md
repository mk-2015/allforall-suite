# Vector3_Set

```c
err Vector3_Set(Vector3* Vector, int64_t x, int64_t y, int64_t z, bool xs, bool ys, bool zs);
```

Sets coordinates of an existing vector selectively.

### Parameters
- `Vector`: Pointer to the target `Vector3`.
- `x`: The new X value.
- `y`: The new Y value.
- `z`: The new Z value.
- `xs`: If `true`, retains the current X value.
- `ys`: If `true`, retains the current Y value.
- `zs`: If `true`, retains the current Z value.

### Returns
`EV3_SUCCESS` on success, or `EV3_NULLVEC` if the input vector is `NULL`.
