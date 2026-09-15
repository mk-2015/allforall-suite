# close_random

```c
void close_random(randalg_t *randalg);
```

Cleans up resources associated with the random number generator.

### Parameters
- `randalg`: Pointer to the `randalg_t` structure to close and deallocate.

### Behavior
This function releases memory and destroys the mutex lock associated with the generator instance. The `randalg` pointer should not be used after calling this function.
