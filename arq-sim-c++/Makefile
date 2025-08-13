# Define o compilador de C++ a ser usado
CPP = g++

# Define as flags de compilação:
CPPFLAGS = -std=c++17 -Wall -Wextra -g

# Define o nome do nosso executável final, conforme as instruções
TARGET = arq-sim

# Encontra todos os arquivos .cpp na pasta para compilar
SOURCES = $(wildcard *.cpp)

# Gera os nomes dos arquivos-objeto (.o) a partir dos arquivos-fonte (.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

# Regra 'all': é a regra padrão que executa quando você digita 'make'
all: $(TARGET)

# Regra de linkagem: como construir o executável final (TARGET)
$(TARGET): $(OBJECTS)
	$(CPP) $(CPPFLAGS) -o $(TARGET) $(OBJECTS)

# Regra de compilação: um padrão para ensinar o 'make' a transformar
# qualquer arquivo .cpp em um arquivo .o correspondente.
%.o: %.cpp
	$(CPP) $(CPPFLAGS) -c $< -o $@

# Regra 'clean': um comando útil para limpar a pasta dos arquivos gerados
# pela compilação. Para usar: 'make clean'
clean:
	rm -f $(OBJECTS) $(TARGET) $(TARGET).exe