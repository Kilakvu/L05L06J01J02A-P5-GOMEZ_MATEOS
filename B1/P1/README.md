# Bloque 1 - Práctica 1: Uso de Interrupciones Externas y GPIOs

**Asignatura:** Sistemas Basados en Microprocesador (SBM)  
**Institución:** Departamento de Ingeniería Telemática y Electrónica (DTE) - UPM  
**Plataforma de Hardware:** NUCLEO STM32F429ZI + Tarjeta mbed Application Board

---

## 1. Objetivos

1. Aprender a configurar los recursos básicos del árbol de reloj (**RCC**) en STM32.
2. Dominar el uso y configuración de puertos **GPIO** tanto de entrada como de salida digital.
3. Implementar y sincronizar **interrupciones externas (EXTI / NVIC)** generadas por pulsadores mecánicos.
4. Interactuar con periféricos externos mediante cableado y asignación de pines.

---

## 2. Estructura del Proyecto y Requisitos

La práctica sigue la organización estándar del repositorio:

```text
B1/
└── P1/
    ├── README.md
    ├── src/          <- código fuente común (main.c, main.h, stm32f4xx_it.c, ...)
    ├── keil/         <- proyecto Keil µVision
    └── vc/           <- proyecto VS Code / CMSIS-Toolbox
```

- El código fuente de la práctica debe residir en `src/`.
- La carpeta `vc/` contiene únicamente la configuración del proyecto para compilar con CMSIS-Toolbox y VS Code.
- Los entregables deben marcarse mediante los siguientes tags de Git:

| Etapa | Tag |
|---|---|
| Ejercicio 1 completado | `B1_P1_Ej1` |
| Ejercicio 2 completado | `B1_P1_Ej2` |
| Ejercicio 3 completado | `B1_P1_Ej3` |

---

## 3. Guía de Desarrollo de los Ejercicios

### Ejercicio 1: Configuración del Árbol de Reloj (RCC)
- **Objetivo:** Crear un proyecto en Keil $\mu$Vision para la placa NUCLEO-F429ZI configurando el reloj de los temporizadores de APB1 a **$48\text{ MHz}$**.
- **Preescaladores fijos:**
  - AHB Prescaler = `1`
  - APB1 Prescaler = `4`
  - APB2 Prescaler = `2`
- **Cálculo de Parámetros:**
  - Reloj de entrada HSE = $8\text{ MHz}$.
  - Frecuencia de los timers en APB1: Dado que el preescalador de APB1 es $4 \ne 1$, los timers reciben el doble del reloj de periféricos de APB1 ($f_{\text{TIM\_APB1}} = 2 \cdot PCLK1$).
  - Por lo tanto: $PCLK1 = \frac{48\text{ MHz}}{2} = 24\text{ MHz}$.
  - Dado que $\text{APB1 Prescaler} = 4$, la frecuencia del bus del sistema debe ser:
    $$HCLK = SYSCLK = 24\text{ MHz} \cdot 4 = 96\text{ MHz}$$
  - Parámetros del PLL ($M, N, P$) para obtener $SYSCLK = 96\text{ MHz}$ a partir de $HSE = 8\text{ MHz}$:
    $$f_{\text{VCO\_IN}} = \frac{8\text{ MHz}}{M} \quad (\text{recomendado } 1\text{ a } 2\text{ MHz, fijamos } M = 8 \Rightarrow f_{\text{VCO\_IN}} = 1\text{ MHz})$$
    $$f_{\text{VCO\_OUT}} = 1\text{ MHz} \cdot N$$
    $$SYSCLK = \frac{f_{\text{VCO\_OUT}}}{P} \Rightarrow \frac{N}{P} = 96$$
    Eligiendo el divisor estándar $P = 2$, se obtiene $N = 192$ ($192\text{ MHz} \le f_{\text{VCO\_OUT}} \le 432\text{ MHz}$, dentro del rango permitido).

| Elemento | Valor |
|---|---|
| **M** | 8 |
| **N** | 192 |
| **P** | 2 |

- **Verificación:** Colocar un punto de ruptura (*breakpoint*) tras `SystemCoreClockUpdate()` y comprobar el valor en la ventana *Watches*. Capturar la pantalla e incluirla en el proyecto.

