#include <x86intrin.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

int main() {
    uint64_t t1 = __rdtsc();
    int val = 0;
    for(short i = 0; i < 10000; i++) {

        val += i;

    }
    printf("Delta TSC: %lu\n", __rdtsc() - t1);
    return 0;
}
