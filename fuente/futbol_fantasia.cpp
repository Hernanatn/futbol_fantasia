
#include "raylib.h"

int main(int argc, char** argv){
    InitWindow(1600, 90, "juan");
    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RED);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}