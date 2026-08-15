#include <windows.h>
#include <stdio.h>

int main() {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);

    if (GetLastInputInfo(&lii)) {
        DWORD dwGetTickCount = GetTickCount();
        DWORD idleTime = dwGetTickCount - lii.dwTime;
        printf("Inactivité : %lu ms\n", idleTime);
    } else {
        printf("Erreur lors de l'appel\n");
    }
    return 0;
}
