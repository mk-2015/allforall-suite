# Vector3::Set

```cpp
err Set(int64_t x, int64_t y, int64_t z, bool xs, bool ys, bool zs);
```

Sets coordinates of an existing vector selectively.

### Parameters
- `x`: The new X value.
- `y`: The new Y value.
- `z`: The new Z value.
- `xs`: If `true`, retains the current X value, ignoring `x`.
- `ys`: If `true`, retains the current Y value, ignoring `y`.
- `zs`: If `true`, retains the current Z value, ignoring `z`.

### Returns
- `EV3_SUCCESS` on success.
