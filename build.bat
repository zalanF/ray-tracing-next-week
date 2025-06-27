@echo off
:: For initialization, use this command
::cmake -G "MinGW Makefiles" ..
cmake --build .
echo:
build/RayTracer.exe > image.ppm
echo:
echo Converting to png...
echo:
ffmpeg -y -loglevel quiet -i image.ppm image.png
echo:
echo Opening image.
echo:
start image.png