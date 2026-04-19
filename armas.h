#ifndef ARMAS_H
#define ARMAS_H

#include <stdbool.h>

struct Juego;

// Tipo de puntero a función para las armas del juego.
// Recibe el juego y la dirección del disparo (dir_x, dir_y).
// Retorna true si la acción fue válida, false si no.

typedef bool (*FuncArma)(struct Juego *j, int dir_x, int dir_y);


// Estructura que contiene el arsenal completo del Rey.
// [0] Escopeta, [1] Francotirador, [2] Granada, [3] Flash
typedef struct {
    int municion_actual[4];
    int municion_maxima[4];
    FuncArma disparar[4];
} Armas;

// Escopeta: dispara en un área cónica en la dirección elegida.
// Retorna true si disparó, false si no había munición.
bool escopeta(struct Juego *j, int dir_x, int dir_y);

// Francotirador: dispara en línea recta sin restricción de distancia en la dirección elegida.
// Retorna true si disparó, false si no había munición.
bool francotirador(struct Juego *j, int dir_x, int dir_y);

// Granada: lanza una granada a 3 casillas de distancia en la dirección elegida.
// Retorna true si disparó, false si no había munición.
bool granada(struct Juego *j, int target_x, int target_y);

// Flash: mueve al Rey 1 casilla en la dirección elegida sin consumir el turno del jugador.
// Solo puede usarse una vez cada 5 turnos.
// Retorna true si se activó, false si está en cooldown o no se puede mover.
bool flash(struct Juego *j, int dir_x, int dir_y);

// Establece las municiones de cada arma del arsenal y asigna los punteros a las funciones correspondientes.
void armas_inicializar(struct Juego *j);

// Recarga todas las armas a su munición máxima. Se llama al avanzar de nivel.
void armas_recargar_todo(struct Juego *j);

#endif