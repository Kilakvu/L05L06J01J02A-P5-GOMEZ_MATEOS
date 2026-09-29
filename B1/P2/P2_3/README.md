# B1 - P2.3: generación de señal con TIM2

## Objetivos

Generar en `PB11` una onda cuadrada de 1 kHz mediante la salida hardware `TIM2_CH4`, sin una interrupción por flanco. Después, alternar entre 1 kHz y 2.5 kHz con el pulsador azul de la placa. El control opcional por joystick requiere un joystick analógico externo.

La lógica se encuentra en `src/main.c`; la solución de compilación es `vc/p2-3.csolution.yml`. El proyecto se compila desde VS Code con CMSIS-Toolbox y Arm Compiler 6.

## Reloj del contador y cálculo

La configuración RCC es la misma que en P2_1: SYSCLK = 168 MHz, PCLK1 = 42 MHz y reloj de TIM2 = 84 MHz, porque el prescaler APB1 es distinto de 1. El prescaler de TIM2 vale 83, por lo que el contador avanza a:

$$f_{CNT}=\frac{84\,000\,000}{83+1}=1\,000\,000\text{ Hz}$$

En Output Compare Toggle, cada coincidencia entre CNT y CCR4 invierte el nivel de salida. Hace falta una coincidencia para cada semiperíodo; por ello:

$$f_{salida}=\frac{f_{CNT}}{2(ARR+1)}$$

El canal se configura con `CCR4 = ARR`, de modo que hay una coincidencia por ciclo del contador. Los valores resultan:

| Frecuencia solicitada | Cuentas por semiperíodo | ARR | CCR4 |
|---:|---:|---:|---:|
| 1 000 Hz | $1\,000\,000/(2\times1\,000)=500$ | 499 | 499 |
| 2 500 Hz | $1\,000\,000/(2\times2\,500)=200$ | 199 | 199 |

Los registros cuentan desde cero: un `ARR` de 499 representa 500 cuentas, no 499. La señal tiene ciclo útil nominal del 50 % porque cada semiperíodo tiene la misma duración.

## Pines y periféricos

- `PB11`: `TIM2_CH4`, función alternativa AF1, salida de la onda. Conectar aquí el analizador lógico.
- `PC13`: pulsador azul de usuario, configurado como entrada EXTI de flanco ascendente con pull-down.
- `PA1` (ADC1_IN1): entrada analógica usada solo si se habilita el control opcional con joystick externo.
- `GND`: masa común entre NUCLEO, analizador y, si se usa, joystick.

TIM2 opera de forma autónoma en el canal 4. No se habilita la interrupción de TIM2: la CPU no genera cada flanco. EXTI de PC13 se usa únicamente para cambiar los parámetros del timer; su callback aplica un antirrebote de 200 ms.

## Recorrido del firmware

1. HAL y RCC se inicializan para que TIM2 reciba 84 MHz.
2. `GPIO_Init()` asigna PB11 a AF1 y prepara PC13 para EXTI.
3. `TIM2_Init()` configura el contador a 1 MHz y el canal 4 en `TIM_OCMODE_TOGGLE` con los valores iniciales para 1 kHz.
4. `HAL_TIM_OC_Start()` habilita la salida del canal. El timer conmuta PB11 por hardware.
5. La IRQ `EXTI15_10_IRQHandler()` delega el flanco del pulsador a HAL. `HAL_GPIO_EXTI_Callback()` alterna la frecuencia solicitada.
6. `Set_Output_Frequency()` detiene brevemente el canal, calcula y carga ARR/CCR, reinicia CNT y vuelve a iniciar la salida. `current_frequency_hz` permite observar el modo actual en el depurador.

Al arrancar, la frecuencia es 1 kHz. Cada pulsación aceptada alterna a 2.5 kHz y la siguiente vuelve a 1 kHz.

## Apartados y comprobación

### Apartado 1: salida fija de 1 kHz

