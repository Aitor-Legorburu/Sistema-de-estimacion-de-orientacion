# Sistema de adquisición y procesamiento de datos mediante sensores

## Descripción

Proyecto desarrollado para medir la inclinación de un objeto mediante sensores inerciales y procesar los datos obtenidos. Más especificamente, para observar el nivel de caida de una enmbarcación. Implementacion de una antena para observar los resultados a distancia.

## Funcionalidades

- Adquisición de datos de acelerómetro y giroscopio.
- Procesamiento en tiempo real.
- Filtrado para reducción de ruido mediante un filtro kalman.
- Visualización de resultados hasta 500m.

## Tecnologías

- Microcontrolador: Arduino
- Lenguaje: C++
- Sensor: MPU6050
- Transmisor/receptor: NRF24L01 PA+LNA
