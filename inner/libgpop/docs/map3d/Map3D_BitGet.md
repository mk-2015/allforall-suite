# Map3D_BitGet

```c
bool Map3D_BitGet(Map3D* Map, int8_t n);
```

Checks the state of the n-th bit (0-7) of the currently selected byte.

### Parameters
- `Map`: Pointer to the `Map3D` structure.
- `n`: Bit index (0-7).

### Returns
`true` if the bit is set, `false` otherwise.
