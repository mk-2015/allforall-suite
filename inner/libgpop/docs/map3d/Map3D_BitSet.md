# Map3D_BitSet

```c
void Map3D_BitSet(Map3D* Map, int8_t n);
```

Sets the n-th bit (0-7) of the currently selected byte, *if the bit is not already set*.

### Parameters
- `Map`: Pointer to the `Map3D` structure.
- `n`: Bit index (0-7).

### Errors
If the bit is already set, the error state is set to `MAP3D_BITNOTZERO`.
