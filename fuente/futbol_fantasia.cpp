/*<dependencias>*/
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <tuple>

/*<estilos>*/
#include "estilos.h"
/*</estilos>*/

#include "Error.hpp"
#include "Resultado.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"

#include "arenas++.hpp"
#include "errores--.hpp"
/*</dependencias>*/

template <typename T> struct Vector2d {
  T x, y;

public:
  Vector2d(){
    x = T{};
    y = T{};
  };
  Vector2d(T x, T y) requires con_constructor_por_defecto<T>{
    x = x;
    y = y;
  };
  Vector2d(Vector2 v2) {
    x = v2.x;
    y = v2.y;
  };
  Vector2d(T a) {
    x = a;
    y = a;
  };
};

template <typename T> struct Vector3d {
  T x, y, z;
};

template <typename T> struct Vector4d {
  T x, y, z, w;
};

/*<estado>*/ namespace estado {
enum Estado {
  // Errores. Enteros negativos explícitos.
  SALIR = -1,

  // Nada. 0, NULL, nullptr.
  NADA,

  // Ciclos principales. Enteros positivos implícitos < 100
  INICIAR, // Lanzador. autenticación y carga/descarga de recursos.
  JUGAR,   // Ciclo principal del juego.
  LIMPIAR, // Limpieza.
  CERRAR,  // Salida.

  // Menúes. Enteros positivos implícitos >= 100.
  MENU = 100,
  AYUDA,
  PANTALLA,
  AUDIO,
  CREDITOS,

};

// errores
namespace errores {
struct VentanaNoEstaLista : public err::Error {
  VentanaNoEstaLista(std::string msj = "") {
    this->codigo = err::FATAL;
    this->mensaje = "No se pudo iniciar la ventana. " + msj;
  };
};
struct AudioNoEstaListo : public err::Error {
  AudioNoEstaListo(std::string msj = "") {
    this->codigo = err::FATAL;
    this->mensaje = "No se pudo iniciar el audios. " + msj;
  };
};
} // namespace errores
}; // namespace estado

/*<pantalla>*/ namespace pantalla {
struct Resolucion {
  int ancho;
  int alto;
  int fuente;

  operator bool() const noexcept { return (ancho != 0 & alto != 0); };
};

const Resolucion r2160p = {3840, 2160, 50};
const Resolucion r1440p = {2560, 1440, 35};
const Resolucion r1080p = {1920, 1080, 35};
const Resolucion r900p = {1600, 900, 30};
const Resolucion r720p = {1280, 720, 25};
const Resolucion rInicial = {640, 360, 20};

enum RESOLUCION {
  RINICIAL = -1,
  R720P,
  R900P,
  R1080P,
  R1440P,

  MINIMA = R720P,
  MAXIMA = R1440P
};

std::map<RESOLUCION, Resolucion> RESOLUCIONES = {
    {R720P, r720p}, {R900P, r900p}, {R1080P, r1080p}, {R1440P, r1440p}};

Resolucion Res(RESOLUCION r) {
  try {
    return r != -1 ? RESOLUCIONES.at(r) : rInicial;
  } catch (std::exception) {
    std::cout << "<DEBUG/> Resolucion invalida: " << r << std::endl;
    return RESOLUCIONES.at(MINIMA);
  }
}
}; // namespace pantalla

// <menu>

// </menu>
// <juego>
struct Sesion {};

struct Juego {
private:
  const char *titulo;
  std::stringstream debug;
  std::stringstream error;
  pantalla::RESOLUCION resolucion;

  bool incializó = false;
  bool cargóRecursos = false;
  estado::Estado estado = estado::NADA;

  Sesion sesion{};

  void centrarVentana() {
    const int monitor = GetCurrentMonitor();
    const int anchoPantalla = GetMonitorWidth(monitor);
    const int altoPantalla = GetMonitorHeight(monitor);
    SetWindowPosition(anchoPantalla / 2 - Resolucion().ancho / 2,
                      altoPantalla / 2 - Resolucion().alto / 2);
  }

