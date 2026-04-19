#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "tablero.h"
#include "piezas.h"
#include "armas.h"

// tamaños del tablero por nivel. Indice 0 para control.
static const int tamaños[4] = {0, 12, 8, 6};

// Crea un nuevo juego, inicializa el tablero, el arsenal y hace spawn del nivel 1.
// Retorna un puntero al juego creado, o NULL si hubo error.
Juego* juego_crear(void) {
    srand((unsigned int)time(NULL));

    Juego *j = malloc(sizeof(Juego));
    if (!j) return NULL;

    j->nivel_actual   = 1;
    j->turno_enemigos = 0;
    j->flash_cooldown = 0;

    j->t = tablero_crear(tamaños[1], tamaños[1]);
    if (!j->t) {
        free(j);
        return NULL;
    }

    armas_inicializar(j);
    spawn_nivel(j, 1);

    return j;
}

// Libera toda la memoria del juego. Es llamada al terminar el juego o perder.
void juego_liberar(Juego *juego) {
    if (!juego) return;

    // Libera todas las piezas del tablero, y luego el tablero.
    if (juego->t) {
        for (int y = 0; y < juego->t->H; y++)
            for (int x = 0; x < juego->t->W; x++) {
                Celda *c = (Celda*)juego->t->celdas[y][x];
                if (c->pieza) {
                    free(c->pieza);
                    c->pieza = NULL;
                }
            }
        if (juego->jugador) {
            int rx = juego->jugador->x;
            int ry = juego->jugador->y;
            Celda *celda_rey = (Celda*)juego->t->celdas[ry][rx];
            if (celda_rey->pieza != juego->jugador) {
                free(juego->jugador);
                juego->jugador = NULL;
            }
        }
        tablero_liberar(juego->t);
    }

    free(juego);
}

// Avanza al siguiente nivel.
void juego_avanzar_nivel(Juego *juego) {
    // Libera piezas y tablero para crear el nivel nuevo.
    for (int y = 0; y < juego->t->H; y++)
        for (int x = 0; x < juego->t->W; x++) {
            Celda *c = (Celda*)juego->t->celdas[y][x];
            if (c->pieza) {
                free(c->pieza);
                c->pieza = NULL;
            }
        }

    tablero_liberar(juego->t);
    juego->nivel_actual++;
    juego->turno_enemigos = 0;
    juego->flash_cooldown = 0;

    printf("\n===================================================\n");
    printf("NIVEL %d COMPLETADO!\n", juego->nivel_actual - 1);
    if (juego->nivel_actual == 2)
        printf("El Rey avanza al Jardin...\n");
    else if (juego->nivel_actual == 3)
        printf("El Rey entra en el castillo...\n");
    printf("Recuperando municion total.\n");
    printf("===================================================\n");

    juego->t = tablero_crear(tamaños[juego->nivel_actual], tamaños[juego->nivel_actual]);
    armas_recargar_todo(juego);
    spawn_nivel(juego, juego->nivel_actual);
}


// Mueve al Rey si el movimiento es válido.
// Recarga una munición de la escopeta si no está llena.
// Retorna true si el movimiento fue exitoso, false si fue inválido.
bool mover_rey(Juego *juego, int nx, int ny) {
    if (nx < 0 || nx >= juego->t->W || ny < 0 || ny >= juego->t->H) {
        printf("Movimiento fuera del tablero!\n");
        return false;
    }

    Celda *dest = (Celda*)juego->t->celdas[ny][nx];
    if (dest->pieza && dest->pieza->tipo != 'R') {
        printf("Casilla ocupada!\n");
        return false;
    }

    Pieza *rey = juego->jugador;
    ((Celda*)juego->t->celdas[rey->y][rey->x])->pieza = NULL;
    dest->pieza = rey;
    rey->x = nx;
    rey->y = ny;

    if (juego->arsenal.municion_actual[0] < juego->arsenal.municion_maxima[0]){
        juego->arsenal.municion_actual[0]++;
    }
    return true;
}

