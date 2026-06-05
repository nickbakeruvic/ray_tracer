#include <stdio.h>

__global__ void cuda_hello() {
    printf("Hello World from GPU! Block: %d, Thread: %d\n", blockIdx.x, threadIdx.x);
}

int main() {
    printf("Hello World from CPU!\n\n");

    cuda_hello<<<2, 4>>>();
    cudaDeviceSynchronize();

    return 0;
}