  err::Error actualizarResolucion() {
    if (!IsWindowReady()) {
      auto e = estado::errores::VentanaNoEstaLista("La ventana debe haber sido inicializada (generalmente llamando `Juego.Inicializar()`) para poder asignar la resolución.");
      this->error << e;
      return e;
    }

    SetWindowSize(Resolucion().ancho, Resolucion().alto);
    GuiSetStyle(DEFAULT, TEXT_SIZE, Resolucion().fuente);
    centrarVentana();
    return err::Exito();
  };

public:
  Vector2d<float> Monitor() const {
    const int monitor =
        GetCurrentMonitor(); // En verdad se debería constatar que
                             // `IsWidnowReady`; pero por defecto retorna 0 si se llama
                             // antes, el cual es el comportamiento deseado en
                             // la amplia mayoría de los casos (el primer
                             // monitor de la lista de monitores disponibles,
                             // que puede ser el único).
    return Vector2d<float>(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
  }
  pantalla::Resolucion Resolucion() const { return pantalla::Res(resolucion); }

  Juego(const char *titulo) {
    this->titulo = titulo;
    this->resolucion = pantalla::RINICIAL;
  }
  Juego(std::string titulo) {
    this->titulo = titulo.c_str();
    this->resolucion = pantalla::RINICIAL;
  }

  bool EstaListo() { return this->estado >= estado::JUGAR; }

  err::Error Inicializar() {
    InitWindow(Resolucion().ancho, Resolucion().alto, this->titulo);
    if (!IsWindowReady()) {
      auto e = estado::errores::VentanaNoEstaLista("La ventana no incializó. IsWindowReady() devuelve 0 inmediatamente después de llamar InitWindow(). Error de Raylib, el SO o la máquina.");
      this->error << e;
      return e;
    }
    GuiLoadStyle("recursos\\temas\\ff.rgs");
    SetTargetFPS(30);

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
      auto e = estado::errores::AudioNoEstaListo("El audio no incializó. IsAudioReady() devuelve 0 inmediatamente después de llamar InitAudioDevice(). Error de Raylib, el SO o la máquina.");
      this->error << e;
      return e;
    }

    auto [resolucionObjetivo, error] = adivinarResolucionAdecuada()();
    if (error) {
      error.agregarMensaje("// En Juego->Inicializar().");
      this->error << error;
      return error;
    }

    cambiarResolucion(resolucionObjetivo);
    actualizarResolucion();

    SetExitKey(NULL);

    this->estado = estado::INICIAR;

    while (this->estado == estado::INICIAR) {
      BeginDrawing();
      ClearBackground(NEGRO);
      std::stringstream mensaje;
      mensaje << "Fútbol Fantasía v0.0.1" << "\n"
              << " | Resolución: " << Resolucion().ancho << " x "
              << Resolucion().alto << "\n";

      int codigo = GuiMessageBox(
          Rectangle{
              float(Resolucion().ancho / 2) - float(Resolucion().ancho / 6),
              float(Resolucion().alto / 2) - float(Resolucion().alto / 6),
              float(Resolucion().ancho / 3), float(Resolucion().alto / 3)},
          titulo, mensaje.str().c_str(), "Jugar;Resolución;Salir");

      switch (codigo) {
      case 1:
        this->estado = estado::JUGAR;
        break;
      case 2:
        siguienteResolucion();
        break;
      case 3:
      case 0:
        this->estado = estado::SALIR;
        break;
      };
      EndDrawing();
    };
    return err::Exito("Juego incializado correctamente.");
  };

  err::Error CargarRecursos() {
    if (this->estado < estado::INICIAR) {
      auto e = err::Fatal(
          "El juego debe estar inicializado para cargar los recursos");
      this->error << e;
      return e;
    }

    this->estado = estado::JUGAR;
    return err::Exito("Recursos cargados satisfactoriamente. Juego Listo.");
  }
  err::Error DescargarRecursos() {
    if (this->estado < estado::INICIAR) {
      auto e = err::Fatal(
          "El juego debe estar inicializado para cargar los recursos");
      this->error << e;
      return e;
    }

    this->estado = estado::CERRAR;
    return err::Exito("Recursos descargados satisfactoriamente.");
  }

