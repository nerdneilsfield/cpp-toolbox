#include <stdio.h>

#ifdef __GLIBC__
    #include <gnu/libc-version.h>
    
    int main() {
        printf("glibc version: %s\n", gnu_get_libc_version());
        printf("glibc compile-time version: %d.%d\n", __GLIBC__, __GLIBC_MINOR__);
        return 0;
    }
#elif defined(__MUSL__)
    int main() {
        printf("musl libc detected\n");
        // musl doesn't provide an API to get its version
        return 0;
    }
#else
    int main() {
        printf("Unknown libc implementation\n");
        return 0;
    }
#endif