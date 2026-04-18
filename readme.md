# Tarea 2 — Rey Destronado
**Curso:** INF-253 Lenguajes de Programación  
**Nombre:** TU NOMBRE COMPLETO  
**Rol:** TU ROL (ej: 202573000-k)  

---

## Descripción

Juego de supervivencia por turnos en consola escrito en C.
El jugador controla al Rey, que debe eliminar todas las piezas
enemigas en 3 niveles para recuperar su castillo.

---

## Compilación

Requiere GCC con soporte C11. Desde la carpeta del proyecto:

```bash
make
```

Para limpiar los archivos compilados:

```bash
make clean
```

Para verificar memory leaks con Valgrind:

```bash
make valgrind
```

---

## Ejecución

```bash
./rey_destronado
```

---

## Controles

### Movimiento (consume turno, recarga escopeta)
| Tecla | Dirección     |
|-------|---------------|
| Q     | Arriba-Izq    |
| W     | Arriba        |
| E     | Arriba-Der    |
| A     | Izquierda     |
| D     | Derecha       |
| Z     | Abajo-Izq     |
| X     | Abajo         |
| C     | Abajo-Der     |

### Disparo (consume turno)
| Tecla | Arma       | Munición | Efecto                              |
|-------|------------|----------|-------------------------------------|
| 1     | Escopeta   | 2/2      | 2 dmg frontal + 1 dmg a 3 detrás   |
| 2     | Sniper     | 1/1      | 3 dmg en línea recta infinita       |
| 3     | Granada    | 2/2      | 2 dmg en área 3x3 a 3 casillas      |
| 4     | Flash      | 1 uso    | Mueve al Rey sin consumir turno     |

---

## Arma Especial — Flash

El Flash permite al Rey desplazarse 1 casilla en cualquier dirección
sin consumir el turno del jugador, permitiendo un movimiento adicional
en la misma ronda.

**Implementación:** Al activarse, mueve al Rey directamente en
`especial()` dentro de `armas.c`, retorna `false` para que
`juego_procesar_input()` en `main.c` no consuma el turno. 
El cooldown se almacena en `juego->flash_cooldown` y se
decrementa en cada turno enemigo dentro de `mover_enemigos()`.

---

## Niveles

| Nivel | Tablero | Enemigos                          |
|-------|---------|-----------------------------------|
| 1     | 12×12   | 4 Peones, 2 Caballos, 2 Alfiles   |
| 2     | 8×8     | 4 Peones, 2 Caballos, 2 Torres    |
| 3     | 6×6     | 2 Peones, 1 Reina, 1 Alfil, 1 Torre |

---

## Estructura del proyecto

```
├── main.c / main.h       — Bucle principal y struct Juego
├── tablero.c / tablero.h — Tablero dinámico con void***
├── piezas.c / piezas.h   — Piezas, spawn y movimiento enemigos
├── armas.c / armas.h     — Arsenal con punteros a función
├── Makefile              — Compilación separada con GCC C11
└── README.md             — Este archivo
```