// Convierte la letra input del usuario en una dirección para ser usada por el programa.
// Retorna true si la letra corresponde a una dirección válida, false si no.
static bool tecla_a_dir(char tecla, int *dx, int *dy) {
    switch (tecla) {
        case 'Q': case 'q': *dx = -1; *dy =  1; return true;
        case 'W': case 'w': *dx =  0; *dy =  1; return true;
        case 'E': case 'e': *dx =  1; *dy =  1; return true;
        case 'A': case 'a': *dx = -1; *dy =  0; return true;
        case 'D': case 'd': *dx =  1; *dy =  0; return true;
        case 'Z': case 'z': *dx = -1; *dy = -1; return true;
        case 'X': case 'x': *dx =  0; *dy = -1; return true;
        case 'C': case 'c': *dx =  1; *dy = -1; return true;
        default: return false;
    }
}


// Procesa el input del jugador, ya sea movimiento o uso de armas.
// Retorna true si la acción fue válida y consume turno, false si fue inválida y no consume turno.
bool juego_procesar_input(Juego *juego, char input) {
    int dx = 0, dy = 0;

    // Movimiento 
    if (tecla_a_dir(input, &dx, &dy)) {
        Pieza *rey = juego->jugador;
        return mover_rey(juego, rey->x + dx, rey->y + dy);
    }

    // Armas
    if (input >= '1' && input <= '4') {
        int arma = input - '1';

        // Si se selecciona flash, verificar cooldown.
        if (arma == 3 && juego->flash_cooldown > 0) {
            printf("Flash en recarga! Turnos restantes: %d\n", juego->flash_cooldown);
            return false;
        }

        printf("Direccion (Q,W,E,A,D,Z,X,C): ");
        char dir;
        scanf(" %c", &dir);

        if (!tecla_a_dir(dir, &dx, &dy)) {
            printf("Direccion inválida!\n");
            return false;
        }

        bool disparo = juego->arsenal.disparar[arma](juego, dx, dy);

        // Si se usa flash de forma exitosa no consume turno.
        if (arma == 3 && disparo) return false;

        return disparo;
    }

    printf("Accion invalida!\n"); // En caso que no cumpla ninguna de las acciones anteriores.
    return false;
}


// Bucle principal del juego. Se ejecuta hasta que el jugador gane o pierda.
void juego_bucle(Juego *juego) {
    while (true) {
        tablero_imprimir(juego);

        // Input del jugador
        printf("\n> Ingrese accion: ");
        char input;
        scanf(" %c", &input);

        bool turno_valido = juego_procesar_input(juego, input);
        if (!turno_valido) continue;

        // Verificar derrota tras acción del jugador
        if (verificar_estado_rey(juego)) {
            tablero_imprimir(juego);
            printf("\n*** El rey ha sido capturado. Has perdido :( ***\n");
            return;
        }

        // Verificar si el nivel fue completado tras acción del jugador 
        if (contar_enemigos(juego) == 0) {
            if (juego->nivel_actual == 3) {
                tablero_imprimir(juego);
                printf("\n*** Victoria! Has recuperado tu castillo! B) ***\n");
                return;
            }
            juego_avanzar_nivel(juego);
            continue;
        }

        // Turno de los enemigos
        printf("\nLos enemigos se acercan...\n");
        mover_enemigos(juego);

        // Verificar derrota tras movimiento enemigo
        if (verificar_estado_rey(juego)) {
            tablero_imprimir(juego);
            printf("\n*** El rey ha sido capturado. Has perdido :( ***\n");
            return;
        }

        // Verificar nivel completado tras turno enemigo (por si acaso)
        if (contar_enemigos(juego) == 0) {
            if (juego->nivel_actual == 3) {
                tablero_imprimir(juego);
                printf("\n*** Victoria! Has recuperado tu castillo! B) ***\n");
                return;
            }
            juego_avanzar_nivel(juego);
        }
    }
}


// Inicio del programa principal. Crea el juego, ejecuta el bucle principal y libera la memoria al finalizar.
int main(void) {
    printf("Revolución! Tus súbditos se han vuelto contra tí\n");
    printf("Sobrevive la horda y recupera tu castillo!\n\n");

    Juego *juego = juego_crear();
    if (!juego) {
        printf("Error al inicializar el juego.\n");
        return 1;
    }

    juego_bucle(juego);
    juego_liberar(juego);

    return 0;
}