# Vector3_FromVector2

```c
Vector3* Vector3_FromVector2(Vector2* v2, int64_t z, err* Err);
```

Creates a 3D vector from a 2D vector and a specified Z-component.

### Parameters
- `v2`: Pointer to the source `Vector2`.
- `z`: Z-component for the new vector.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
Pointer to a newly allocated `Vector3`, or `NULL` on failure.
