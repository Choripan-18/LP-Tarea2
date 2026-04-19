#include <stdio.h>
#include <stdlib.h>
#include "tablero.h"
#include "piezas.h"
#include "main.h"

// Crea un tablero dinámico con las dimensiones dadas, reservando memoria para cada celda.
struct Tablero* tablero_crear(int ancho, int alto) {
    Tablero *t = malloc(sizeof(Tablero));
    if (!t) return NULL;

    t->W = ancho;
    t->H = alto;

    // Reservar memoria para las filas
    t->celdas = malloc(sizeof(void**) * alto);
    if (!t->celdas) {
        free(t);
        return NULL;
    }

    // Reservar memoria para cada fila y cada celda
    for (int y = 0; y < alto; y++) {
        t->celdas[y] = malloc(sizeof(void*) * ancho);
        if (!t->celdas[y]) {
            for (int i = 0; i < y; i++) {
                for (int x = 0; x < ancho; x++) free(t->celdas[i][x]);
                free(t->celdas[i]);
            }
            free(t->celdas);
            free(t);
            return NULL;
        }

        // Inicializar cada celda vacía.
        for (int x = 0; x < ancho; x++) {
            Celda *c = malloc(sizeof(Celda));
            if (!c) {
                // Libera la memoria ya reservada en caso de error.
                for (int j = 0; j < x; j++) free(t->celdas[y][j]);
                free(t->celdas[y]);
                for (int i = 0; i < y; i++) {
                    for (int j = 0; j < ancho; j++) free(t->celdas[i][j]);
                    free(t->celdas[i]);
                }
                free(t->celdas);
                free(t);
                return NULL;
            }
            c->pieza = NULL;
            t->celdas[y][x] = c;
        }
    }

    return t;
}

// Imprime el tablero y el HUD completo en consola, mostrando nivel, munición y piezas.
void tablero_imprimir(struct Juego *juego) {
    Tablero *t = juego->t;

    // Contar enemigos restantes en el tablero.
    int enemigos = 0;
    for (int y = 0; y < t->H; y++)
        for (int x = 0; x < t->W; x++) {
            Celda *c = (Celda*)t->celdas[y][x];
            if (c->pieza && c->pieza->tipo != 'R') enemigos++;
        }

    // Nombres de niveles para el HUD.
    const char *nombre_nivel[] = {"", "Plaza principal", "Jardín del Rey", "Entrada del castillo"};

    printf("\n===================================================\n");
    printf("Nivel: %d (%s) | Enemigos restantes: %d\n",
           juego->nivel_actual, nombre_nivel[juego->nivel_actual], enemigos);
    printf("Arsenal: [1] Escopeta (%d/%d) | [2] Sniper (%d/%d)\n",
           juego->arsenal.municion_actual[0], juego->arsenal.municion_maxima[0],
           juego->arsenal.municion_actual[1], juego->arsenal.municion_maxima[1]);
    printf("         [3] Granada  (%d/%d) | [4] Flash (%d/%d)\n",
           juego->arsenal.municion_actual[2], juego->arsenal.municion_maxima[2],
           juego->arsenal.municion_actual[3], juego->arsenal.municion_maxima[3]);
    printf("===================================================\n");

    // Imprimir el tablero desde la fila superior (H-1) hasta la inferior (0).
    for (int y = t->H - 1; y >= 0; y--) {
        printf("%2d ", y + 1);
        for (int x = 0; x < t->W; x++) {
            Celda *c = (Celda*)t->celdas[y][x];
            if (c->pieza)
                printf("[%c]", c->pieza->tipo);
            else
                printf("[ ]");
        }
        printf("\n");
    }

    // Imprimir números de columna debajo del tablero.
    printf("   ");
    for (int x = 0; x < t->W; x++) printf("%2d ", x + 1);
    printf("\n");

    // Instrucciones de acciones y movimiento.
    printf("\nACCIONES: Disparo: [1] Escopeta [2] Sniper [3] Granada [4] Flash\n");
    printf("Movimiento:\n");
    printf("  [Q][W][E]\n");
    printf("  [A] R [D]\n");
    printf("  [Z][X][C]\n");
}

// Libera toda la memoria del heap asociada al tablero.
void tablero_liberar(struct Tablero *tablero) {
    if (!tablero) return;

    for (int y = 0; y < tablero->H; y++) {
        for (int x = 0; x < tablero->W; x++)
            free(tablero->celdas[y][x]);
        free(tablero->celdas[y]);
    }

    free(tablero->celdas);
    free(tablero);
}