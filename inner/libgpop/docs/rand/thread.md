# thread

```c
int thread(int th);
```

Sets the global threading configuration for the random number generator.

### Parameters
- `th`: The threading mode.
    - 0: Disable thread safety (potential performance boost, not thread-safe).
    - 1: Enable thread safety (uses internal locking).

### Returns
Returns 0 on success, or an error code if the threading configuration could not be set.
