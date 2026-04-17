#include <stdio.h>
#include <stdlib.h>
#include "armas.h"
#include "piezas.h"
#include "main.h"

/* ─── Auxiliares internas ─────────────────────────────── */

/*
 * Retorna la Celda en (x, y) casteando el void***.
 * Uso interno para no repetir el casteo.
 */
static Celda* get_celda(Juego *juego, int x, int y) {
    return (Celda*)juego->t->celdas[y][x];
}

/*
 * Verifica si (x, y) está dentro de los límites del tablero.
 */
static bool en_tablero(Juego *juego, int x, int y) {
    return x >= 0 && x < juego->t->W && y >= 0 && y < juego->t->H;
}

/*
 * Aplica daño a la pieza en (x, y) si existe y no es el Rey.
 * Si la pieza queda con hp <= 0 la elimina e imprime mensaje.
 */
static void aplicar_daño(Juego *juego, int x, int y, int daño) {
    if (!en_tablero(juego, x, y)) return;
    Celda *c = get_celda(juego, x, y);
    if (!c->pieza || c->pieza->tipo == 'R') return;

    Pieza *p = c->pieza;
    p->hp -= daño;
    printf("IMPACTO!! - %d HP infligido a pieza '%c' en (%d,%d). HP: %d/%d\n",
           daño, p->tipo, x + 1, y + 1, p->hp < 0 ? 0 : p->hp,
           p->hp + daño);

    if (p->hp <= 0) {
        printf("La pieza '%c' ha caido!!\n", p->tipo);
        eliminar_pieza(juego, x, y);
    }
}

/* ─── Armas ───────────────────────────────────────────── */

/*
 * Escopeta: inflige 2 de daño a la primera pieza a distancia 1
 * en la dirección (dir_x, dir_y), y 1 de daño a las 3 casillas
 * inmediatamente detrás de ella.
 * Munición máxima: 2. Se recarga al moverse.
 * Retorna true si disparó, false si no había munición.
 */
bool escopeta(Juego *j, int dir_x, int dir_y) {
    if (j->arsenal.municion_actual[0] <= 0) {
        printf("Sin municion en Escopeta!\n");
        return false;
    }

    j->arsenal.municion_actual[0]--;

    Pieza *rey = j->jugador;
    int x1 = rey->x + dir_x;
    int y1 = rey->y + dir_y;

    /* Daño principal: primera casilla */
    aplicar_daño(j, x1, y1, 2);

    /* Daño secundario: 3 casillas detrás de la primera */
    for (int i = 2; i <= 4; i++)
        aplicar_daño(j, rey->x + dir_x * i, rey->y + dir_y * i, 1);

    return true;
}

/*
 * Sniper: dispara en línea recta infinita en dirección (dir_x, dir_y).
 * Inflige 3 de daño a la primera pieza encontrada. No atraviesa.
 * Munición máxima: 1.
 * Retorna true si disparó, false si no había munición.
 */
bool francotirador(Juego *j, int dir_x, int dir_y) {
    if (j->arsenal.municion_actual[1] <= 0) {
        printf("Sin municion en Sniper!\n");
        return false;
    }

    j->arsenal.municion_actual[1]--;

    Pieza *rey = j->jugador;
    int x = rey->x + dir_x;
    int y = rey->y + dir_y;

    while (en_tablero(j, x, y)) {
        Celda *c = get_celda(j, x, y);
        if (c->pieza && c->pieza->tipo != 'R') {
            aplicar_daño(j, x, y, 3);
            return true;
        }
        x += dir_x;
        y += dir_y;
    }

    printf("El disparo no impacto a nadie.\n");
    return true;
}

/*
 * Granada: se lanza a exactamente 3 casillas en dirección (dir_x, dir_y).
 * Genera explosión en área 3x3 centrada en el impacto, 2 de daño a todo.
 * Munición máxima: 2.
 * Retorna true si disparó, false si no había munición.
 */
bool granada(Juego *j, int dir_x, int dir_y) {
    if (j->arsenal.municion_actual[2] <= 0) {
        printf("Sin municion en Granada!\n");
        return false;
    }

    j->arsenal.municion_actual[2]--;

    Pieza *rey = j->jugador;
    int cx = rey->x + dir_x * 3;
    int cy = rey->y + dir_y * 3;

    printf("Granada lanzada a (%d, %d)!\n", cx + 1, cy + 1);

    /* Explosión 3x3 centrada en (cx, cy) */
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
            aplicar_daño(j, cx + dx, cy + dy, 2);

    return true;
}

/*
 * Flash: mueve al Rey 1 casilla en dirección (dir_x, dir_y)
 * sin consumir el turno del jugador.
 * Solo puede usarse cuando flash_cooldown == 0.
 * Al usarse, establece flash_cooldown = 5 (recarga en 5 turnos).
 * Retorna true si se activó, false si está en cooldown.
 */
bool especial(Juego *j, int dir_x, int dir_y) {
    if (j->flash_cooldown > 0) {
        printf("Flash en recarga! Turnos restantes: %d\n", j->flash_cooldown);
        return false;
    }

    Pieza *rey = j->jugador;
    int nx = rey->x + dir_x;
    int ny = rey->y + dir_y;

    if (!en_tablero(j, nx, ny)) {
        printf("Movimiento Flash fuera del tablero!\n");
        return false;
    }

    Celda *dest = (Celda*)j->t->celdas[ny][nx];
    if (dest->pieza && dest->pieza->tipo != 'R') {
        printf("Casilla ocupada, Flash cancelado!\n");
        return false;
    }

    /* Mover al Rey sin consumir turno */
    get_celda(j, rey->x, rey->y)->pieza = NULL;
    dest->pieza = rey;
    rey->x = nx;
    rey->y = ny;

    j->flash_cooldown = 5;
    j->arsenal.municion_actual[3] = 0;

    printf("FLASH! El Rey se desplaza a (%d, %d). Recarga en 5 turnos.\n",
           nx + 1, ny + 1);

    return true;
}

/* ─── Inicialización ──────────────────────────────────── */

/*
 * Inicializa el arsenal: establece municiones máximas y actuales,
 * y asigna los punteros a función al arreglo disparar[].
 * [0]=Escopeta, [1]=Sniper, [2]=Granada, [3]=Flash
 */
void armas_inicializar(Juego *j) {
    j->arsenal.municion_maxima[0] = 2;
    j->arsenal.municion_maxima[1] = 1;
    j->arsenal.municion_maxima[2] = 2;
    j->arsenal.municion_maxima[3] = 1;

    j->arsenal.municion_actual[0] = 2;
    j->arsenal.municion_actual[1] = 1;
    j->arsenal.municion_actual[2] = 2;
    j->arsenal.municion_actual[3] = 1;

    /* Asignación de punteros a función — obligatorio por enunciado */
    j->arsenal.disparar[0] = escopeta;
    j->arsenal.disparar[1] = francotirador;
    j->arsenal.disparar[2] = granada;
    j->arsenal.disparar[3] = especial;
}

/*
 * Recarga todas las armas a su munición máxima.
 * Se llama al avanzar de nivel.
 */
void armas_recargar_todo(Juego *j) {
    for (int i = 0; i < 4; i++)
        j->arsenal.municion_actual[i] = j->arsenal.municion_maxima[i];
    printf("Municion recargada al maximo!\n");
}