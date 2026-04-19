#ifndef TABLERO_H
#define TABLERO_H

struct Juego;

// Estructura del tablero, con dimensiones y celdas dinámicas.
typedef struct Tablero {
    int W, H;
    void ***celdas;
} Tablero;

// Crea un tablero dinámico con las dimensiones dadas, reservando memoria para cada celda.
struct Tablero* tablero_crear(int ancho, int alto);

// Imprime el tablero y el HUD completo en consola, mostrando nivel, munición y piezas.
void tablero_imprimir(struct Juego *juego);

// Libera toda la memoria del heap asociada al tablero.
void tablero_liberar(struct Tablero *tablero);

#endif