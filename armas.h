#ifndef ARMAS_H
#define ARMAS_H

#include <stdbool.h>

struct Juego;

/*
 * Tipo de puntero a función para las armas del juego.
 * Recibe el juego y la dirección del disparo (dir_x, dir_y).
 * Retorna true si la acción fue válida, false si no.
 */
typedef bool (*FuncArma)(struct Juego *j, int dir_x, int dir_y);

/*
 * Estructura que contiene el arsenal completo del Rey.
 * municion_actual: usos disponibles por arma.
 * municion_maxima: tope de usos por arma.
 * disparar: arreglo de punteros a función con las 4 armas.
 *   [0] Escopeta, [1] Sniper, [2] Granada, [3] Flash
 */
typedef struct {
    int municion_actual[4];
    int municion_maxima[4];
    FuncArma disparar[4];
} Armas;

/*
 * Escopeta: inflige 2 de daño a la primera pieza a distancia 1
 * en la dirección elegida, y 1 de daño a las 3 casillas detrás.
 * Munición máxima: 2. Se recarga al moverse.
 * Retorna true si disparó, false si no había munición.
 */
bool escopeta(struct Juego *j, int dir_x, int dir_y);

/*
 * Sniper: alcance infinito en línea recta, 3 de daño a la primera
 * pieza impactada. No atraviesa piezas. Munición máxima: 1.
 * Retorna true si disparó, false si no había munición.
 */
bool francotirador(struct Juego *j, int dir_x, int dir_y);

/*
 * Granada: se lanza a exactamente 3 casillas de distancia.
 * Explosión en área 3x3 centrada en el impacto, 2 de daño a todo.
 * Munición máxima: 2.
 * Retorna true si disparó, false si no había munición.
 */
bool granada(struct Juego *j, int target_x, int target_y);

/*
 * Flash: mueve al Rey 1 casilla en cualquier dirección
 * sin consumir el turno del jugador. Solo puede usarse
 * una vez cada 5 turnos. Munición máxima: 1 (recarga cada 5 turnos).
 * Retorna true si se activó, false si aún está en recarga.
 */
bool especial(struct Juego *j, int dir_x, int dir_y);

/*
 * Inicializa el arsenal con las municiones máximas y asigna
 * los punteros a función a cada ranura del arreglo disparar[].
 */
void armas_inicializar(struct Juego *j);

/*
 * Recarga todas las armas a su munición máxima.
 * Se llama al avanzar de nivel.
 */
void armas_recargar_todo(struct Juego *j);

#endif