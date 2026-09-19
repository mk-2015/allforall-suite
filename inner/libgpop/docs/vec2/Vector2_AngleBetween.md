# Vector2_AngleBetween

```c
double Vector2_AngleBetween(Vector2* a, Vector2* b, err* Err);
```

Calculates the angle in radians between vector `a` and vector `b`.

### Parameters
- `a`: Pointer to the first `Vector2`.
- `b`: Pointer to the second `Vector2`.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
The angle between the vectors in radians as a `double`, or a negative value on error.
