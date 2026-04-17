#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "piezas.h"
#include "main.h"

/* ─── Auxiliares internas ─────────────────────────────── */

/*
 * Retorna la Celda en (x, y) casteando el void***.
 * Uso interno para no repetir el casteo en todo el código.
 */
static Celda* get_celda(Juego *juego, int x, int y) {
    return (Celda*)juego->t->celdas[y][x];
}

/*
 * Verifica si (x, y) está dentro de los límites del tablero.
 * Retorna true si es válida.
 */
static bool en_tablero(Juego *juego, int x, int y) {
    return x >= 0 && x < juego->t->W && y >= 0 && y < juego->t->H;
}

/*
 * Coloca una pieza en la celda (x, y) del tablero.
 * Actualiza las coordenadas de la pieza.
 */
static void colocar_pieza(Juego *juego, Pieza *p, int x, int y) {
    get_celda(juego, x, y)->pieza = p;
    p->x = x;
    p->y = y;
}

/*
 * Busca una posición aleatoria libre en la fila indicada.
 * Excluye las esquinas (x=0 y x=W-1) si exclude_corners es true.
 * Retorna la x encontrada, o -1 si no hay lugar.
 */
static int pos_aleatoria_fila(Juego *juego, int fila, bool exclude_corners) {
    int W = juego->t->W;
    int intentos = 0;
    int x;
    do {
        x = exclude_corners ? 1 + rand() % (W - 2) : rand() % W;
        intentos++;
    } while (get_celda(juego, x, fila)->pieza != NULL && intentos < 100);
    return (intentos < 100) ? x : -1;
}

/* ─── Spawn ───────────────────────────────────────────── */

/*
 * Crea una pieza con el tipo y hp indicados y la coloca en (x, y).
 * Retorna puntero a la pieza creada.
 */
static Pieza* crear_pieza(Juego *juego, char tipo, int hp, int fila, bool exclude_corners) {
    int x = pos_aleatoria_fila(juego, fila, exclude_corners);
    if (x == -1) return NULL;

    Pieza *p = malloc(sizeof(Pieza));
    if (!p) return NULL;

    p->tipo = tipo;
    p->hp   = hp;
    colocar_pieza(juego, p, x, fila);
    return p;
}

/*
 * Inicializa las piezas del nivel indicado en el tablero.
 * Rey: última fila, sin esquinas. Enemigos: primera fila (peones en segunda).
 * Nivel 1: 4 peones, 2 caballos, 2 alfiles.
 * Nivel 2: 4 peones, 2 caballos, 2 torres.
 * Nivel 3: 2 peones, 1 reina, 1 alfil, 1 torre.
 */
void spawn_nivel(Juego *juego, int nivel) {
    int H = juego->t->H;

    /* Crear y colocar al Rey en la última fila */
    Pieza *rey = malloc(sizeof(Pieza));
    if (!rey) return;
    rey->tipo = 'R';
    rey->hp   = 10;
    int rx = pos_aleatoria_fila(juego, 0, true);
    colocar_pieza(juego, rey, rx, 0);
    juego->jugador = rey;

    /* Spawn según nivel */
    if (nivel == 1) {
        for (int i = 0; i < 4; i++) crear_pieza(juego, 'P', 1, 1,     false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'C', 2, H - 1, false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'A', 2, H - 1, false);
    } else if (nivel == 2) {
        for (int i = 0; i < 4; i++) crear_pieza(juego, 'P', 1, 1,     false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'C', 2, H - 1, false);
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'T', 4, H - 1, false);
    } else if (nivel == 3) {
        for (int i = 0; i < 2; i++) crear_pieza(juego, 'P', 1, 1,     false);
        crear_pieza(juego, 'Q', 3, H - 1, false);
        crear_pieza(juego, 'A', 2, H - 1, false);
        crear_pieza(juego, 'T', 4, H - 1, false);
    }
}

/* ─── Utilidades públicas ─────────────────────────────── */

/*
 * Elimina la pieza en (x, y): libera su memoria y vacía la celda.
 */
void eliminar_pieza(Juego *juego, int x, int y) {
    Celda *c = get_celda(juego, x, y);
    if (c->pieza) {
        free(c->pieza);
        c->pieza = NULL;
    }
}

/*
 * Cuenta y retorna el número de enemigos vivos en el tablero.
 */
int contar_enemigos(Juego *juego) {
    int count = 0;
    for (int y = 0; y < juego->t->H; y++)
        for (int x = 0; x < juego->t->W; x++) {
            Celda *c = get_celda(juego, x, y);
            if (c->pieza && c->pieza->tipo != 'R') count++;
        }
    return count;
}

/*
 * Verifica si algún enemigo ocupa la casilla del Rey.
 * Retorna true si el Rey fue capturado (derrota).
 */
