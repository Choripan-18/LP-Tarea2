#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "main.h"
#include "tablero.h"
#include "piezas.h"
#include "armas.h"

/* ─── Tamaños de tablero por nivel ───────────────────── */

static const int TAMAÑOS[4][2] = {
    {0, 0},   /* índice 0 no usado */
    {12, 12}, /* Nivel 1: Plaza principal */
    {8,  8},  /* Nivel 2: Jardín del Rey */
    {6,  6}   /* Nivel 3: Entrada del castillo */
};

/* ─── Creación y liberación ───────────────────────────── */

/*
 * Crea e inicializa el juego completo.
 * Reserva memoria, crea tablero nivel 1, inicializa arsenal y spawn.
 * Retorna puntero al Juego, o NULL si falla la memoria.
 */
Juego* juego_crear(void) {
    srand((unsigned int)time(NULL));

    Juego *j = malloc(sizeof(Juego));
    if (!j) return NULL;

    j->nivel_actual   = 1;
    j->turno_enemigos = 0;
    j->flash_cooldown = 0;

    j->t = tablero_crear(TAMAÑOS[1][0], TAMAÑOS[1][1]);
    if (!j->t) {
        free(j);
        return NULL;
    }

    armas_inicializar(j);
    spawn_nivel(j, 1);

    return j;
}

/*
 * Libera toda la memoria del juego.
 * Libera piezas del tablero, el tablero y la estructura Juego.
 * Debe llamarse al terminar o perder.
 */
void juego_liberar(Juego *juego) {
    if (!juego) return;

    /* Liberar todas las piezas vivas en el tablero */
    if (juego->t) {
        for (int y = 0; y < juego->t->H; y++)
            for (int x = 0; x < juego->t->W; x++) {
                Celda *c = (Celda*)juego->t->celdas[y][x];
                if (c->pieza) {
                    free(c->pieza);
                    c->pieza = NULL;
                }
            }
        tablero_liberar(juego->t);
    }

    free(juego);
}

/*
 * Avanza al siguiente nivel.
 * Libera el tablero actual, crea uno nuevo, recarga armas y hace spawn.
 */
void juego_avanzar_nivel(Juego *juego) {
    /* Liberar piezas del tablero actual */
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

    juego->t = tablero_crear(TAMAÑOS[juego->nivel_actual][0],
                              TAMAÑOS[juego->nivel_actual][1]);
    armas_recargar_todo(juego);
    spawn_nivel(juego, juego->nivel_actual);
}

/* ─── Movimiento del Rey ──────────────────────────────── */

/*
 * Mueve al Rey a (nx, ny) si la posición es válida y está libre.
 * Recarga 1 bala de escopeta al moverse (máx 2).
 * Retorna true si el movimiento fue exitoso.
 */
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

    /* Recargar escopeta al moverse */
    if (juego->arsenal.municion_actual[0] < juego->arsenal.municion_maxima[0])
        juego->arsenal.municion_actual[0]++;

    return true;
}

/* ─── Procesamiento de input ──────────────────────────── */

/*
 * Convierte una tecla de dirección a desplazamiento (dx, dy).
 * Q=arriba-izq, W=arriba, E=arriba-der, A=izq,
 * D=der, Z=abajo-izq, X=abajo, C=abajo-der.
 * Retorna true si la tecla era válida.
 */
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

/*
 * Procesa el input del jugador.
 * Movimiento (QWEADXZC): mueve al Rey.
 * Disparo (1-4): solicita dirección y llama al arma correspondiente.
 * Retorna true si la acción fue válida y consume turno,
 * false si fue inválida (no consume turno).
 */
bool juego_procesar_input(Juego *juego, char input) {
    int dx = 0, dy = 0;

    /* Movimiento */
    if (tecla_a_dir(input, &dx, &dy)) {
        Pieza *rey = juego->jugador;
        return mover_rey(juego, rey->x + dx, rey->y + dy);
    }

    /* Disparo */
    if (input >= '1' && input <= '4') {
        int arma = input - '1';

        /* Flash (arma 4) no necesita dirección si no hay munición */
        if (arma == 3 && juego->flash_cooldown > 0) {
            printf("Flash en recarga! Turnos restantes: %d\n",
                   juego->flash_cooldown);
            return false;
        }

        printf("Direccion (Q,W,E,A,D,Z,X,C): ");
        char dir;
        scanf(" %c", &dir);

        if (!tecla_a_dir(dir, &dx, &dy)) {
            printf("Direccion invalida!\n");
            return false;
        }

        bool disparo = juego->arsenal.disparar[arma](juego, dx, dy);

        /* Flash no consume turno si fue exitoso */
        if (arma == 3 && disparo) return false;

        return disparo;
    }

    printf("Accion invalida!\n");
    return false;
}

/* ─── Bucle principal ─────────────────────────────────── */

/*
 * Bucle principal del juego.
 * Alterna entre turno del jugador y turno de enemigos.
 * Termina por victoria (limpiar nivel 3) o derrota (Rey capturado).
 */
void juego_bucle(Juego *juego) {
    while (1) {
        tablero_imprimir(juego);

        /* Mostrar cooldown del Flash si está activo */
        if (juego->flash_cooldown > 0)
            printf("[4] Flash en recarga (%d turnos)\n", juego->flash_cooldown);
        else
            printf("[4] Flash listo!\n");

        /* Input del jugador */
        printf("> Ingrese accion: ");
        char input;
        scanf(" %c", &input);

        bool turno_valido = juego_procesar_input(juego, input);
        if (!turno_valido) continue;

        /* Verificar si el Rey cayó tras su propio movimiento */
        if (verificar_estado_rey(juego)) {
            tablero_imprimir(juego);
            printf("\n*** EL REY HA SIDO CAPTURADO. FIN DEL JUEGO ***\n");
            return;
        }

        /* Verificar si el nivel fue completado */
        if (contar_enemigos(juego) == 0) {
            if (juego->nivel_actual == 3) {
                tablero_imprimir(juego);
                printf("\n*** VICTORIA! EL REY HA RECUPERADO EL CASTILLO ***\n");
                return;
            }
            juego_avanzar_nivel(juego);
            continue;
        }

        /* Turno de los enemigos */
        printf("\nLas piezas avanzan...\n");
        mover_enemigos(juego);

        /* Verificar derrota tras movimiento enemigo */
        if (verificar_estado_rey(juego)) {
            tablero_imprimir(juego);
            printf("\n*** EL REY HA SIDO CAPTURADO. FIN DEL JUEGO ***\n");
            return;
        }

        /* Verificar nivel completado tras turno enemigo (por si acaso) */
        if (contar_enemigos(juego) == 0) {
            if (juego->nivel_actual == 3) {
                tablero_imprimir(juego);
                printf("\n*** VICTORIA! EL REY HA RECUPERADO EL CASTILLO ***\n");
                return;
            }
            juego_avanzar_nivel(juego);
        }
    }
}

/* ─── Main ────────────────────────────────────────────── */

/*
 * Punto de entrada del programa.
 * Crea el juego, ejecuta el bucle principal y libera la memoria.
 */
int main(void) {
    printf("=== REY DESTRONADO ===\n");
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