| Variable | Valor teórico | Valor leído |
|---|---|---|
| `SystemCoreClock` | 96000000 (96 MHz) | *(Completar con lectura de depuración)* |

---

### Ejercicio 2: GPIOs y EXTI con LEDs de Usuario
- **Objetivo:** Controlar las frecuencias de parpadeo de los LEDs de la placa Nucleo con el pulsador azul B1 (`PC13`).
- **Recursos empleados:**
  - Pulsador B1: Pin `PC13` en modo `GPIO_MODE_IT_RISING` (línea `EXTI15_10_IRQn`).
  - LEDs de usuario: `LD1` (Verde, `PB0`), `LD2` (Azul, `PB7`), `LD3` (Rojo, `PB14`).
  - Base de tiempo: `HAL_Delay()`.
- **Comportamiento:**
  - Pulsador B1 conmuta la frecuencia de `LD1` secuencialmente: **1 Hz $\rightarrow$ 2 Hz $\rightarrow$ 4 Hz $\rightarrow$ 1 Hz ...**
  - Frecuencia de `LD2`: la mitad de `LD1` ($0.5\text{ Hz}, 1\text{ Hz}, 2\text{ Hz}$).
  - Frecuencia de `LD3`: la cuarta parte de `LD1` ($0.25\text{ Hz}, 0.5\text{ Hz}, 1\text{ Hz}$).
  - Frecuencia inicial de `LD1` al arrancar: **1 Hz**.

---

### Ejercicio 3: Integración con Tarjeta mbed Application Board
- **Objetivo:** Trasladar el comportamiento del Ejercicio 2 al LED RGB presente en la tarjeta de expansión mbed App Board.
- **Mapeo de conexiones:**

| Señal | Pin mbed App Board | STM32F429ZI Pin | Conector NUCLEO STM32F429ZI |
|---|---|---|---|
| **VCC 3.3V** | DIP40 | +3.3V | CN8.7 / +3.3V |
| **GND** | DIP1 | GND | CN8.11 / GND |
| **R (Rojo)** | DIP23 | PD13 | CN10.19 / D28 |
| **G (Verde)** | DIP24 | PD12 | CN10.21 / D29 |
| **B (Azul)** | DIP25 | PD11 | CN10.23 / D30 |

- **Asignación lógica de LEDs:**
  - `LD1` (frecuencia base) $\rightarrow$ **RGB ROJO** (`PD13`)
  - `LD2` ($f/2$) $\rightarrow$ **RGB VERDE** (`PD12`)
  - `LD3` ($f/4$) $\rightarrow$ **RGB AZUL** (`PD11`)

## Compilar y cargar desde PowerShell

La práctica sigue la organización estándar del repositorio:
- El código fuente está en `B1/P1/src`
- La configuración del proyecto CMSIS está en `B1/P1/vc`

Conecta la NUCLEO-F429ZI mediante el puerto USB de **ST-LINK** y abre PowerShell en `B1/P1/vc`.

```powershell
cd B1/P1/vc

# Preparar o actualizar los archivos RTE
cbuild b1-p1.csolution.yml --update-rte

# Compilar el target Target_1
cbuild b1-p1.csolution.yml --target all --active Target_1 --packs

# Comprobar que pyOCD detecta el ST-LINK
pyocd list

# Cargar el programa en la placa
pyocd load --probe stlink: --cbuild-run .\out\b1-p1+Target_1.cbuild-run.yml
```

Si `pyocd` no se reconoce como comando, ejecuta su ruta instalada por la extensión:

```powershell
$pyocd = "$env:USERPROFILE\.vscode\extensions\arm.vscode-cmsis-debugger-1.8.0-win32-x64\tools\pyocd\pyocd.exe"
& $pyocd list
& $pyocd load --probe stlink: --cbuild-run .\out\b1-p1+Target_1.cbuild-run.yml
```

Para depurar desde VS Code, sigue estos pasos:
1. Abre la solución `B1/P1/vc/b1-p1.csolution.yml`
2. Selecciona `Target_1`
3. Pulsa **Update launch.json and tasks.json**
4. Inicia **Run and Debug**

La lógica del programa sigue estando en `B1/P1/src`, y la carpeta `vc` sirve únicamente para compilar y depurar en VS Code.