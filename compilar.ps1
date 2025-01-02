param(
    [switch]$correr = $false,
    [switch]$distriubir = $false,
    [switch]$produccion = $false
)
$env:CC="C:/msys64/ucrt64/bin/clang.exe";
$env:CXX="C:/msys64/ucrt64/bin/clang++.exe";

cd externos/raylib/src;
make;
cd ../../..;

if($produccion){
    clang++ fuente/futbol_fantasia.cpp -IC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/errores--/fuente -IC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/raylib/src -IC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/raygui/src -LC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/raylib/src -lraylib  -std=c++20 -static-libgcc -static-libstdc++ -lopengl32 -lgdi32 -lwinmm -o compilar/futbol_fantasia.exe -v;
} else {
    clang++ fuente/futbol_fantasia.cpp -IC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/errores--/fuente -IC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/raylib/src -IC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/raygui/src -LC:/Users/wasab/Documents/Programacion/Prototipos/futbol_fantasia/futbol_fantasia/externos/raylib/src -lraylib  -std=c++20 -static-libgcc -static-libstdc++ -lopengl32 -lgdi32 -lwinmm -mwindows -o compilar/futbol_fantasia.exe -v;
}
Remove-Item -Force -Recurse "distribucion/*";

Compress-Archive -Path "compilar/*" -DestinationPath "distribucion/ff.zip"
Compress-Archive -Path "data/publico/recursos" -Update -DestinationPath "distribucion/ff.zip"

if ($distribuir){
    iscc instalador/instalador.iss
}

if($distribuir -Or $correr){
    Expand-Archive -Path "distribucion/ff.zip" -DestinationPath "distribucion/ff";
}

if($correr){
cd distribucion/ff; 
./futbol_fantasia.exe;
cd ../..;
}