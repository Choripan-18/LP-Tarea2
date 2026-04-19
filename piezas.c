#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

// Coloca la pieza p en (x,y) y actualiza sus coordenadas.
static void colocar_pieza(Juego *juego, Pieza *p, int x, int y) {
    get_celda(juego, x, y)->pieza = p;
    p->x = x;
    p->y = y;
}


// Busca una posición aleatoria libre en la fila indicada, evitando esquinas si se indica.
static int pos_aleatoria_fila(Juego *juego, int fila, bool evitar_esquinas) {
    int W = juego->t->W;
    int intentos = 0;
    int x;
    do {
        x = evitar_esquinas ? 1 + rand() % (W - 2) : rand() % W;
    } while (get_celda(juego, x, fila)->pieza != NULL);
    return x;
}


// Crea una pieza con el tipo y hp indicados, y la coloca en una posición aleatoria de la fila dada.
// Retorna un puntero a la pieza creada, o NULL si no se pudo crear o colocar.
static Pieza* crear_pieza(Juego *juego, char tipo, int hp, int fila, bool evitar_esquinas) {
    int x = pos_aleatoria_fila(juego, fila, evitar_esquinas);

    Pieza *p = malloc(sizeof(Pieza));
    if (!p) return NULL;

    p->tipo = tipo;
    p->hp   = hp;
    colocar_pieza(juego, p, x, fila);
    return p;
}


// Crea las piezas del nivel indicado y las coloca en la zona del tablero correspondiente incluyendo al Rey.
void spawn_nivel(Juego *juego, int nivel) {
    int H = juego->t->H;

    // Rey
    Pieza *rey = malloc(sizeof(Pieza));
    if (!rey) return;
    rey->tipo = 'R';
    rey->hp   = 1;
    int rx = pos_aleatoria_fila(juego, 0, true);
    colocar_pieza(juego, rey, rx, 0);
    juego->jugador = rey;

    // Enemigos según nivel
    if (nivel == 1) {
        for (int i = 0; i < 4; i++) crear_pieza(juego, 'P', 1, H - 2, false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'C', 2, H - 1, false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'A', 2, H - 1, false);
    } else if (nivel == 2) {
        for (int i = 0; i < 4; i++) crear_pieza(juego, 'P', 1, H - 2, false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'C', 2, H - 1, false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'T', 4, H - 1, false);
    } else if (nivel == 3) {
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'P', 1, H - 2, false);
        crear_pieza(juego, 'Q', 3, H - 1, false);
        crear_pieza(juego, 'A', 2, H - 1, false);
        crear_pieza(juego, 'T', 4, H - 1, false);
    }
}

// Elimina la pieza en la posición indicda y libera su memoria.
void eliminar_pieza(Juego *juego, int x, int y) {
    Celda *c = get_celda(juego, x, y);
    if (c->pieza) {
        free(c->pieza);
        c->pieza = NULL;
    }
}

// Cuenta el número de enemigos vivos en el tablero y lo retorna.
int contar_enemigos(Juego *juego) {
    int count = 0;
    for (int y = 0; y < juego->t->H; y++)
        for (int x = 0; x < juego->t->W; x++) {
            Celda *c = get_celda(juego, x, y);
            if (c->pieza && c->pieza->tipo != 'R') count++;
        }
    return count;
}

// Verifica si alguna pieza enemiga ocupa la misma casilla que el Rey.
// Retorna true si el Rey ha sido capturado.
bool verificar_estado_rey(Juego *juego) {
    Pieza *rey = juego->jugador;
    Celda *c = get_celda(juego, rey->x, rey->y);
    return (c->pieza != NULL && c->pieza->tipo != 'R');
}

// Calculadora de signo.
static int signo(int n) {
    return (n > 0) - (n < 0);
}

// Intenta mover la pieza a la posición indicada.
// Se mueve si la casilla de destino está vacía u ocupada por el Rey.
// Retorna true si el movimiento fue exitoso.
static bool intentar_mover(Juego *juego, Pieza *p, int nx, int ny) {
    if (!en_tablero(juego, nx, ny)) return false;
    Celda *dest = get_celda(juego, nx, ny);

    // Verifica que no haya otro enemigo
    if (dest->pieza && dest->pieza->tipo != 'R') return false;

    // Limpia el origen
    get_celda(juego, p->x, p->y)->pieza = NULL;

    // Coloca la pieza en el destino
    colocar_pieza(juego, p, nx, ny);
    return true;
}

// Mueve el peón una casilla hacia el Rey.
// Si el Rey está en diagonal, ataca.
static void mover_peon(Juego *juego, Pieza *p) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    // Intenta atacar
    if (dx != 0 && dy != 0)
        if (intentar_mover(juego, p, p->x + dx, p->y + dy)) return;

    // Movimiento ortogonal hacia el Rey
    if (dy != 0) intentar_mover(juego, p, p->x, p->y + dy);
    else         intentar_mover(juego, p, p->x + dx, p->y);
}

