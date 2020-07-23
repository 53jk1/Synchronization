// > gcc uid.c -Wall -Wextra -O3 -o uid.exe
// > uid.exe

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

typedef int (*uid_func_t)(void);

#define THREAD_COUNT 8
#define IDS_PER_THREAD (4 * 1000 * 1000)

int InterlockedGetUniqueID(void) {
    static LONG next_uid;
    return (int)InterlockedIncrement(&next_uid);
}

int BadGetUniqueID(void) {
    static volatile LONG next_uid;
    return ++next_uid;
}

static volatile LONG g_poor_mans_lock;
DWORD WINAPI BruteTest(LPVOID param) {
    int i;
    clock_t start_time;
    uid_func_t get_unique_id_ptr = (uid_func_t)param;

    // Attempting to synchronize the threads to start at once, which will increase the number of observable collisions. In practice, several threads may be preempted in this mment, but there is a significant probability that at least two threads will start at the same time.
    InterlockedIncrement(&g_poor_mans_lock);
    while (g_poor_mans_lock != THREAD_COUNT);

    // Getting the ID.
    start_time = clock();
    for (i = 0; i < IDS_PER_THREAD; i++) {
        get_unique_id_ptr();
    }

    // Return - the "exit code" returned is the execution time.
    return (DWORD)(clock() - start_time);
}

void RunTest(uid_func_t f) {
    int i;
    HANDLE h[THREAD_COUNT];
    clock_t total_time;

    // *THREAD_COUNT* threads start.
    g_poor_mans_lock = 0;
    for (i = 0; i < THREAD_COUNT; i++) {
        h[i] = CreateThread(NULL, 0, BruteTest, (LPVOID)f, 0, NULL);
    }
    WaitForMultipleObjects(THREAD_COUNT, h, TRUE, INFINITE);

    // Calculating time and closing handles.
    total_time = 0;
    for (i = 0; i < THREAD_COUNT; i++) {
        DWORD exit_time = 0;
        GetExitCodeThread(h[i], &exit_time);
        total_time += (clock_t)exit_time;

        CloseHandle(h[i]);
    }

    printf("Next ID  : %i\n", f());
    printf("Should be: %i\n", THREAD_COUNT * IDS_PER_THREAD + 1);
    printf("CPU time : %f sec\n", (double)total_time / CLOCKS_PER_SEC);
}

int main(void) {
    puts("*** Non-atomic UID:");
    RunTest(BadGetUniqueID);
    puts("\n*** Interlocked UID:");
    RunTest(InterlockedGetUniqueID);

    return 0;
}