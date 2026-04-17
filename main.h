#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include "tablero.h"
#include "piezas.h"
#include "armas.h"

/*
 * Estructura principal del juego.
 * Contiene todos los componentes necesarios para el ciclo de juego.
 * t: puntero al tablero actual.
 * arsenal: armas y munición del Rey.
 * jugador: puntero a la pieza Rey.
 * nivel_actual: nivel en curso (1, 2 o 3).
 * turno_enemigos: contador de turnos para gestionar piezas lentas (Torre).
 * flash_cooldown: turnos restantes para poder usar Flash de nuevo.
 */
typedef struct Juego {
    Tablero *t;
    Armas arsenal;
    Pieza *jugador;
    int nivel_actual;
    int turno_enemigos;
    int flash_cooldown;
} Juego;

/*
 * Inicializa el juego completo: crea el tablero del nivel 1,
 * inicializa el arsenal y hace spawn de las piezas.
 * Retorna puntero al Juego creado, o NULL si falla.
 */
Juego* juego_crear(void);

/*
 * Libera toda la memoria del juego: tablero, piezas y estructura.
 * Debe llamarse al terminar o perder.
 */
void juego_liberar(Juego *juego);

/*
 * Avanza al siguiente nivel: libera el tablero actual,
 * crea uno nuevo con el tamaño correspondiente y hace spawn.
 * Recarga la munición de todas las armas.
 */
void juego_avanzar_nivel(Juego *juego);

/*
 * Procesa la entrada del jugador (movimiento o disparo).
 * Retorna true si la acción fue válida y consume el turno,
 * false si fue inválida y no consume el turno.
 */
bool juego_procesar_input(Juego *juego, char input);

/*
 * Mueve al Rey a la posición (nx, ny) si es válida.
 * Recarga 1 bala de escopeta al moverse (máx 2).
 * Retorna true si el movimiento fue exitoso.
 */
bool mover_rey(Juego *juego, int nx, int ny);

/*
 * Bucle principal del juego. Alterna turnos entre el jugador
 * y los enemigos hasta victoria o derrota.
 */
void juego_bucle(Juego *juego);

#endif