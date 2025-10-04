#include <dlfcn.h>
#include <stdio.h>

int main(void) {
    printf("Loading start...\n");
    auto preload = dlopen("/home/msojocs/github/Yui-native/build/libpreload.so", RTLD_NOW);
    if (!preload) {
        fprintf(stderr, "Failed to load so: %s\n", dlerror());
        return 1;
    }
    typedef int (*gnutls_global_init_fn_t)(void);
    typedef void (*gnutls_global_deinit_fn_t)(void);

    gnutls_global_init_fn_t gnutls_global_init =
        (gnutls_global_init_fn_t)dlsym(preload, "gnutls_global_init");
    if (!gnutls_global_init) {
        fprintf(stderr, "Failed to find gnutls_global_init: %s\n", dlerror());
        return 1;
    }

    gnutls_global_deinit_fn_t gnutls_global_deinit =
        (gnutls_global_deinit_fn_t)dlsym(preload, "gnutls_global_deinit");
    if (!gnutls_global_deinit) {
        fprintf(stderr, "Failed to find gnutls_global_deinit: %s\n", dlerror());
        return 1;
    }

    if (gnutls_global_init() != 0) {
        fprintf(stderr, "gnutls_global_init failed\n");
        return 1;
    }

    // 寻找gnutls_free符号
    typedef void* (*gnutls_calloc_fn_t)(size_t, size_t);
    void *gnutls_calloc_symbol = dlsym(preload, "gnutls_calloc");
    if (!gnutls_calloc_symbol) {
        fprintf(stderr, "Failed to find gnutls_calloc: %s\n", dlerror());
        return 1;
    }
    gnutls_calloc_fn_t gnutls_calloc = *(gnutls_calloc_fn_t *)gnutls_calloc_symbol;
    if (!gnutls_calloc) {
        fprintf(stderr, "gnutls_calloc symbol is null\n");
        return 1;
    }
    printf("Found gnutls_calloc at %p\n", (void*)gnutls_calloc);
    // 调用gnutls_calloc分配内存
    void *ptr = gnutls_calloc(10, 20);
    if (!ptr) {
        fprintf(stderr, "gnutls_calloc failed\n");
        return 1;
    }
    printf("gnutls_calloc allocated memory at %p\n", ptr);
    typedef void (*gnutls_free_fn_t)(void *);
    void *gnutls_free_symbol = dlsym(preload, "gnutls_free");
    if (!gnutls_free_symbol) {
        fprintf(stderr, "Failed to find gnutls_free: %s\n", dlerror());
        return 1;
    }
    gnutls_free_fn_t gnutls_free = *(gnutls_free_fn_t *)gnutls_free_symbol;
    if (!gnutls_free) {
        fprintf(stderr, "gnutls_free symbol is null\n");
        return 1;
    }
    printf("free\n");
    // 调用gnutls_free
    gnutls_free(ptr);

    gnutls_global_deinit();
    return 0;
}