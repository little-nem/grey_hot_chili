TARGET=temp_name

CC=g++
CFLAGS=-std=c++11 -Wall -O3 -Isrc/lib/ -fopenmp
LDFLAGS=-lgomp

SRC=$(addprefix	src/,\
		main.cpp image.cpp pixel.cpp stb_implem.cpp)

OBJ=$(patsubst src/%.cpp, build/%.o, $(SRC))

all: $(TARGET)

build:
	mkdir -p build

$(TARGET): build $(OBJ)
	$(CC) -o $@	$(OBJ) $(LDFLAGS)

build/%.o: src/%.cpp
	$(CC) -o $@	-c $< $(CFLAGS)

clean:
	rm -rf build

mrproper: clean
	rm -f $(TARGET)

.PHONY:	all	clean mrproper
