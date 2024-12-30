
#include "raylib.h"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"

const char * titulo = "fútbol Fantasía";
const int anchoPantalla = 1600;
const int altoPantalla = 900;

void Inicializar(){
    InitAudioDevice();
};
void Actualizar(){};
void Renderizar(){};
void Cerrar(){};


int main(int argc, char** argv){
    InitWindow(anchoPantalla,altoPantalla, titulo);
    SetTargetFPS(30);

    Inicializar();
    while (!WindowShouldClose()) {
        Actualizar();
        BeginDrawing();
        ClearBackground(GREEN);
        GuiLabel({10,10,300,300}, titulo);
        Renderizar();
        EndDrawing();
    }
    Cerrar();
    CloseWindow();
    return 0;
}