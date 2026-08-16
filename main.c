#include <windows.h>
#include <stdio.h>

int main() {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);
    DWORD start_time = GetTickCount();

    do {
        if (GetLastInputInfo(&lii)) {
            DWORD current_time = GetTickCount();
            DWORD inactivity_period = current_time - lii.dwTime;
            printf("Inactivity : %lu ms\n", inactivity_period);
        }
        Sleep(1000);
    } while (GetTickCount() - start_time < 30000);
    printf("OK\n");
    return 0;
}