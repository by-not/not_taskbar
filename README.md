# not_taskbar

> **Language:** English | [Español](README.es.md)

Dynamic visual **TaskBar HUD** system for **SA-MP** and **open.mp**, displayed in the bottom-right corner with title, dynamic-height description, sprite icon, time label, and auto-colored progress bar (`progress2`).

## Features

- Multiple simultaneous taskbars per player with independent slots.
- Dynamic box height calculated from wrapped description lines.
- Automatic progress bar color thresholds (green / yellow / red) or custom colors.
- Real-time updates for progress, max progress, time, description, and icon.
- Automatic cleanup on `OnPlayerConnect` and `OnPlayerDisconnect`.

## Requirements

- `<a_samp>`
- `<progress2>` by Southclaws

## Installation

```pawn
#include <a_samp>
#include <progress2>

#define MAX_TASKBAR 3 // Optional (default: 3)
#include <not_taskbar>
```

## Quick Start

```pawn
new taskid = ShowTaskBar(playerid, "~y~Delivery", "Deliver the package to LS", "05:00", 100, "LD_GRAV:timer");
UpdateTaskBarProgress(playerid, taskid, 50);
UpdateTaskBarTime(playerid, taskid, "02:30");
HideTaskBar(playerid, taskid);
```

## API Summary

`ShowTaskBar`, `HideTaskBar`, `HideAllTaskBars`, `UpdateTaskBarProgress`, `UpdateTaskBarMaxProgress`, `UpdateTaskBarTime`, `UpdateTaskBarDescription`, `UpdateTaskBarIcon`, `SetTaskBarColor`, `IsTaskBarActive`, `GetActiveTaskBarsCount`, `GetTaskBarProgress`.

## Credits

- Written and refactored by **not**.
- Progress bar rendering via **progress2** by **Southclaws**.
- Character width table by **Nero_3D** (`td-string-width`).

## License

Released under the [MIT License](LICENSE).
