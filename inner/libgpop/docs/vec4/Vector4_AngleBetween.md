# Vector4_AngleBetween

```c
double Vector4_AngleBetween(Vector4* a, Vector4* b, err* Err);
```

Calculates the angle in radians between vector `a` and vector `b`.

### Parameters
- `a`: Pointer to the first `Vector4`.
- `b`: Pointer to the second `Vector4`.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
The angle between the vectors in radians as a `double`, or a negative value on error.
