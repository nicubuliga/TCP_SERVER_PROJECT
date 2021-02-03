g++ -c clientProject.cpp
g++ clientProject.o -o clientProject -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio -lsfml-network -pthread
./clientProject 127.0.0.1 2016