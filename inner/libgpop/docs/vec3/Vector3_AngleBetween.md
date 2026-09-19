# Vector3_AngleBetween

```c
double Vector3_AngleBetween(Vector3* a, Vector3* b, err* Err);
```

Calculates the angle in radians between vector `a` and vector `b`.

### Parameters
- `a`: Pointer to the first `Vector3`.
- `b`: Pointer to the second `Vector3`.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
The angle between the vectors in radians as a `double`, or a negative value on error.
