# Minijuego de reacción ESP32 + MQTT

Proyecto para **ESP-IDF 5.x** y la extensión **Espressif IDF** de Visual Studio Code.

## Funcionamiento

1. Pulse y mantenga presionado el botón 1.
2. El LED de espera se enciende durante un tiempo aleatorio de 2 a 5 segundos.
3. Cuando se enciende el LED de salida, suelte el botón 1 y pulse el botón 2.
4. Se publican ambos tiempos, medidos desde el encendido del LED de salida.
5. Soltar el botón 1 o pulsar el botón 2 antes de la señal se registra como salida falsa.

El archivo `sdkconfig.defaults` configura el tick de FreeRTOS a 1000 Hz. El juego
sondea cada 1 ms y valida 15 ms de estabilidad para eliminar rebotes; la marca de
tiempo se toma al comienzo del cambio, por lo que el antirrebote no agrega 15 ms
al resultado.

Ejemplo de resultado MQTT:

```json
{
  "ronda": 7,
  "liberacion_boton_1_ms": 183.421,
  "pulsacion_boton_2_ms": 241.906,
  "salida_falsa": false
}
```

## Conexiones predeterminadas

| Elemento | GPIO | Conexión |
|---|---:|---|
| Botón 1 (inicio/liberación) | 18 | Entre GPIO18 y GND |
| Botón 2 (reacción) | 19 | Entre GPIO19 y GND |
| LED de espera | 25 | GPIO25 -> resistencia 220-330 ohm -> ánodo; cátodo a GND |
| LED de salida | 26 | GPIO26 -> resistencia 220-330 ohm -> ánodo; cátodo a GND |

Los botones usan las resistencias pull-up internas. Los GPIO se pueden cambiar en
`main/config.h`; compruebe el pinout si usa ESP32-C3, S3, C6 u otra placa.

## Configuración y carga desde VS Code

1. Instale y configure la extensión **Espressif IDF**.
2. Abra esta carpeta mediante **File > Open Folder**.
3. Edite `main/config.h` y escriba el SSID, clave Wi-Fi y URI del broker MQTT.
4. En la barra inferior seleccione el modelo de chip, el puerto serie y el perfil.
5. Ejecute **ESP-IDF: Build, Flash and Start a Monitor** desde la paleta de comandos.

También puede usar una terminal ESP-IDF:

```text
idf.py set-target esp32
idf.py build
idf.py -p COM3 flash monitor
```

## Topics

- `juego/reaccion/resultados`: tiempos y salidas falsas, QoS 1.
- `juego/reaccion/estado`: estados del juego y conexión del dispositivo.

Para observarlos con Mosquitto:

```text
mosquitto_sub -h 192.168.1.50 -t "juego/reaccion/#" -v
```

Si el broker requiere TLS, use `mqtts://...` y añada el certificado raíz a la
configuración del cliente antes de usarlo fuera de una red de laboratorio.
