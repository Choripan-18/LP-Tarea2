#ifndef TABLERO_H
#define TABLERO_H

struct Juego;

/*
    Estructura del tablero de juego.
    W: ancho, H: alto.
    celdas[y][x] apunta a Celda* mediante triple puntero void***.
 */
typedef struct Tablero {
    int W, H;
    void ***celdas;
} Tablero;

/*
    Crea un tablero de ancho x alto reservando memoria en el heap.
    Retorna puntero al Tablero creado, o NULL si falla.
 */
struct Tablero* tablero_crear(int ancho, int alto);

/*
    Imprime el tablero y el HUD completo en consola.
    Recibe el juego para acceder a nivel, munición y piezas.
 */
void tablero_imprimir(struct Juego *juego);

/*
 * Libera toda la memoria heap del tablero (celdas, filas, estructura).
 */
void tablero_liberar(struct Tablero *tablero);

#endif