Deja el valor inicial `INITIAL_FREQUENCY_HZ` en `1000U` y no pulses el botón durante la captura. Conecta el analizador a `PB11` y GND, selecciona el canal digital y una frecuencia de muestreo de al menos 100 kHz. Una captura de 10 ms contiene aproximadamente 10 períodos de 1 kHz. Mide entre flancos ascendentes consecutivos: el período esperado es 1 ms.

### Apartado 2: alternancia con pulsador

La placa arranca a 1 kHz. Inicia una captura que incluya varios segundos, pulsa el botón azul una vez y observa el cambio a 2.5 kHz; vuelve a pulsarlo para regresar a 1 kHz. Períodos esperados:

| Frecuencia | Período entre flancos ascendentes |
|---:|---:|
| 1 000 Hz | 1 ms |
| 2 500 Hz | 0.4 ms |

Guarda una captura con cursores o medidas que muestre ambos valores. No uses la duración del nivel alto como período completo; con Toggle, cada nivel dura la mitad del período.

### Apartado 3 opcional: joystick

La NUCLEO-F429ZI no incorpora joystick. Para probar este modo se necesita uno analógico externo compatible con 3.3 V: conectar alimentación a 3.3 V, GND común y eje Y a `PA1`. Habilitarlo cambiando `ENABLE_JOYSTICK` a `1` y recompilar.

- ADC menor que 1000: gesto UP, duplica la frecuencia.
- ADC mayor que 3000: gesto DOWN, divide la frecuencia entre dos.
- Al volver al intervalo central (1500 a 2600), se rearma la lectura del siguiente gesto.

La frecuencia está limitada a 1–16 000 Hz. Las capturas deben mostrar los valores resultantes y documentar el gesto aplicado.

## Compilar y cargar en VS Code

Desde la raíz del repositorio, con CMSIS-Toolbox, Arm Compiler 6, CMake y pyOCD disponibles:

```powershell
cbuild B1/P2/P2_3/vc/p2-3.csolution.yml --update-rte
cbuild B1/P2/P2_3/vc/p2-3.csolution.yml --target all --active Target_1 --packs
pyocd load --probe stlink: --cbuild-run B1/P2/P2_3/vc/out/p2-3+Target_1.cbuild-run.yml
```

En VS Code también se puede abrir `vc/p2-3.csolution.yml`, seleccionar `Target_1`, ejecutar **Update RTE** y después **Build**. El binario esperado es `vc/out/p2-3/Target_1/p2-3.axf`.

## Estado de validación

La implementación y la configuración CMSIS están preparadas. En la última compilación, CMake llegó a invocar `armclang`, pero el compilador no obtuvo la licencia (`Flex error -15`, servidor `8224@LICENCIAS.SEC.UPM.ES` inaccesible); por ello aún no hay una compilación confirmada ni se puede verificar el binario de este estado. El aviso deprecado `no-dev` no es la causa del fallo. Reintentar el build cuando haya conexión a la red/VPN del servidor de licencias.

La validación eléctrica también queda pendiente: medir PB11 con el analizador y comprobar PC13 con la placa. Registrar las lecturas reales y adjuntar las capturas aquí o en una carpeta `capturas/`; no sustituirlas por los valores teóricos.

## Git

Crear un commit por apartado después de compilar y probar el comportamiento correspondiente. Tags previstos por el enunciado:

| Hito | Tag |
|---|---|
| Apartado 1: salida fija de 1 kHz | `B1_P2_3_Ej1` |
| Apartado 2: cambio entre 1 kHz y 2.5 kHz | `B1_P2_3_Ej2` |
| Apartado 3 opcional: joystick | `B1_P2_3_Ej3` |

El README raíz ignora archivos Markdown, así que añadirlo explícitamente con `git add -f B1/P2/P2_3/README.md`. No crear los tags hasta verificar el apartado en la placa.