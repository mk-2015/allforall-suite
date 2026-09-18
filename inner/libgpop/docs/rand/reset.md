# reset

```c
void reset(randalg_t *randalg);
```

Resets the generator state to its initial configuration.

### Parameters
- `randalg`: Pointer to the `randalg_t` structure to reset.

### Note
***MAKE SURE TO CALL [setopt](./setopt.md) AFTER THIS FUNCTION HAS BEEN CALLED***

Calling `reset()` clears the internal generator state but does not re-apply the algorithmic configuration. `setopt()` must be called subsequently to safely reconfigure the generator for further use.
