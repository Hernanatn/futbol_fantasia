#include <cstdlib>
#include <exception>
#include <sstream>
#include <queue>
#include <iostream>
#include <map>
#include <tuple>


#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"

#include "errores--.hpp"




//<estilos>
#include "estilos.h"
//</estilos>


namespace controles {

    struct Accion {
        protected:
            int codigo;

        public:
        Accion(int c){codigo = c;};
        int operator()(){
            return codigo;
        };
    };

    struct AccionTeclado : public Accion {
            KeyboardKey operator()(){
            return KeyboardKey(codigo);
        };
    };
    
    struct AccionRaton : public Accion {
            MouseButton operator()(){
            return MouseButton(codigo);
        }   
    };

    struct Opcion : public Accion {
    bool operator==(Opcion otro){
        return otro() == this->codigo;
    };

    operator bool() {
        return this->codigo != 0;
    };
    };

    const Opcion SALIR = Opcion(-1);
};



// <pantalla>
namespace pantalla {
    struct Resolucion {
        int ancho;
        int alto;
        int fuente;

        operator bool() const noexcept{
            return (ancho != 0 & alto !=0);
        };
    };

    const Resolucion r2160p = {3840,2160, 50};
    const Resolucion r1440p = {2560,1440, 35};
    const Resolucion r1080p = {1920,1080, 35};
    const Resolucion r900p = {1600,900, 30};
    const Resolucion r720p = {1280,720, 25};
    const Resolucion rInicial = {640,360, 20};

    enum RESOLUCION {
        RINICIAL = -1,
        R720P,
        R900P,
        R1080P,
        R1440P,

        MINIMA = R720P,
        MAXIMA = R1440P
    };

    std::map<RESOLUCION,Resolucion> RESOLUCIONES = {
        {R720P,r720p},
        {R900P,r900p},
        {R1080P,r1080p},
        {R1440P,r1440p}
    };

    Resolucion Res(RESOLUCION r){
        try{
            return  r != -1 ? RESOLUCIONES.at(r): rInicial;
        } catch (std::exception) {
            std::cout << "<DEBUG/> Resolucion invalida: " << r << std::endl;
            return RESOLUCIONES.at(MINIMA);
        }
    }
}


// <menu>

// </menu>
// <juego>
struct Juego{
private:
    const char * titulo;
    std::stringstream log;
    pantalla::RESOLUCION resolucion;

    bool incializo = false;
    bool cargoRecursos = false;


    void centrarVentana(){
        const int monitor = GetCurrentMonitor();
        const int anchoPantalla = GetMonitorWidth(monitor);
        const int altoPantalla =  GetMonitorHeight(monitor);
        SetWindowPosition(anchoPantalla /2 - Resolucion().ancho / 2, altoPantalla/2 - Resolucion().alto/2);
    }

public:

    std::tuple<int,int> Monitor() const{
        const int monitor = GetCurrentMonitor(); 
        return  std::make_tuple(GetMonitorWidth(monitor),GetMonitorHeight(monitor));
    }
    pantalla::Resolucion Resolucion() const{
        return  pantalla::Res(resolucion);
    }

    Juego(const char * titulo){
        this->titulo = titulo;
        this->resolucion = pantalla::RINICIAL; 

    }
    Juego(std::string titulo){
        this->titulo = titulo.c_str();
        this->resolucion = pantalla::RINICIAL;
    }


    bool EstaListo(){
        return (incializo & cargoRecursos);
    }




    err::Error Inicializar(){
        InitWindow(Resolucion().ancho, Resolucion().alto, this->titulo); 
        if (!IsWindowReady()){
            auto e = err::Fatal("No se pudo iniciar la ventana.");
            log << e; return e;
        }
        GuiLoadStyle("recursos\\temas\\ff.rgs");
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

        SetExitKey(NULL);
        incializo = true;
        return err::Exito("Juego incializado correctamente.");
    };

    err::Error CargarRecursos(){
        if (!incializo){
            auto e = err::Fatal("El juego debe estar inicializado para cargar los recursos");
            log << e; return e;
        }

        cargoRecursos = true;
        return err::Exito("Recursos cargados satisfactoriamente. Juego Listo.");
    }

