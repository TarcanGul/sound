SRC := src/
BIN := bin/

CC := g++
FLAGS := -std=c++11

TARGET := $(BIN)sound
SOURCES := $(wildcard $(SRC)*.cpp)
OBJECTS := $(patsubst $(SRC)%.cpp, $(BIN)%.o, $(SOURCES))

all : $(TARGET) 
	@echo Build complete!

$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) -o $(TARGET)

$(BIN)%.o: $(SRC)%.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm $(TARGET) $(OBJECTS) 