// Mueve el caballo a la casilla de salto que lo acerque más al Rey, evitando otras piezas.
static void mover_caballo(Juego *juego, Pieza *p) {
    Pieza *rey = juego->jugador;

    int saltos[8][2] = {
        {1,2},{2,1},{2,-1},{1,-2},
        {-1,-2},{-2,-1},{-2,1},{-1,2}
    };

    int mejor_x = p->x, mejor_y = p->y;
    double mejor_dist = 1e9;

    for (int i = 0; i < 8; i++) {
        int nx = p->x + saltos[i][0];
        int ny = p->y + saltos[i][1];
        if (!en_tablero(juego, nx, ny)) continue;

        Celda *dest = get_celda(juego, nx, ny);
        if (dest->pieza && dest->pieza->tipo != 'R') continue;

        double dist = sqrt(pow(nx - rey->x, 2) + pow(ny - rey->y, 2));
        if (dist < mejor_dist) {
            mejor_dist = dist;
            mejor_x = nx;
            mejor_y = ny;
        }
    }

    if (mejor_x != p->x || mejor_y != p->y)
        intentar_mover(juego, p, mejor_x, mejor_y);
}

// Movimiento diagonal hasta max_pasos hacia el Rey. Se detiene si encuentra una pieza en el camino.
static void mover_diagonal(Juego *juego, Pieza *p, int max_pasos) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    // Si el rey no está en diagonal, buscar la casilla diagonal más cercana al rey.
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;

    int ux = p->x, uy = p->y;
    for (int paso = 1; paso <= max_pasos; paso++) {
        int nx = ux + dx * paso;
        int ny = uy + dy * paso;
        if (!en_tablero(juego, nx, ny)) break;

        Celda *dest = get_celda(juego, nx, ny);
        if (dest->pieza) {
            // Si encuentra al Rey, lo captura.
            if (dest->pieza->tipo == 'R')
                intentar_mover(juego, p, nx, ny);
            break;
        }
        ux = nx; uy = ny;
    }
    if (ux != p->x || uy != p->y)
        intentar_mover(juego, p, ux, uy);
}

// Movimiento ortogonal hasta max_pasos hacia el Rey. Se detiene si encuentra una pieza en el camino.
static void mover_ortogonal(Juego *juego, Pieza *p, int max_pasos) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    // Elige la dirección ortogonal que lo acerque más al Rey.
    if (abs(rey->x - p->x) >= abs(rey->y - p->y)) dy = 0;
    else dx = 0;

    int ux = p->x, uy = p->y;
    for (int paso = 1; paso <= max_pasos; paso++) {
        int nx = ux + dx * paso;
        int ny = uy + dy * paso;
        if (!en_tablero(juego, nx, ny)) break;

        Celda *dest = get_celda(juego, nx, ny);
        if (dest->pieza) {
            if (dest->pieza->tipo == 'R')
                intentar_mover(juego, p, nx, ny);
            break;
        }
        ux = nx; uy = ny;
    }
    if (ux != p->x || uy != p->y)
        intentar_mover(juego, p, ux, uy);
}

// Combina movimiento ortogonal y diagonal para la Reina, eligiendo la dirección que lo acerque más al Rey.
static void mover_reina(Juego *juego, Pieza *p) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    // Si está alineada en alguna dirección con el Rey, mover en esa dirección.
    bool ortogonal = (dx == 0 || dy == 0);
    bool diagonal  = (abs(rey->x - p->x) == abs(rey->y - p->y));

    if (diagonal) mover_diagonal(juego, p, 4);
    else if (ortogonal) mover_ortogonal(juego, p, 4);
    else {
        // Si no está alineada, mover en diagonal por defecto.
        mover_diagonal(juego, p, 4);
    }
    (void)dx; (void)dy;
}

// Mueve todas las piezas enemigas. La torre solo se mueve cada 2 turnos.
void mover_enemigos(Juego *juego) {
    juego->turno_enemigos++;

    for (int y = 0; y < juego->t->H; y++) {
        for (int x = 0; x < juego->t->W; x++) {
            Celda *c = get_celda(juego, x, y);
            if (!c->pieza || c->pieza->tipo == 'R') continue;

            Pieza *p = c->pieza;

            switch (p->tipo) {
                case 'P': mover_peon(juego, p);    break;
                case 'C': mover_caballo(juego, p); break;
                case 'A': mover_diagonal(juego, p, 3);  break;
                case 'T':
                    // Torre solo se mueve en turnos pares.
                    if (juego->turno_enemigos % 2 == 0)
                        mover_ortogonal(juego, p, 3);
                    break;
                case 'Q': mover_reina(juego, p);   break;
            }
        }
    }

    // Disminuir cooldown del flash si no está disponible.
    if (juego->flash_cooldown > 0) juego->flash_cooldown--;
}