  res::Resultado<pantalla::RESOLUCION> adivinarResolucionAdecuada() {
    err::Error e = err::Exito();
    pantalla::RESOLUCION r = resolucion;

    if (!IsWindowReady()) {
      auto e = estado::errores::VentanaNoEstaLista("La ventana debe haber sido inicializada (generalmente llamando `Juego.Inicializar()`) para poder adivinar la resolución.");
      this->error << e;
      return res::Resultado<pantalla::RESOLUCION>(resolucion, e);
    }
    auto monitor = Monitor();
    const int anchoPantalla = monitor.x;
    const int altoPantalla = monitor.y;

    if (!(anchoPantalla || altoPantalla)) {
      e = err::Error{err::ERROR,
                     "No se pudieron leer las dimensiones del monitor"};
    };

    pantalla::RESOLUCION seleccionada = pantalla::MINIMA;
    for (int i = 0; i < pantalla::RESOLUCIONES.size(); i++) {
      if (abs(pantalla::Res(pantalla::RESOLUCION(i)).ancho - anchoPantalla) <
          abs(pantalla::Res(seleccionada).ancho - anchoPantalla)) {
        seleccionada = pantalla::RESOLUCION(i);
      }
    }

    if (!Res(seleccionada)) {
      e = err::Error{err::ERROR, "No se pudo deducir el tamaño de la pantalla"};
    }

    r = seleccionada;

    return res::Resultado<pantalla::RESOLUCION>{r, e};
  }

  err::Error cambiarResolucion(pantalla::RESOLUCION resolucion) {
    this->resolucion = resolucion;
    return actualizarResolucion();
  }

  err::Error anteriorResolucion() {
    if (this->resolucion > pantalla::RESOLUCION::MINIMA) {
      this->resolucion =
          static_cast<pantalla::RESOLUCION>(static_cast<int>(resolucion) - 1);
    } else {
      this->resolucion = pantalla::RESOLUCION::MAXIMA;
    }
    return actualizarResolucion();
  };

  err::Error siguienteResolucion() {
    if (this->resolucion < pantalla::RESOLUCION::MAXIMA) {
      this->resolucion =
          static_cast<pantalla::RESOLUCION>(static_cast<int>(resolucion) + 1);
    } else {
      this->resolucion = pantalla::RESOLUCION::MINIMA;
    }
    return actualizarResolucion();
  };

  estado::Estado Actualizar() { return estado::NADA; };

  void Renderizar() {};

  err::Error Jugar() { return err::Exito(); };

  err::Error Correr() {
    if (!EstaListo()) {
      return err::Fatal("El juego debe estar listo para poder Correr.");
    }
    auto estado = estado::INICIAR;
    std::stringstream teclas;

    while (!WindowShouldClose()) {
      int tecla = GetKeyPressed();
      if (IsKeyPressed(tecla)) {
        teclas << tecla << "\n";
      }

    Jugar:
      while (estado == estado::JUGAR) {
        BeginDrawing();
        ClearBackground(NEGRO);
        GuiLabel(Rectangle{0, 0, 100, 100}, "JUGAR");
        if (IsKeyPressed(KEY_ESCAPE)) {
          estado = estado::MENU;
          EndDrawing();
          goto Menu;
        }
        EndDrawing();
      }
    Menu:
      while (estado == estado::MENU) {
        BeginDrawing();
        ClearBackground(NEGRO);
        GuiLabel(Rectangle{0, 0, 100, 100}, "MENU");
        if (IsKeyPressed(KEY_ESCAPE)) {
          estado = estado::JUGAR;
          EndDrawing();
          goto Jugar;
        }
        EndDrawing();
      }
    Salir:
      if (estado == estado::SALIR) {
        break;
      }
    }
    return err::Exito();
  }

  void Cerrar() {
    std::cout << this->error.str() << std::endl;
    CloseAudioDevice();
    CloseWindow();
  };
};

// </juego>

int main(int argc, char **argv) {
  const char *titulo = "Fútbol Fantasía";
  Juego juego(titulo);

  err::Error error;
  error = juego.Inicializar();
  if (error) {
    std::cout << error;
    goto cerrar;
  }
  error = juego.CargarRecursos();
  if (error) {
    std::cout << error << std::endl;
    goto limpiar;
  }
  error = juego.Correr();
  if (error) {
    std::cout << error << std::endl;
    goto limpiar;
  }

limpiar:
  error = juego.DescargarRecursos();
  if (error) {
    std::cout << error << std::endl;
    goto cerrar;
  }

cerrar:
  juego.Cerrar();
  return 0;
}