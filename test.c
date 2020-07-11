#include <stdio.h>
#include <windows.h>

int main(void) {
    char buffer[128] = "You have entered: ";
    scanf("%109s", buffer + 18);
    MessageBoxA(NULL, buffer, "Example", MB_OK);
    return 0;
}