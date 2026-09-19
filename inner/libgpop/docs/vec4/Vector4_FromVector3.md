# Vector4_FromVector3

```c
Vector4* Vector4_FromVector3(Vector3* v3, int64_t w, err* Err);
```

Creates a 4D vector from a 3D vector and a specified W-component.

### Parameters
- `v3`: Pointer to the source `Vector3`.
- `w`: W-component for the new vector.
- `Err`: Pointer to an error code (`err`) destination.

### Returns
Pointer to a newly allocated `Vector4`, or `NULL` on failure.
