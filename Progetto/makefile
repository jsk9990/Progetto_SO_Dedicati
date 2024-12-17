$(info Sources: $(SOURCES))
$(info Objects: $(OBJECTS))
$(info Target: $(TARGET))


SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Definisci i file sorgente
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/draw.cpp $(SRC_DIR)/config.cpp 
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Nome dell'eseguibile
TARGET = $(BUILD_DIR)/main

# Opzioni di compilazione
CXX = g++
CXXFLAGS = -I$(INCLUDE_DIR) -std=c++17
LDFLAGS = -lallegro -lallegro_primitives -lallegro_font -lallegro_ttf -lpthread

# Regola per creare l'eseguibile finale
$(TARGET): $(OBJECTS)
	$(CXX) $^ -o $@ $(LDFLAGS)

# Regola per compilare i file oggetto
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Pulizia dei file generati
clean:
	rm -rf $(BUILD_DIR)/*
