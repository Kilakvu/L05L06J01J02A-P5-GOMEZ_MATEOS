# B1 - P2.2: medida de la señal de TIM7 con PulseView

## Objetivo

Verificar con un analizador lógico la señal cuadrada que el firmware de P2_1 genera en `PB0`, pin que controla el LED LD1 de la NUCLEO-F429ZI. Se comparan la frecuencia medida y la calculada a partir de TIM7.

P2_2 es una práctica de medida, no otro firmware: se compila y carga P2_1 para cada apartado y se captura la misma salida con el analizador LHT00SU1 y PulseView.

## Fundamento y valores esperados

TIM7 recibe 84 MHz. Con `TIM7_PRESCALER = 8399U`, el contador avanza a 10 kHz. Cada evento de actualización conmuta `PB0`; hacen falta dos interrupciones para completar un ciclo de la señal. Por tanto:

$$f_{CNT}=\frac{84\,000\,000}{8399+1}=10\,000\text{ Hz}$$

$$T_{nivel}=\frac{ARR+1}{f_{CNT}}, \qquad T_{senal}=2T_{nivel}, \qquad f_{senal}=\frac{1}{T_{senal}}$$

| Apartado | `TIM7_PERIOD` (ARR) | Nivel alto/bajo | Período completo | Frecuencia | Captura a 20 kHz |
|---|---:|---:|---:|---:|---:|
| A | `4999U` | 0.5 s | 1 s | 1 Hz | 5 s = 100 000 muestras |
| B | `14999U` | 1.5 s | 3 s | 1/3 Hz ≈ 0.333 Hz | 10 s = 200 000 muestras |

A 20 kHz, cada muestra representa 50 microsegundos. Se esperan 20 000 muestras entre flancos ascendentes consecutivos en A y 60 000 en B. Los flancos consecutivos de sentidos opuestos están separados por medio período: 0.5 s en A y 1.5 s en B.

## Montaje

1. Conecta la NUCLEO-F429ZI al ordenador por el ST-LINK y conecta el LHT00SU1 al mismo ordenador.
2. Conecta un canal digital del analizador a `PB0` (LD1) y conecta la masa del analizador a `GND` de la placa.
3. Comprueba que el analizador admite señales lógicas de 3.3 V. No conectes 5 V a `PB0`.
4. Abre PulseView, selecciona el dispositivo LHT00SU1 y habilita el canal conectado a `PB0`.

## Capturar el apartado A

1. En `B1/P2/P2_1/src/main.c`, configura `TIM7_PERIOD` como `4999U`.
2. Desde la raíz del repositorio, actualiza RTE, compila y carga el firmware:

	```powershell
	cbuild B1/P2/P2_1/vc/p2-1.csolution.yml --update-rte
	cbuild B1/P2/P2_1/vc/p2-1.csolution.yml --target all --active Target_1 --packs
	pyocd load --probe stlink: --cbuild-run B1/P2/P2_1/vc/out/p2-1+Target_1.cbuild-run.yml
	```

3. En PulseView, fija el muestreo en 20 kHz y la duración de captura en al menos 5 s (100 000 muestras).
4. Inicia la captura y deja que termine. Comprueba que aparecen varios ciclos estables.
5. Coloca los cursores en dos flancos ascendentes consecutivos (o dos descendentes). Anota el intervalo como el período completo; debe estar cerca de 1 s.
6. Guarda el proyecto/captura de PulseView y exporta una imagen legible para la entrega.

## Capturar el apartado B

1. Cambia `TIM7_PERIOD` a `14999U`, recompila y vuelve a cargar P2_1.
2. Configura PulseView a 20 kHz y captura al menos 10 s (200 000 muestras).
3. Mide entre dos flancos ascendentes consecutivos, no entre flancos opuestos. El intervalo debe estar cerca de 3 s.
4. Guarda la captura y exporta una imagen independiente de la del apartado A.

## Cálculos de la medida

Si los cursores indican un intervalo $T_{medido}$ entre flancos del mismo sentido:

$$f_{medida}=\frac{1}{T_{medido}}$$

La desviación relativa respecto a la frecuencia teórica se calcula como:

$$error_{relativo}=\frac{|f_{medida}-f_{teorica}|}{f_{teorica}}\times100\%$$

Usa segundos para el período al calcular la frecuencia en hertz. Si solo se mide entre un flanco ascendente y el siguiente descendente, se ha medido medio período: multiplícalo por dos antes de obtener la frecuencia. Una lectura cercana a 0.5 s en A o 1.5 s en B entre flancos opuestos es correcta; interpretarla como el período completo produciría un error de factor dos.

## Registro de resultados

Completar esta tabla después de medir físicamente. No sustituir datos pendientes por valores teóricos. Adjuntar las capturas exportadas junto a este README, por ejemplo en `capturas/`.

| Apartado | Flancos usados | Período medido | Frecuencia medida | Error relativo | Captura |
|---|---|---:|---:|---:|---|
| A | Ascendentes consecutivos | Pendiente | Pendiente | Pendiente | Pendiente |
| B | Ascendentes consecutivos | Pendiente | Pendiente | Pendiente | Pendiente |

## Criterios de comprobación

- La captura está muestreada a 20 kHz y tiene la duración mínima indicada.
- La señal permanece en niveles lógicos seguros de 0 a 3.3 V y comparte GND con la placa.
- Se mide entre flancos del mismo sentido para obtener el período completo.
- La frecuencia medida es compatible con 1 Hz en A y aproximadamente 0.333 Hz en B.
- Se guardan los archivos y capturas reales de ambos apartados.

Cuando estén registradas las mediciones y las capturas, crear el commit de P2_2 y el tag `B1_P2_2_Ej1` sobre ese commit.
