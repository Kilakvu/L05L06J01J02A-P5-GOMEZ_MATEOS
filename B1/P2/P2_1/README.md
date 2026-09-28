# B1 - P2.1: interrupciones periódicas con TIM7

## Objetivo

Generar una interrupción periódica con el temporizador básico TIM7 del STM32F429ZI y conmutar el LED LD1 de la NUCLEO-F429ZI. El ejercicio permite separar tres frecuencias que suelen confundirse: el reloj del temporizador, la frecuencia de interrupción y la frecuencia completa del parpadeo.

El proyecto se desarrolla y compila en **VS Code con CMSIS-Toolbox**. El código está en `src/` y la solución para CMSIS-Toolbox en `vc/`; no es necesario abrir ni modificar un proyecto de Keil.

## Configuración del reloj

En `src/main.c`, `SystemClock_Config()` toma el HSE de 8 MHz y configura el PLL con `M=8`, `N=336`, `P=2` y `Q=7`:

1. La entrada del VCO queda en $8/8=1\text{ MHz}$.
2. La salida del VCO es $1\times336=336\text{ MHz}$.
3. SYSCLK resulta $336/2=168\text{ MHz}$; AHB no divide ese reloj.
4. APB1 divide HCLK por 4, así que PCLK1 es $168/4=42\text{ MHz}$.
5. Como el prescaler APB1 no es 1, el reloj de los timers APB1 se duplica: TIM7 recibe $2\times42=84\text{ MHz}$.

Por tanto, `SystemCoreClock` debe valer `168000000`, pero no es el reloj que se debe usar directamente para calcular TIM7. TIM7 recibe 84 MHz.

## Configuración de TIM7

El fuente define `TIM7_PRESCALER = 8399U`. Los registros PSC y ARR cuentan desde cero, por lo que el divisor efectivo es el valor programado más uno:

$$f_{CNT}=\frac{84\,000\,000}{8399+1}=10\,000\text{ Hz}$$

El contador avanza una cuenta cada 100 microsegundos. Con `ARR = N`, TIM7 genera el evento de actualización cada `N + 1` cuentas:

$$T_{IRQ}=\frac{ARR+1}{f_{CNT}}$$

La interrupción conmuta PB0 una vez. Se necesitan dos conmutaciones para volver al mismo nivel lógico y completar un período de onda; por eso el período del LED es el doble del intervalo entre interrupciones.

| Apartado | `TIM7_PERIOD` / ARR | Tiempo entre IRQ | IRQ/s | Período del LED | Frecuencia del LED |
|---|---:|---:|---:|---:|---:|
| A | `4999U` | 0.5 s | 2 | 1 s | 1 Hz |
| B | `14999U` | 1.5 s | 2/3 | 3 s | 1/3 Hz ≈ 0.333 Hz |

El valor actual en `src/main.c` es `14999U`, es decir, el apartado B: LD1 permanece 1.5 s encendido y 1.5 s apagado. Para reproducir A, cambiarlo a `4999U`, compilar y cargar de nuevo.

## Recorrido del programa

1. `HAL_Init()` inicializa HAL y la base de tiempo usada por sus servicios.
2. `SystemClock_Config()` configura PLL, SYSCLK y los divisores AHB/APB.
3. `SystemCoreClockUpdate()` actualiza la variable global que representa el reloj del núcleo.
4. `GPIO_Init()` habilita GPIOB, configura PB0 como salida push-pull sin resistencias internas y lo deja inicialmente a nivel bajo. En esta placa PB0 controla LD1.
5. `TIM7_Init()` habilita el reloj de TIM7, configura el prescaler, el período y el conteo ascendente, y prepara la interrupción en NVIC.
6. `HAL_TIM_Base_Start_IT()` inicia el contador y habilita sus interrupciones de actualización.
7. `TIM7_IRQHandler()` deriva la IRQ al manejador de HAL. `HAL_TIM_PeriodElapsedCallback()` identifica TIM7, incrementa `tim7_interrupt_count` y conmuta PB0.
8. El bucle `while (1)` queda vacío porque el temporizador y la interrupción realizan el trabajo periódico sin retardos bloqueantes ni sondeo continuo.

`tim7_interrupt_count` es `volatile` porque se modifica en una interrupción y puede observarse desde el depurador. En A aumenta aproximadamente 2 veces por segundo; en B, 2 veces cada 3 segundos. Ese contador mide IRQ, no ciclos completos del LED.

## Compilar y cargar desde VS Code

Requisitos: extensión Arm CMSIS Solution, CMSIS-Toolbox, CMake, Arm Compiler 6 configurado y pyOCD. Abre la carpeta raíz del repositorio en VS Code y selecciona `Target_1` en la solución `vc/p2-1.csolution.yml`.

Desde PowerShell, en la raíz del repositorio:

```powershell
cbuild B1/P2/P2_1/vc/p2-1.csolution.yml --update-rte
cbuild B1/P2/P2_1/vc/p2-1.csolution.yml --target all --active Target_1 --packs
pyocd load --probe stlink: --cbuild-run B1/P2/P2_1/vc/out/p2-1+Target_1.cbuild-run.yml
```

La imagen compilada es `vc/out/p2-1/Target_1/p2-1.axf`. El aviso `The no-dev option is deprecated` no impide compilar; el resultado de `cbuild` debe terminar con `1 succeeded, 0 failed`. Si el compilador informa que no puede obtener una licencia, hay que conectarse a la red/VPN que da acceso al servidor de licencias antes de repetir el build.

## Comprobación en la placa

La carga se realizó con el ST-LINK integrado y LD1 se observó parpadeando. Para verificar la temporización con el analizador lógico, conectar su entrada a PB0 y GND a GND de la placa; no aplicar más de 3.3 V. Muestrear a 20 kHz y medir entre flancos ascendentes consecutivos (un período completo):

- Apartado A: alrededor de 1 s, es decir, 1 Hz.
- Apartado B: alrededor de 3 s, es decir, 0.333 Hz.

También se puede detener el depurador después de `SystemCoreClockUpdate()` y revisar `SystemCoreClock`; para observar las IRQ, añadir `tim7_interrupt_count` a **Watch**. Las mediciones con analizador deben anotarse en `../P2_2/README.md` junto con la captura real.

## Entrega Git

El hito A se identifica con `B1_P2_1_Ej1` y el hito B con `B1_P2_1_Ej2`. Cada tag se crea después de validar el apartado correspondiente y sobre el commit que contiene exactamente ese estado. Para registrar esta solución, usar un mensaje breve como:

```text
B1-P2_1: implementa TIM7
```