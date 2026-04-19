Tarea 2: La revolución del rey
Nombre: Matias Ibañez
Rol: 202473019-3

Instrucciones por terminal:
- Para compilar:
    make

- Para limpiar los archivos compilados:
    make clean

- Para verificar memory leaks:
    make valgrind

- Ejecución: 
    ./revolucion_francesa

Los controles siguen la definición del enunciado.

Arma especial: Flash
    El Rey se desplaza una casilla en la dirección indicada
    sin consumir el turno del jugador. Luego de usarse entra en 
    cooldown por 5 turnos.
- Sobre la implementación: El movimiento especial ocupa la 
misma implementación normal del movimiento, pero al momento de procesar
el input, la funcion retorna false igualmente para no consumir el turno.
El cooldown se maneja dentro de la lógica de los turnos.