bool verificar_estado_rey(Juego *juego) {
    Pieza *rey = juego->jugador;
    Celda *c = get_celda(juego, rey->x, rey->y);
    return (c->pieza != NULL && c->pieza->tipo != 'R');
}

/* ─── Movimiento de enemigos ──────────────────────────── */

/*
 * Calcula el signo de n: retorna -1, 0 o 1.
 */
static int signo(int n) {
    return (n > 0) - (n < 0);
}

/*
 * Intenta mover la pieza p a (nx, ny).
 * Solo mueve si la celda destino está vacía o es el Rey.
 * Retorna true si el movimiento fue exitoso.
 */
static bool intentar_mover(Juego *juego, Pieza *p, int nx, int ny) {
    if (!en_tablero(juego, nx, ny)) return false;
    Celda *dest = get_celda(juego, nx, ny);

    /* No puede moverse a una celda ocupada por otro enemigo */
    if (dest->pieza && dest->pieza->tipo != 'R') return false;

    /* Limpiar celda origen */
    get_celda(juego, p->x, p->y)->pieza = NULL;

    /* Mover a destino */
    colocar_pieza(juego, p, nx, ny);
    return true;
}

/*
 * Mueve el Peón 1 casilla hacia el Rey.
 * Si el Rey está en diagonal frontal a 1 casilla, ataca en diagonal.
 * Si el Rey está directamente adelante, avanza ortogonal.
 */
static void mover_peon(Juego *juego, Pieza *p) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    /* Intentar diagonal primero si el Rey está en diagonal */
    if (dx != 0 && dy != 0)
        if (intentar_mover(juego, p, p->x + dx, p->y + dy)) return;

    /* Movimiento ortogonal hacia el Rey */
    if (dy != 0) intentar_mover(juego, p, p->x, p->y + dy);
    else         intentar_mover(juego, p, p->x + dx, p->y);
}

/*
 * Mueve el Caballo en L minimizando la distancia al Rey.
 * Evalúa los 8 saltos posibles y elige el que deja menor distancia.
 * Puede saltar sobre otras piezas.
 */
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

/*
 * Mueve el Alfil en diagonal hasta max_pasos casillas hacia el Rey.
 * Se detiene si encuentra una pieza en el camino.
 */
static void mover_diagonal(Juego *juego, Pieza *p, int max_pasos) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    /* Si el Rey no está en diagonal, buscar la diagonal más cercana */
    if (dx == 0) dx = 1;
    if (dy == 0) dy = 1;

    int ux = p->x, uy = p->y;
    for (int paso = 1; paso <= max_pasos; paso++) {
        int nx = ux + dx * paso;
        int ny = uy + dy * paso;
        if (!en_tablero(juego, nx, ny)) break;

        Celda *dest = get_celda(juego, nx, ny);
        if (dest->pieza) {
            /* Si es el Rey, avanza igual (lo captura) */
            if (dest->pieza->tipo == 'R')
                intentar_mover(juego, p, nx, ny);
            break;
        }
        ux = nx; uy = ny;
    }
    if (ux != p->x || uy != p->y)
        intentar_mover(juego, p, ux, uy);
}

/*
 * Mueve la Torre en línea ortogonal hasta max_pasos hacia el Rey.
 * Se detiene si encuentra una pieza en el camino.
 */
static void mover_ortogonal(Juego *juego, Pieza *p, int max_pasos) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    /* Torre solo se mueve ortogonal: elegir eje dominante */
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

/*
 * Mueve la Reina: combina movimiento ortogonal y diagonal
 * hasta 4 casillas, eligiendo la dirección más cercana al Rey.
 */
static void mover_reina(Juego *juego, Pieza *p) {
    Pieza *rey = juego->jugador;
    int dx = signo(rey->x - p->x);
    int dy = signo(rey->y - p->y);

    /* Si está alineada ortogonal o diagonal, moverse en esa dirección */
    bool ortogonal = (dx == 0 || dy == 0);
    bool diagonal  = (abs(rey->x - p->x) == abs(rey->y - p->y));

    if (diagonal)       mover_diagonal(juego, p, 4);
    else if (ortogonal) mover_ortogonal(juego, p, 4);
    else {
        /* No alineada: mover en diagonal hacia el Rey */
        mover_diagonal(juego, p, 4);
    }
    (void)dx; (void)dy;
}

/*
 * Mueve todos los enemigos vivos un paso hacia el Rey.
 * La Torre solo se mueve una vez cada 2 turnos de enemigos.
 * Incrementa el contador de turnos.
 */
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
                    /* Torre se mueve solo en turnos pares */
                    if (juego->turno_enemigos % 2 == 0)
                        mover_ortogonal(juego, p, 3);
                    break;
                case 'Q': mover_reina(juego, p);   break;
            }
        }
    }

    /* Decrementar cooldown del Flash si está activo */
    if (juego->flash_cooldown > 0) juego->flash_cooldown--;
}