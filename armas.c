#include <stdio.h>
#include <stdlib.h>
#include "armas.h"
#include "piezas.h"
#include "main.h"

// Retorna la celda en (x, y).
static Celda* get_celda(Juego *juego, int x, int y) {
    return (Celda*)juego->t->celdas[y][x];
}

// Verifica si (x, y) está dentro de los límites del tablero.
static bool en_tablero(Juego *juego, int x, int y) {
    return x >= 0 && x < juego->t->W && y >= 0 && y < juego->t->H;
}

// Aplica daño a la pieza en (x, y) si existe y no es el Rey.
// Si la pieza queda con hp <= 0 la elimina e imprime mensaje.
static void aplicar_daño(Juego *juego, int x, int y, int daño) {
    if (!en_tablero(juego, x, y)) return;
    Celda *c = get_celda(juego, x, y);
    if (!c->pieza || c->pieza->tipo == 'R') return;

    Pieza *p = c->pieza;
    p->hp -= daño;
    printf("Impacto! - %d HP infligido a pieza '%c' en (%d,%d). HP: %d/%d\n",
           daño, p->tipo, x + 1, y + 1, p->hp < 0 ? 0 : p->hp,
           p->hp + daño);

    if (p->hp <= 0) {
        printf("La pieza '%c' ha caido!\n", p->tipo);
        eliminar_pieza(juego, x, y);
    }
}


// Escopeta: inflige 2 de daño a la primera pieza a distancia 1
// en la dirección (dir_x, dir_y), y 1 de daño a las 3 casillas
// inmediatamente detrás de ella.
// Munición máxima: 2. Se recarga al moverse.
// Retorna true si disparó, false si no había munición.
bool escopeta(Juego *j, int dir_x, int dir_y) {
    if (j->arsenal.municion_actual[0] <= 0) {
        printf("Sin munición en Escopeta! :(\n");
        return false;
    }

    j->arsenal.municion_actual[0]--;

    Pieza *rey = j->jugador;
    int x1 = rey->x + dir_x;
    int y1 = rey->y + dir_y;

    // Daño al primer objetivo.
    aplicar_daño(j, x1, y1, 2);

    // Daño a las 3 casillas detrás del objetivo.
    aplicar_daño(j, x1 + dir_x - dir_y, y1 + dir_y - dir_x, 1);     
    aplicar_daño(j, x1 + dir_x, y1 + dir_y, 1);                     
    aplicar_daño(j, x1 + dir_x + dir_y, y1 + dir_y + dir_x, 1); 
    return true;    
}


// Francotirador: dispara en línea recta infinita en dirección (dir_x, dir_y).
// Inflige 3 de daño a la primera pieza encontrada. No atraviesa.
// Munición máxima: 1.
// Retorna true si disparó, false si no había munición.
bool francotirador(Juego *j, int dir_x, int dir_y) {
    if (j->arsenal.municion_actual[1] <= 0) {
        printf("Sin munición en Francotirador! :(\n");
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

    printf("El disparo no impactó a nadie :/.\n");
    return true;
}


// Granada: se lanza a 3 casillas en dirección (dir_x, dir_y).
// Genera explosión en área 3x3 centrada en el impacto, 2 de daño a todos los enemigos.
// Munición máxima: 2.
// Retorna true si disparó, false si no había munición.
bool granada(Juego *j, int dir_x, int dir_y) {
    if (j->arsenal.municion_actual[2] <= 0) {
        printf("Sin munición en Granada! :(\n");
        return false;
    }

    j->arsenal.municion_actual[2]--;

    Pieza *rey = j->jugador;
    int cx = rey->x + dir_x * 3;
    int cy = rey->y + dir_y * 3;

    printf("Granada lanzada a (%d, %d)!\n", cx + 1, cy + 1);

    // Explosion 3x3 centrada en (cx, cy)
    for (int dy = -1; dy <= 1; dy++)
        for (int dx = -1; dx <= 1; dx++)
            aplicar_daño(j, cx + dx, cy + dy, 2);

    return true;
}


// Flash: mueve al Rey 1 casilla en dirección (dir_x, dir_y)
// sin consumir el turno del jugador.
// Solo puede usarse cuando flash_cooldown == 0.
// Al usarse, establece flash_cooldown = 5 (recarga en 5 turnos).
// Retorna true si se activó, false si está en cooldown.
bool flash(Juego *j, int dir_x, int dir_y) {
    if (j->flash_cooldown > 0) {
        printf("Flash en recarga! Turnos restantes: %d\n", j->flash_cooldown);
        return false;
    }

    Pieza *rey = j->jugador;
    int nx = rey->x + dir_x;
    int ny = rey->y + dir_y;

    if (!en_tablero(j, nx, ny)) {
        printf("No puedes moverte fuera del tablero!\n");
        return false;
    }

    Celda *dest = (Celda*)j->t->celdas[ny][nx];
    if (dest->pieza && dest->pieza->tipo != 'R') {
        printf("No puedes moverte a una casilla ocupada!\n");
        return false;
    }

    // Mover al Rey sin consumir el turno.
    get_celda(j, rey->x, rey->y)->pieza = NULL;
    dest->pieza = rey;
    rey->x = nx;
    rey->y = ny;

    j->flash_cooldown = 5;
    j->arsenal.municion_actual[3] = 0;

    printf("Te has movido con Flash a (%d, %d)!\n", nx + 1, ny + 1);

    return true;
}

// Establece las municiones de cada arma del arsenal y asigna los punteros a las funciones correspondientes.
void armas_inicializar(Juego *j) {
    j->arsenal.municion_maxima[0] = 2;
    j->arsenal.municion_maxima[1] = 1;
    j->arsenal.municion_maxima[2] = 2;
    j->arsenal.municion_maxima[3] = 1;

    j->arsenal.municion_actual[0] = 2;
    j->arsenal.municion_actual[1] = 1;
    j->arsenal.municion_actual[2] = 2;
    j->arsenal.municion_actual[3] = 1;

    // Asignar punteros
    j->arsenal.disparar[0] = escopeta;
    j->arsenal.disparar[1] = francotirador;
    j->arsenal.disparar[2] = granada;
    j->arsenal.disparar[3] = flash;
}


// Recarga todas las armas a su munición máxima.
void armas_recargar_todo(Juego *j) {
    for (int i = 0; i < 4; i++)
        j->arsenal.municion_actual[i] = j->arsenal.municion_maxima[i];
    printf("Todas las armas han sido recargadas!\n");
}