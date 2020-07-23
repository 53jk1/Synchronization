// > gcc -Wall -Wextra starv.c -O3 -o starv
// > starv

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <windows.h>

#define THREAD_COUNT 8
#define MAX_COUNTER 1000

static volatile LONG g_spinlock;
static int g_counter[THREAD_COUNT];
static uint64_t g_max_wait[THREAD_COUNT];
static uint64_t g_min_wait[THREAD_COUNT];
static uint64_t g_total[THREAD_COUNT];
static uint32_t g_round_time[THREAD_COUNT][MAX_COUNTER];
static volatile int g_finish;
static volatile LONG g_poor_mans_lock;

inline void my_spinlock_lock(LONG volatile *lock) {
    while(InterlockedExchange(lock, 1) == 1);
}

inline void my_spinlock_unlock(LONG volatile *lock) {
    InterlockedExchange(lock, 0);
}

inline uint64_t rdtsc() {
    uint64_t timestamp;
}

    // I use the RDTSCP instruction instead of RDTSC
    // because the first one is a serialization instruction,
    // i.e. it waits for all previous instructions to be
    // executed - RDTSC could be executed out of order,
    // which would result in an incorrect result.

    asm volatile ("rdtscp"
        : "=A" (timestamp) // EDX:EAX are to be written to
                            // the timestamp variable.
        :                   // No input arguments.
        : "ecx");           // The contents of the ECX registry
                            // are overwritten (if the compiler
                            // considers the ECX value to be
                            // significant).

    return timestamp;
}

DWORD WINAPI BruteTest(LPVOID param) {
    uint64_t t_start, t_end, t_final;
    uint64_t t_total_start, t_total_end;
    int thread_id = (int)param;
    g_min_wait[thread_id] = ~0ULL;

    // Attempt to synchronize threads to start simultaneously
    // (as in the previous example).

    InterlockedIncrement(&g_poor_mans_lock);
    while (g_poor_mans_lock != THREAD_COUNT);

    t_total_start = rdtsc();
    while(!g_finish) {
        // Take spinlock and measure the time of the operation
        // in processor cycles.
        t_start = rdtsc();
        my_spinlock_lock(&g_spinlock);
        t_end = rdtsc();

        // Note the duration of the operation.
        t_final = t_end - t_start;
        g_round_time[thread_id][g_counter[thread_id]] = t_end - t_total_start;

        if (t_final > g_max_wait[thread_id]) {
            g_max_wait[thread_id] = t_final;
        }

        if (t_final < g_min_wait[thread_id]) {
            g_min_wait[thread_id] = t_final;
        }

        if (++g_counter[thread_id] == MAX_COUNTER) {
            g_finish = 1;
        }

        my_spinlock_unlock(&g_spinlock);
    }

    t_total_end = rdtsc();

    // Note the total duration of the loop.
    g_total[thread_id] = t_total_end - t_total_start;
    return 0;
}

int main(void) {
    int i, j;
    int non_zero_threads = 0;
    HANDLE h[THREAD_COUNT];

    // Running THREAD_COUNT threads.
    g_poor_mans_lock = 0;
    for (i = 0; i < THREAD_COUNT; i++) {
        h[i] = CreateThread(NULL, 0, BruteTest, (LPVOID)i, 0, NULL);
    }
    WaitForMultipleObjects(THREAD_COUNT, h, TRUE, INFINITE);

    //Close the handles and write basic informations.
    for (i = 0; i < THREAD_COUNT; i++) {
        CloseHandle(h[i]);
        if (g_counter[i] > 0) {
            printf("Counter %2i: %10i [%10I64u -- %10I64u] %I64u\n", i, g_counter[i], g_min_wait[i], g_max_wait[i], g_total[i]);
            non_zero_threads++;
        }
    }

    printf("Total: %i threads\n", non_zero_threads);
    
    FILE *f = fopen("starv.txt", "w");
    uint64_t tsc_sum[THREAD_COUNT] = { 0 };

    for (i = 0; i < MAX_COUNTER; i++) {
        fprintf(f, "%i\t", i);
        for (j = 0; k < THREAD_COUNT; j++) {
            if (g_round_time[j][i] == 0) {
                g_round_time[j][i] = tsc_sum[j];
            }
            tsc_sum[j] = g_round_time[j][i];

            fprintf(f, "%u\t", g_round_time[j][i]);
        }
        fprintf(f, "\n");
    }
    fclose(f);

    return 0;
}