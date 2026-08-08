# Portón automático ESP32 — control local v2.3

Proyecto nativo para ESP-IDF. No utiliza Arduino, Wi-Fi ni MQTT.

## Comportamiento al encender

- Entra en `INICIO` durante 1 segundo, con el LED blanco de GPIO33 encendido.
- Durante `INICIO`, todos los relés permanecen apagados y se estabilizan las
  lecturas de los finales de carrera, sensor IR y botón.
- Si el final de cerrado está activo, inicia en `CERRADO`.
- Si el portón está abierto o en una posición intermedia, inicia `CERRANDO`
  hasta encontrar el final de cerrado.
- Si el sensor IR está obstruido al encender, permanece en `OBSTRUIDO` y
  continúa el cierre inicial después de confirmar que la zona está libre.
- Si ambos finales aparecen activos simultáneamente, entra en `ERROR` y corta
  el relé maestro.

El cierre inicial conserva el tiempo máximo de movimiento de 30 segundos.

## Pines

| GPIO | Función |
|---|---|
| 23 | LED CERRADO |
| 25 | LED ABRIENDO |
| 26 | LED ABIERTO |
| 27 | LED ESPERANDO_CIERRE |
| 32 | LED CERRANDO |
| 33 | LED blanco INICIO |
| 13 | LED OBSTRUIDO |
| 14 | LED ERROR |
| 4 | Relé maestro de potencia |
| 16 | Relé de dirección ABRIR |
| 17 | Relé de dirección CERRAR |
| 18 | Final ABIERTO |
| 19 | Final CERRADO |
| 21 | Sensor infrarrojo |
| 22 | Botón de ciclo automático |

El antiguo estado `DETENIDO` continúa eliminado. GPIO33 se utiliza únicamente
para indicar el nuevo estado `INICIO`.

## Compilar y flashear

Desde una terminal ESP-IDF 6.0.2:

```powershell
idf.py fullclean
idf.py set-target esp32
idf.py build
idf.py -p COM5 flash monitor
```

Cambiar `COM5` si el ESP32 utiliza otro puerto.

## Primera prueba

Probar sin conectar el motor:

1. Con el final cerrado activo, debe encender el LED CERRADO.
2. Sin ningún final activo, debe pasar a CERRANDO.
3. Al activar el final cerrado, debe cortar el relé maestro y quedar CERRADO.
4. Si el IR detecta un obstáculo, debe cortar inmediatamente la potencia.
5. Ambos relés de dirección nunca deben quedar activos simultáneamente.
