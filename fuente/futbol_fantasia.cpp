/*<dependencias>*/
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <tuple>

/*<estilos>*/
#include "estilos.h"
/*</estilos>*/

#include "Error.hpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"

#include "errores--.hpp"
/*</dependencias>*/

template <typename T> struct Vector2d {
  T x, y;

public:
  Vector2d() {
    x = T{};
    y = T{};
  };
  Vector2d(T x, T y) {
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

/*<controles>*/ namespace controles {
template <typename T> struct Accion {
protected:
  T accion;

public:
  Accion(T c) { accion = c; };
  T operator()() const { return accion; };
  bool operator==(Accion otro) { return otro() == this->accion; };
  operator bool() { return this->accion != 0; };
};

const Accion SALIR = Accion(-1);
const Accion NADA = Accion(0);
const Accion JUGAR = Accion(1);
const Accion PAUSAR = Accion(2);
const Accion CONTINUAR = Accion(3);
const Accion CONFIGURACION = Accion(3);
}; // namespace controles

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
  std::stringstream log;
  pantalla::RESOLUCION resolucion;

  bool incializó = false;
  bool cargóRecursos = false;

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
      auto e = err::Fatal("No se pudo iniciar la ventana.");
      log << e;
      return e;
    }

    SetWindowSize(Resolucion().ancho, Resolucion().alto);
    GuiSetStyle(DEFAULT, TEXT_SIZE, Resolucion().fuente);
    centrarVentana();
    return err::Exito();
  };

public:
  Vector2 Monitor() const {
    const int monitor =
        GetCurrentMonitor(); // En verdad se debería constatar que
                             // `IsWidnowReady`; pero por retorna 0 si se llama
                             // antes, el cual es el comportamiento deseado en
                             // la amplia mayoría de los casos (el primer
                             // monitor de la lista de monitores disponibles,
                             // que puede ser el único).
    return Vector2(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
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

  bool EstaListo() { return (incializó & cargóRecursos); }

  err::Error Inicializar() {
    InitWindow(Resolucion().ancho, Resolucion().alto, this->titulo);
    if (!IsWindowReady()) {
      auto e = err::Fatal("No se pudo iniciar la ventana.");
      log << e;
      return e;
    }
    GuiLoadStyle("recursos\\temas\\ff.rgs");
    SetTargetFPS(30);

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
      auto e = err::Generico("No se pudo iniciar el audio.");
      log << e;
      return e;
    }

    auto [resolucionObjetivo, error] = adivinarResolucionAdecuada()();
    if (error) {
      error.agregarMensaje("// En Juego->Inicializar().");
      log << error;
      return error;
    }

    cambiarResolucion(resolucionObjetivo);
    actualizarResolucion();

    SetExitKey(NULL);
    incializó = true;
    return err::Exito("Juego incializado correctamente.");
  };

  err::Error CargarRecursos() {
    if (!incializó) {
      auto e = err::Fatal(
          "El juego debe estar inicializado para cargar los recursos");
      log << e;
      return e;
    }

    cargóRecursos = true;
    return err::Exito("Recursos cargados satisfactoriamente. Juego Listo.");
  }
  err::Error DescargarRecursos() {
    if (!incializó) {
      auto e = err::Fatal(
          "El juego debe estar inicializado para cargar los recursos");
      log << e;
      return e;
    }

    cargóRecursos = false;
    return err::Exito("Recursos descargados satisfactoriamente.");
  }

  res::Resultado<pantalla::RESOLUCION> adivinarResolucionAdecuada() {
    err::Error e = err::Exito();
    pantalla::RESOLUCION r = resolucion;

    if (!IsWindowReady()) {
      e = err::Fatal("No se pudo iniciar la ventana.");
      log << e;
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

  controles::Accion<int> Actualizar(std::queue<controles::Accion<int>>) {
    return controles::NADA;
  };

  void Renderizar() {};

  err::Error Jugar() { return err::Exito(); };

  err::Error Correr() {
    if (!EstaListo()) {
      return err::Fatal("El juego debe estar listo para poder Correr.");
    }
    controles::Accion estado = controles::NADA;
    std::stringstream teclas;

    while (!WindowShouldClose()) {
      int tecla = GetKeyPressed();
      if (IsKeyPressed(tecla)){
          teclas << tecla << "\n";
      }
      BeginDrawing();
      ClearBackground(NEGRO);

    
    Iniciar:
      std::stringstream mensaje;
      mensaje << "Fútbol Fantasía v0.0.1" << "\n"
              << teclas.str()
              << " | Resolución: " << Resolucion().ancho << " x "
              << Resolucion().alto << "\n";

      if (!estado) {
        int codigo = GuiMessageBox(
            Rectangle{
                float(Resolucion().ancho / 2) - float(Resolucion().ancho / 6),
                float(Resolucion().alto / 2) - float(Resolucion().alto / 6),
                float(Resolucion().ancho / 3), float(Resolucion().alto / 3)},
            titulo, mensaje.str().c_str(), "Jugar;Configuración;Salir");

        switch (codigo) {
        case 1:
          estado = controles::JUGAR;

          // goto Jugar;
          break;
        case 2:
          estado = controles::CONFIGURACION;
          siguienteResolucion();

          // goto Configuracion;
          estado = controles::NADA;
          break;
        case 3:
        case 0:
          estado = controles::SALIR;
          goto Salir;
          break;
        };
      }

    Configuracion:
      while (estado == controles::CONFIGURACION) {
        if (IsKeyPressed(KEY_ESCAPE)) {
          estado = controles::JUGAR;

          goto Jugar;
        }

        int codigo = GuiMessageBox(
            Rectangle{
                float(Resolucion().ancho / 2) - float(Resolucion().ancho / 6),
                float(Resolucion().alto / 2) - float(Resolucion().alto / 6),
                float(Resolucion().ancho / 3), float(Resolucion().alto / 3)},
            titulo, mensaje.str().c_str(), "Jugar;Configuración;Salir");

        switch (codigo) {
        case 1:
          estado = controles::JUGAR;

          // goto Jugar;
          break;
        case 2:
          estado = controles::CONFIGURACION;
          siguienteResolucion();

          // goto Configuracion;
          estado = controles::NADA;
          break;
        case 3:
        case 0:
          estado = controles::SALIR;
          goto Salir;
          break;
        };
      }
    Jugar:
      while (estado == controles::JUGAR) {
        if (IsKeyPressed(KEY_ESCAPE)) {
          estado = controles::PAUSAR;
          goto Pausar;
        }
      }
    Pausar:
      while (estado == controles::PAUSAR) {
        if (IsKeyPressed(KEY_ESCAPE)) {
          estado = controles::JUGAR;
          goto Jugar;
        }
      }
    Salir:
      if (estado == controles::SALIR) {
        break;
      }
      EndDrawing();
    }
    return err::Exito();
  }

  void Cerrar() {
    std::cout << log.str() << std::endl;
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
    goto cerrar;
  }
  error = juego.Correr();
  if (error) {
    std::cout << error << std::endl;
    goto cerrar;
  }

  error = juego.DescargarRecursos();
  if (error) {
    std::cout << error << std::endl;
    goto cerrar;
  }

cerrar:
  juego.Cerrar();
  return 0;
}