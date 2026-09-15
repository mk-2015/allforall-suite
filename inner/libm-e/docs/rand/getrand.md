# getrand

```c
RANDOM_OUT getrand(randalg_t *randalg);
```

Retrieves the current random number from the generator.

### Parameters
- `randalg`: Pointer to the initialized `randalg_t` structure.

### Returns
Returns the `RANDOM_OUT` value computed during the last call to `genrand`. If the generator was not initialized correctly, this may return an invalid value; check `lasterr()` if unexpected results occur.
