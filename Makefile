SRC := src/
BIN := bin/

CC := clang++
FLAGS := -std=c++11
FRAMEWORKS := -framework AudioToolbox -framework Foundation

TARGET := $(BIN)sound
SOURCES := $(wildcard $(SRC)*.cpp)
OBJECTS := $(patsubst $(SRC)%.cpp, $(BIN)%.o, $(SOURCES))

all : $(TARGET) 
	@echo Build complete!

# Making the final executible.
$(TARGET): $(OBJECTS)
	$(CC) $(FLAGS) $(OBJECTS) $(FRAMEWORKS) -o $(TARGET)

# Building the object files.
$(BIN)%.o: $(SRC)%.cpp
	$(CC) $(FLAGS) -c $< -o $@

clean:
	rm $(TARGET) $(OBJECTS) 
