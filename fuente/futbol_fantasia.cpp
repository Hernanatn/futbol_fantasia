
#include "raylib.h"

int main(int argc, char** argv){
    InitWindow(1600, 900, "Fútbol Fantasía");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GREEN);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}