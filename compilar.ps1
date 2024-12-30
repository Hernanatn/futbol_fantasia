param(
    [switch]$correr = $false
)

#Copy-Item -Force -Recurse -Path "compilar/Debug/*" -Destination "distribucion";
cmake -S . --preset=ff
cmake --build compilar;

Compress-Archive -Path "compilar/Debug/*" -DestinationPath "distribucion/ff.zip"
iscc instalador/instalador.iss

if($correr){
Expand-Archive -Path "distribucion/ff.zip" -DestinationPath "distribucion/ff"
distribucion/ff/futbol_fantasia.exe 
}