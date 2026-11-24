# not_taskbar

> **Idioma:** [English](README.md) | Español

Sistema visual dinámico de **barras de tareas (TaskBar HUD)** para **SA-MP** y **open.mp** en la esquina inferior derecha, con título, descripción con altura dinámica, icono sprite, etiqueta de tiempo y barra de progreso con color automático (`progress2`).

## Características

- Múltiples taskbars simultáneas por jugador en slots independientes.
- Altura dinámica calculada automáticamente según las líneas de la descripción.
- Color automático de progreso (verde / amarillo / rojo) o color personalizado.
- Actualización en tiempo real de progreso, máximo, tiempo, descripción e icono.
- Limpieza automática en `OnPlayerConnect` y `OnPlayerDisconnect`.

## Requisitos

- `<a_samp>`
- `<progress2>` de Southclaws

## Instalación

```pawn
#include <a_samp>
#include <progress2>

#define MAX_TASKBAR 3 // Opcional (default: 3)
#include <not_taskbar>
```

## Uso rápido

```pawn
new taskid = ShowTaskBar(playerid, "~y~Entrega", "Lleva el paquete a LS", "05:00", 100, "LD_GRAV:timer");
UpdateTaskBarProgress(playerid, taskid, 50);
UpdateTaskBarTime(playerid, taskid, "02:30");
HideTaskBar(playerid, taskid);
```

## Resumen de API

`ShowTaskBar`, `HideTaskBar`, `HideAllTaskBars`, `UpdateTaskBarProgress`, `UpdateTaskBarMaxProgress`, `UpdateTaskBarTime`, `UpdateTaskBarDescription`, `UpdateTaskBarIcon`, `SetTaskBarColor`, `IsTaskBarActive`, `GetActiveTaskBarsCount`, `GetTaskBarProgress`.

## Créditos

- Escrito y refactorizado por **not**.
- Barras de progreso mediante **progress2** de **Southclaws**.
- Tabla de anchos de caracteres por **Nero_3D** (`td-string-width`).

## Licencia

Distribuido bajo la [Licencia MIT](LICENSE).
