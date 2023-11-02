@echo on

cd /d %~dp0
del /q /f /s .\src\libfacedetection\build
mkdir .\src\libfacedetection\build

cd .\src\libfacedetection\build
cmake .. -DCMAKE_INSTALL_PREFIX=..\..\..\build -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release -DDEMO=OFF
cmake --build . --config Release
cmake --build . --config Release --target install


cd /d %~dp0
del /q /f /s .\src\libfacedetection\build
mkdir .\src\libfacedetection\build

cd .\src\libfacedetection\build
cmake .. -DCMAKE_INSTALL_PREFIX=..\..\..\build -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DDEMO=OFF
cmake --build . --config Release
cmake --build . --config Release --target install