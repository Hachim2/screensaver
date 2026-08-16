// Inclure raylib uniquement (pas de conflits!)
#include "raylib.h"
#include "inactivity.h"
#include <stdio.h>

int main(){
    while(1){
        uint32_t inactivity = get_inactivity_time();
        printf("Inactivite lue : %lu ms\n", inactivity);
        wait_ms(1000);

        if(inactivity >= 10000){
            SetConfigFlags(FLAG_WINDOW_UNDECORATED);
            InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Screensaver - Ma fenêtre Raylib");
        
            // Définir les FPS (60 images par seconde)
            SetTargetFPS(60);
            
            // Boucle principale
            while (!WindowShouldClose()) {
                if(!GetKeyPressed()){
                    // Début du rendu
                    BeginDrawing();
                
                    // Couleur de fond (noir)
                    ClearBackground(BLACK);
                    
                    // Dessiner quelque chose (exemple: un rectangle bleu)
                    DrawRectangle(100, 100, 200, 150, BLUE);
                    
                    // Afficher le texte
                    DrawText("Screensaver en cours...", 50, 50, 20, WHITE);
                    EndDrawing();
                }
                else{
                    break;
                }
            }
        // Fermer la fenêtre
        CloseWindow();
        }
    }
    return 0;
}