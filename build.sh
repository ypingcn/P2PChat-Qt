sudo apt install g++ qt5-default qt5-qmake -y
mkdir build
cd build
qmake ../src
make
./p2pchat-qt
