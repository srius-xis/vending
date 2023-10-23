# make:
# arm-linux-gnueabi-g++ ./cls*/**/*.c* ./main.cpp  -lpthread -std=c++11 -o ./main.exe
# arm-linux-gnueabi-g++ ./cls*/*.c*  cls*/cls*/*.c* ./main.cpp  -lpthread -std=c++11 -o ./main.exe


TARGET = ./bin/vending.exe

SOURCE = ./cls*/*.c*  ./cls*/cls*/*.c* ./main.cpp

LIBS += -lpthread -lm -std=c++11
CC = arm-linux-gnueabi-g++


$(TARGET): $(SOURCE)
	$(CC) $^ -o $@ $(LIBS)

.PHONY:$(TARGET)

clean:
	$(RM) $(TARGET)

.PHONY:clean

# install:
