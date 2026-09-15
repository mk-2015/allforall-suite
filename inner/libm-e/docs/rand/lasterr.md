# lasterr

```c
int lasterr();
```

Retrieves the error code from the last operation.

### Returns
Returns the integer error code of the last failed operation. See `randalg.h` for `OPEN_RAND_*`, `GENRAND_*`, etc., error code definitions. Returns 0 if the last operation was successful.
