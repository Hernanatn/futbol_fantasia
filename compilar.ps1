param(
    [switch]$correr = $false
)

cmake -S . --preset=ff
cmake --build compilar;

Remove-Item -Force -Recurse "distribucion/*";

Compress-Archive -Path "compilar/Debug/*" -DestinationPath "distribucion/ff.zip"
Compress-Archive -Path "data/publico/recursos" -Update -DestinationPath "distribucion/ff.zip"

iscc instalador/instalador.iss

if($correr){
Expand-Archive -Path "distribucion/ff.zip" -DestinationPath "distribucion/ff";
distribucion/ff/futbol_fantasia.exe;
}