#ifndef DEBUG_MALLOC_H
#define DEBUG_MALLOC_H

int alloc_count = 0;
int free_count = 0;
void* debug_malloc(size_t size) {
    void* ptr = malloc(size);
    alloc_count++;
    return ptr;
}

void debug_free(void* ptr) {
    free(ptr);
    free_count++;
}

#define malloc debug_malloc 
#define free debug_free 

#endif /* DEBUG_MALLOC_H */
