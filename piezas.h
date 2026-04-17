#ifndef PIEZAS_H
#define PIEZAS_H

#include <stdbool.h>

struct Juego;

/*
 * Estructura que representa una pieza en el tablero.
 * tipo: 'P'=Peón, 'C'=Caballo, 'A'=Alfil, 'T'=Torre, 'Q'=Reina, 'R'=Rey
 * hp: puntos de vida de la pieza.
 * x, y: posición actual en el tablero.
 */
typedef struct {
    char tipo;
    int hp;
    int x, y;
} Pieza;

/*
 * Estructura que representa una celda del tablero.
 * pieza: puntero a la pieza que ocupa la celda, NULL si está vacía.
 */
typedef struct {
    Pieza *pieza;
} Celda;

/*
 * Inicializa las piezas del nivel indicado en el tablero.
 * Ubica al Rey en la última fila y a los enemigos en las primeras filas.
 */
void spawn_nivel(struct Juego *juego, int nivel);

/*
 * Mueve todos los enemigos vivos un paso hacia el Rey.
 * Cada pieza respeta sus reglas de movimiento de ajedrez.
 */
void mover_enemigos(struct Juego *juego);

/*
 * Verifica si algún enemigo ocupa la casilla del Rey.
 * Retorna true si el Rey fue capturado (derrota), false si sigue vivo.
 */
bool verificar_estado_rey(struct Juego *juego);

/*
 * Elimina una pieza del tablero y libera su memoria.
 * Deja la celda vacía (pieza = NULL).
 */
void eliminar_pieza(struct Juego *juego, int x, int y);

/*
 * Retorna la cantidad de enemigos vivos en el tablero.
 */
int contar_enemigos(struct Juego *juego);

#endif