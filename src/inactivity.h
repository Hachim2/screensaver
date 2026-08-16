/* inactivity.h - Déclaration de la fonction d'inactivité */
#ifndef INACTIVITY_H
#define INACTIVITY_H
#include <stdint.h>

uint32_t get_inactivity_time(void);
void wait_ms(uint32_t milliseconds);

#endif /* INACTIVITY_H */