    res::Resultado<pantalla::RESOLUCION> adivinarResolucionAdecuada(){
        err::Error e = err::Exito();
        pantalla::RESOLUCION r = resolucion;

        if (!IsWindowReady()){
            e = err::Fatal("No se pudo iniciar la ventana.");
            log << e;
        }

        const int monitor = GetCurrentMonitor();
        const int anchoPantalla = GetMonitorWidth(monitor);
        const int altoPantalla =  GetMonitorHeight(monitor);
        
        std::stringstream msj;
        msj << "<DEBUG/> " << monitor << " Ancho: " << anchoPantalla << " Alto: " << altoPantalla << "\n";
        GuiLabel(Rectangle{0,0,500,100}, msj.str().c_str());
        if (!(anchoPantalla|| altoPantalla)){
            e = err::Error{
                err::ERROR,"No se pudieron leer las dimensiones del monitor"
            };
        };

        pantalla::RESOLUCION seleccionada = pantalla::MINIMA;
        for (int i = 0; i < pantalla::RESOLUCIONES.size(); i++){
            if (abs(pantalla::Res(pantalla::RESOLUCION(i)).ancho - anchoPantalla) < abs(pantalla::Res(seleccionada).ancho - anchoPantalla)){
                seleccionada = pantalla::RESOLUCION(i);
            }
        }

        if (!Res(seleccionada)){
            e = err::Error{
                err::ERROR,"No se pudo deducir el tamaño de la pantalla"
            };
        }

        r = seleccionada;

        return res::Resultado<pantalla::RESOLUCION>{r,e};

    }

    void cambiarResolucion(pantalla::RESOLUCION resolucion){
        this->resolucion = resolucion;
    }

    void anteriorResolucion(){
        if (this->resolucion > pantalla::RESOLUCION::MINIMA){ 
            this->resolucion = static_cast<pantalla::RESOLUCION>(static_cast<int>(resolucion) - 1);
        }else{
            this->resolucion = pantalla::RESOLUCION::MAXIMA;
        }
    }

    void siguienteResolucion(){
        if (this->resolucion < pantalla::RESOLUCION::MAXIMA){ 
            this->resolucion = static_cast<pantalla::RESOLUCION>(static_cast<int>(resolucion) + 1);
        }else{
            this->resolucion = pantalla::RESOLUCION::MINIMA;
        }
    }

    err::Error actualizarResolucion(){
        if (!IsWindowReady()){
            auto e = err::Fatal("No se pudo iniciar la ventana.");
            log << e; return e;
        }

        SetWindowSize(Resolucion().ancho, Resolucion().alto);
        GuiSetStyle(DEFAULT, TEXT_SIZE, Resolucion().fuente);
        centrarVentana();
        return err::Exito();
    };

    void Actualizar(std::queue<controles::Accion>){};
    void Renderizar(){};

    err::Error Correr() {
      if (!EstaListo()) {
        return err::Fatal("El juego debe estar listo para poder Correr.");
      }
      while (!WindowShouldClose()) {
        std::queue<controles::Accion> acciones;

        this->Actualizar(acciones);
        BeginDrawing();
        ClearBackground(NEGRO);
        // Menu
        std::stringstream mensaje;
        mensaje << "Fútbol Fantasía v0.0.1" << "\n"
                << "Resolución: " << Resolucion().ancho << " x "
                << Resolucion().alto << "\n";

        GuiWindowBox(Rectangle{0,0,float(Resolucion().ancho/2),float(Resolucion().alto/2)}, "Menu");
        controles::Opcion seleccion(0);
        if (!seleccion) {
          int codigo = GuiMessageBox(
              Rectangle{
                  float(Resolucion().ancho / 2) - float(Resolucion().ancho / 6),
                  float(Resolucion().alto / 2) - float(Resolucion().alto / 6),
                  float(Resolucion().ancho / 3), float(Resolucion().alto / 3)},
              titulo, mensaje.str().c_str(), "Jugar;Configuración;Salir");

          if (codigo == 1) {
            seleccion = controles::Opcion(1);
          } else if (codigo == 2) {
            seleccion = controles::Opcion(100);
            siguienteResolucion();
            actualizarResolucion();
          } else if (codigo == 3 || codigo == 0) {
            seleccion = controles::SALIR;
          }
        }

        if (seleccion == controles::SALIR) {
          break;
        }
        this->Renderizar();
        EndDrawing();
      }

      return err::Exito();
    }


    void Cerrar(){
        std::cout << log.str() << std::endl;
        CloseAudioDevice();
        CloseWindow();
    };
};


// </juego>






int main(int argc, char** argv){
    const char * titulo = "Fútbol Fantasía";
    Juego juego(titulo);

    err::Error error;
    error = juego.Inicializar();
    if (error){
        std::cout << error;
        goto cerrar;
    }
    error = juego.CargarRecursos();
    if (error){
        std::cout << error << std::endl;
        goto cerrar;
        
    }
    error = juego.Correr();
    if (error){
        std::cout << error << std::endl;
        goto cerrar;
    }

    cerrar:
    juego.Cerrar();
    return 0;
}