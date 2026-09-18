# Map3D_SetByte

```c
void Map3D_SetByte(Map3D* Map, uint8_t byte);
```

Sets the currently selected byte to the provided `byte` value, *if the current byte is zero*.

### Parameters
- `Map`: Pointer to the `Map3D` structure.
- `byte`: The new byte value.

### Errors
If the selected byte is not zero, the error state is set to `MAP3D_BYTENOTZERO`.
