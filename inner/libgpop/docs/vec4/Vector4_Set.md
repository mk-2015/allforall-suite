# Vector4::Set

```cpp
err Set(int64_t x, int64_t y, int64_t z, int64_t w, bool xs, bool ys, bool zs, bool ws);
```

Sets coordinates of an existing vector selectively.

### Parameters
- `x`: The new X value.
- `y`: The new Y value.
- `z`: The new Z value.
- `w`: The new W value.
- `xs`: If `true`, retains current X, ignoring `x`.
- `ys`: If `true`, retains current Y, ignoring `y`.
- `zs`: If `true`, retains current Z, ignoring `z`.
- `ws`: If `true`, retains current W, ignoring `w`.

### Returns
- `EV4_SUCCESS` on success.
