cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -B build -S .
cd build
make -j4
