/* windows_compat.h - Wrapper pour éviter les conflits entre windows.h et raylib.h */
#ifndef WINDOWS_COMPAT_H
#define WINDOWS_COMPAT_H

/* Inclure raylib EN PREMIER */
#include "../raylib/src/raylib.h"

/* Ensuite inclure les parties de windows.h dont nous avons besoin,
   en désactivant les symboles qui conflictent */
#define NOGDI        /* Désactive GDI: Rectangle, etc. */
#include <windows.h>

#endif /* WINDOWS_COMPAT_H */
