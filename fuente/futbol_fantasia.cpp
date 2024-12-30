
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
        GuiLoadStyle("recursos/temas/ff.rgs");
        GuiLabel({1,1,300,300}, titulo);
        int result = GuiMessageBox((Rectangle){ 85, 70, 250, 100 },
                    "#191#Message Box", "Hi! This is a message!", "Nice;Cool");
        Renderizar();
        EndDrawing();
    }
    Cerrar();
    CloseWindow();
    return 0;
}