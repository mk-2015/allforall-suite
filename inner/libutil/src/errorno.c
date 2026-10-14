int errorno = 0;

#if defined(__STDC_NO_THREADS__)
    // Where are my threads?
    #error "I NEED THREADS! C11 or newer required."
#else
    #include <threads.h>
#endif