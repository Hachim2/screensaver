// Inclure raylib uniquement (pas de conflits !)
#include "raylib.h"
#include "inactivity.h"
#include "video.h"

#include <stdio.h>
#include <stdint.h>

int main()
{
    while (1)
    {
        // =========================================================
        // Vérification de l'inactivité
        // =========================================================

        uint32_t inactivity = get_inactivity_time();

        printf(
            "Inactivité : %lu ms\n",
            (unsigned long)inactivity
        );

        // Ta fonction personnalisée
        wait_ms(1000);


        // =========================================================
        // Lancer le screensaver après 10 secondes
        // =========================================================

        if (inactivity >= 10000)
        {
            SetConfigFlags(
                FLAG_WINDOW_UNDECORATED
            );

            InitWindow(
                GetMonitorWidth(0),
                GetMonitorHeight(0),
                "Screensaver - Ma fenêtre Raylib"
            );

            SetTargetFPS(60);


            // =====================================================
            // Initialisation vidéo
            // =====================================================

            Video video;

            if (!Video_Init(&video,"..\\videos\\spidyxvenom.mp4",GetMonitorWidth(0),GetMonitorHeight(0))){
                CloseWindow();
                return 1;
            }


            // =====================================================
            // Boucle du screensaver
            // =====================================================

            while (!WindowShouldClose())
            {
                // -------------------------------------------------
                // Détection d'une interaction utilisateur
                // -------------------------------------------------

                if (GetKeyPressed() == 0)
                {
                    // -------------------------------------------------
                    // Mise à jour de la vidéo
                    // -------------------------------------------------

                    Video_Update(&video);


                    // -------------------------------------------------
                    // Début du rendu
                    // -------------------------------------------------

                    BeginDrawing();

                    ClearBackground(BLACK);


                    // -------------------------------------------------
                    // Mesure du rendu
                    // -------------------------------------------------

                    double draw_start =
                        GetTime();

                    Video_Draw(&video);

                    double draw_time =
                        (GetTime() - draw_start) * 1000.0;


                    // -------------------------------------------------
                    // Profiling
                    // -------------------------------------------------

                    static int profile_counter = 0;

                    profile_counter++;

                    if (profile_counter >= 60)
                    {
                        printf(
                            "\n========== PROFILING ==========\n"
                            "Resolution : %dx%d\n"
                            "FPS vidéo : %.2f\n"
                            "Taille frame : %.2f MB\n"
                            "DrawTexturePro : %.3f ms\n"
                            "================================\n",

                            video.width,
                            video.height,

                            video.fps,

                            (double)video.frame_size /
                            (1024.0 * 1024.0),

                            draw_time
                        );

                        profile_counter = 0;
                    }


                    EndDrawing();
                }
                else
                {
                    // -------------------------------------------------
                    // Interaction utilisateur
                    // -------------------------------------------------

                    break;
                }
            }


            // =========================================================
            // Nettoyage
            // =========================================================

            Video_Close(&video);

            CloseWindow();
        }
    }

    return 0;
}