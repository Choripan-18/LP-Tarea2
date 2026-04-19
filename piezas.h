#ifndef PIEZAS_H
#define PIEZAS_H

#include <stdbool.h>

struct Juego;

// Estructura que representa una pieza en el tablero.
// Los tipos y sus HP están definidos en el enunciado.
// Indica la posición actual de la pieza en el tablero.
typedef struct {
    char tipo;
    int hp;
    int x, y;
} Pieza;

// Estructura que representa una celda del tablero.
// Contiene un puntero a la pieza que ocupa la celda, o NULL si está vacía.
typedef struct {
    Pieza *pieza;
} Celda;

// Inicializa las piezas en el tablero según el nivel indicado.
void spawn_nivel(struct Juego *juego, int nivel);

// Mueve todas las piezas enemigas hacia el Rey según sus reglas de movimiento.
void mover_enemigos(struct Juego *juego);

// Verifica si el Rey ha sido capturado por alguna pieza enemiga.
// Retorna true si el Rey ha sido capturado, false si sigue vivo.
bool verificar_estado_rey(struct Juego *juego);

// Elimina la pieza en la posición indicada y libera su memoria.
void eliminar_pieza(struct Juego *juego, int x, int y);

// Retorna el número de enemigos vivos en el tablero.
int contar_enemigos(struct Juego *juego);

#endif