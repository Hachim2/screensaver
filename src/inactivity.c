/* inactivity.c - Fonctions Windows pour la détection d'inactivité */
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

/* Vérifier l'inactivité pendant 30 secondes */
uint32_t get_inactivity_time() {
    LASTINPUTINFO lii;
    lii.cbSize = sizeof(LASTINPUTINFO);

    if (GetLastInputInfo(&lii)) {
        DWORD current_time = GetTickCount();
        return current_time - lii.dwTime;
    }

    return 0;
}

void wait_ms(uint32_t milliseconds){
    Sleep(milliseconds);
}