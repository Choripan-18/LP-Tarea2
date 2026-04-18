CC     = gcc
STD    = -std=c11
WARN   = -Wall -Wextra
FLAGS  = $(STD) $(WARN)
LIBS   = -lm

SRC    = main.c tablero.c piezas.c armas.c
OBJ    = main.o tablero.o piezas.o armas.o
TARGET = rey_destronado

# ─── Compilación principal ─────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(FLAGS) -o $(TARGET) $(OBJ) $(LIBS)

# ─── Compilación separada por módulo ──────────────────
main.o: main.c main.h tablero.h piezas.h armas.h
	$(CC) $(FLAGS) -c main.c

tablero.o: tablero.c tablero.h piezas.h main.h
	$(CC) $(FLAGS) -c tablero.c

piezas.o: piezas.c piezas.h main.h
	$(CC) $(FLAGS) -c piezas.c

armas.o: armas.c armas.h piezas.h main.h
	$(CC) $(FLAGS) -c armas.c

# ─── Utilidades ───────────────────────────────────────
clean:
	rm -f $(OBJ) $(TARGET)

valgrind: $(TARGET)
	valgrind --leak-check=full --track-origins=yes ./$(TARGET)

.PHONY: all clean valgrind