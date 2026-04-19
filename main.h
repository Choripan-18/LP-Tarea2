#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>
#include "tablero.h"
#include "piezas.h"
#include "armas.h"


// Estructura principal del juego, contiene el tablero, arsenal,
// estado del jugador y variables de control de niveles y turnos.
typedef struct Juego {
    Tablero *t;
    Armas arsenal;
    Pieza *jugador;
    int nivel_actual;
    int turno_enemigos;
    int flash_cooldown;
} Juego;

// Inicializa el juego completo: crea el tablero del nivel 1, inicializa el arsenal y hace spawn de las piezas.
// Retorna un puntero al Juego creado, o NULL si hubo error.
Juego* juego_crear(void);


// Libera toda la memoria del juego. Es llamado al terminar el juego o perder.
void juego_liberar(Juego *juego);


// Avanza al siguiente nivel.
void juego_avanzar_nivel(Juego *juego);

// Procesa el input del jugador, ya sea movimiento o uso de arma.
// Retorna true si la acción fue válida y consume turno, false si fue inválida y no consume turno.
bool juego_procesar_input(Juego *juego, char input);

// Intenta mover al rey en la dirección indicada. Retorna true si fue exitoso, false si el movimiento es inválido.
bool mover_rey(Juego *juego, int nx, int ny);

// Bucle principal del juego. Se ejecuta hasta que el jugador gane o pierda.
void juego_bucle(Juego *juego);

#endif