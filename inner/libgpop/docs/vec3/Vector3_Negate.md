# Vector3::Negate

```cpp
err Negate();
```

Negates all components of the current vector (`this.x = -this.x`, `this.y = -this.y`, `this.z = -this.z`).

### Returns
- `EV3_SUCCESS` on success.
- `EV3_UNDERFLOW` if any component is `INT64_MIN` (which cannot be negated).
