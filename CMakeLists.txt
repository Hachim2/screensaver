# Version minimale de CMake requise
cmake_minimum_required(VERSION 3.10)

# Nom du projet (C = langage C)
project(screensaver C)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

# Créer l'exécutable "main" à partir de deux fichiers sources:
# - src/main.c (raylib uniquement)
# - src/inactivity.c (Windows uniquement)
add_executable(main src/main.c src/inactivity.c)

# Lier l'exécutable "main" à la bibliothèque raylib
# Cela inclut automatiquement:
#   - Les chemins d'en-têtes (-I)
#   - Les flags de compilation
#   - Les dépendances système (opengl32, gdi32, etc.)
target_link_libraries(main raylib)
