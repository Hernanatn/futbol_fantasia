#include <cstdlib>
#include <sstream>
#include <iostream>
#include <vector>

#include "errores--/fuente/Error.hpp"
#include "errores--/fuente/Resultado.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"

#include "errores--/fuente/errores--.hpp"



//<estilos>
#include "estilos.h"
//</estilos>


// <juego.hpp>
namespace pantalla {
    struct Resolucion {
        int ancho;
        int alto;
        int fuente;

        operator bool() const noexcept{
            return (ancho != 0 & alto !=0);
        };
    };

    const Resolucion r2160p = {3840,2160, 35};
    const Resolucion r1080p = {1920,1080, 35};
    const Resolucion r900p = {1600,900, 30};
    const Resolucion r720p = {1280,720, 25};
    const Resolucion rInicial = {640,360, 20};

    const std::vector<Resolucion> RESOLUCIONES_DISPONIBLES{
        r2160p,r1080p,r900p,r720p  
    };

}

struct Juego{
private:
    const char * titulo;
    std::stringstream log;
    pantalla::Resolucion resolucion;

    bool incializo = false;
    bool cargoRecursos = false;

public:

    Juego(const char * titulo){
        this->titulo = titulo;
        this->resolucion = pantalla::rInicial; 

    }
    Juego(std::string titulo){
        this->titulo = titulo.c_str();
        this->resolucion = pantalla::rInicial;
    }


    bool EstaListo(){
        return (incializo & cargoRecursos);
    }

    err::Error Inicializar(){
        InitWindow(resolucion.ancho, resolucion.alto, this->titulo); 
        if (!IsWindowReady()){
            auto e = err::Fatal("No se pudo iniciar la ventana.");
            log << e; return e;
        }
        SetTargetFPS(30);


        InitAudioDevice(); 
        if (!IsAudioDeviceReady()){
            auto e = err::Generico("No se pudo iniciar el audio.");
            log << e; return e;
        }

        auto [resolucionObjetivo, error] = adivinarResolucionAdecuada()();
        if (error){
            error.agregarMensaje("// En Juego->Inicializar().");
            log << error; return error;
        }

        cambiarResolucion(resolucionObjetivo);
        actualizarResolucion();

        incializo = true;
        return err::Exito("Juego incializado correctamente.");
    };

    err::Error CargarRecursos(){
        if (!incializo){
            auto e = err::Fatal("El juego debe estar inicializado para cargar los recursos");
            log << e; return e;
        }

        GuiLoadStyle("recursos\\temas\\ff.rgs");


        cargoRecursos = true;
        return err::Exito("Recursos cargados satisfactoriamente. Juego Listo.");
    }

    res::Resultado<pantalla::Resolucion> adivinarResolucionAdecuada(){
        err::Error e = err::Exito();
        pantalla::Resolucion r = resolucion;

        if (!IsWindowReady()){
            e = err::Fatal("No se pudo iniciar la ventana.");
            log << e;
        }

        const int monitor = GetCurrentMonitor();
        const int anchoPantalla = GetMonitorWidth(monitor);
        const int altoPantalla =  GetMonitorHeight(monitor);

        if (!(anchoPantalla|| altoPantalla)){
            e = err::Error{
                err::ERROR,"No se pudieron leer las dimensiones del monitor"
            };
        };

        pantalla::Resolucion seleccionada;
        for (int i = 0; i < pantalla::RESOLUCIONES_DISPONIBLES.size(); i++){
            if (abs(pantalla::RESOLUCIONES_DISPONIBLES[i].ancho - anchoPantalla) < seleccionada.ancho - anchoPantalla){
                seleccionada = pantalla::RESOLUCIONES_DISPONIBLES[i];
            }
        }

        if (!seleccionada){
            e = err::Error{
                err::ERROR,"No se pudo deducir el tamaño de la pantalla"
            };
        }

        r = seleccionada;

        return res::Resultado<pantalla::Resolucion>{r,e};

    }

    void cambiarResolucion(int indice){
        this->resolucion = pantalla::RESOLUCIONES_DISPONIBLES[indice];
    }
    void cambiarResolucion(pantalla::Resolucion resolucion){
        this->resolucion = resolucion;
    }

    err::Error actualizarResolucion(){
        if (!IsWindowReady()){
            auto e = err::Fatal("No se pudo iniciar la ventana.");
            log << e; return e;
        }

        SetWindowSize(resolucion.ancho, resolucion.alto);
        GuiSetStyle(DEFAULT, TEXT_SIZE, resolucion.fuente);

        return err::Exito();
    };

    void Actualizar(){};
    void Renderizar(){};


    err::Error Jugar(){
        if (!EstaListo()){
            return err::Fatal("El juego debe estar listo para poder Jugar.");
        }

        bool mostrarMensaje = false;
        while (!WindowShouldClose()) {
            this->Actualizar();
            BeginDrawing();
            ClearBackground(NEGRO);
            if (            GuiButton(Rectangle{
                    float(resolucion.ancho/2),
                    float(resolucion.alto/2),
                    200,
                    200,
                
                }, "Fútbol Fantasía")) mostrarMensaje = true;

            std::stringstream mensaje;
            mensaje     <<
                "Ancho: "   << resolucion.ancho     << " Alto: "   << resolucion.alto  << "\n"
                "Fuente: "  << resolucion.fuente    << "\n";

            if (mostrarMensaje)
            {
                int result = GuiMessageBox(Rectangle{ 85, 70, 500, 200 },
                    titulo, mensaje.str().c_str(), "Ok;Salir");
                if (result >= 0) mostrarMensaje = false;
                if (result >= 2) break;
            }
            this->Renderizar();
            EndDrawing();
        }

        return err::Exito();
    }
    void Cerrar(){
        CloseAudioDevice();
        CloseWindow();
    };
};


// </juego.hpp>







int main(int argc, char** argv){
    const char * titulo = "Fútbol Fantasía";
    Juego juego(titulo);

    err::Error error;
    error = juego.Inicializar();
    error = juego.CargarRecursos();
    error = juego.Jugar();

    juego.Cerrar();
    return 0;
}