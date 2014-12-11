#include <stdio.h>
#include <sys/object_pool.h>

int main()
{
    uint32_t i;
    sys::CRawObjectPool rbp;
    uint32_t pool_size = 1000; 
    int** intptr_array = new int*[pool_size]; 
    
    rbp.create(sizeof(int), pool_size);

    for (i=0; i<pool_size; ++i)
    {
        intptr_array[i] = (int *)rbp.allocate();
        int* x = intptr_array[i];
        *x = (int)(i+1);
        printf("==>%d\n", *x);
    }
    for (i=0; i<pool_size; ++i)
    {
        rbp.reclaim(intptr_array[i]);
    }
    
    int* p = (int *)rbp.allocate();
    *p = 8899;
    printf("*p == %d\n", *p);
    rbp.reclaim(p);

    rbp.destroy();
}
