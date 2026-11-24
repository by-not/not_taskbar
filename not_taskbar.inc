/*
	=====================================================
	NOT_TASKBAR.INC - Dynamic TaskBar HUD System
	=====================================================
	Author: not
	Version: 1.1
	Date: 2026
	License: MIT
	Credits / Dependencies: Southclaws (progress2), Nero_3D (td-string-width)

	DESCRIPTION:
	Visual taskbar system for SA-MP and open.mp displayed in the bottom-right
	corner of the screen. Each taskbar displays a title, dynamic-height
	description, sprite icon, time label, and an auto-colored progress bar
	powered by progress2.

	MAIN FEATURES:
	- Multiple simultaneous taskbars per player (independent slots)
	- Dynamic box height based on wrapped description line count
	- Automatic progress bar color thresholds (green / yellow / red) or custom color
	- Real-time updates for progress, max progress, time, description, and icon
	- Automatic cleanup on OnPlayerConnect and OnPlayerDisconnect
	- Standard ALS hooks compatible with YSI (y_hooks) and other includes

	BASIC CONFIGURATION:
	```pawn
	#include <a_samp>
	#include <progress2>           // Required

	#define MAX_TASKBAR 3          // Max simultaneous taskbars per player (default: 3)
	// #define NOT_TASKBAR_DEBUG   // Optional: debug slot labels
	#include <not_taskbar>
	```

	USAGE EXAMPLE:
	```pawn
	new taskid = ShowTaskBar(playerid, "~y~Delivery", "Deliver the package to LS", "05:00", 100, "LD_GRAV:timer");
	UpdateTaskBarProgress(playerid, taskid, 50);
	UpdateTaskBarTime(playerid, taskid, "02:30");
	HideTaskBar(playerid, taskid);
	```

	PUBLIC API:
	- ShowTaskBar, HideTaskBar, HideAllTaskBars
	- UpdateTaskBarProgress, UpdateTaskBarMaxProgress, UpdateTaskBarTime
	- UpdateTaskBarDescription, UpdateTaskBarIcon, SetTaskBarColor
	- IsTaskBarActive, GetActiveTaskBarsCount, GetTaskBarProgress

	COMPATIBILITY:
	- SA-MP 0.3.7 and open.mp
	- Requires <a_samp> and <progress2>
	- Compatible with not_notify.inc and not_menu.inc (shared char-width guard)

	=====================================================
*/

#if defined _not_taskbar_included
	#endinput
#endif
#define _not_taskbar_included

#include <a_samp>

#if defined NOT_TASKBAR_DEBUG
	#tryinclude <textdraw-streamer>
#endif

#include <progress2>

#if !defined MAX_LENGTH_MESSAGE
	#define MAX_LENGTH_MESSAGE (512)
#endif

#if !defined MAX_LENGTH_TITLE
	#define MAX_LENGTH_TITLE (44)
#endif

#if !defined MAX_LENGTH_ICON
	#define MAX_LENGTH_ICON (32)
#endif

#if !defined MAX_TASKBAR
	#define MAX_TASKBAR (3)
#endif

#define MAX_TEXTDRAW_TASKBAR (6)

#if !defined TASKBAR_COLOR_FULL
	#define TASKBAR_COLOR_FULL (0x28B90AFF)
#endif
#if !defined TASKBAR_COLOR_MEDIUM
	#define TASKBAR_COLOR_MEDIUM (0xFFBE0AFF)
#endif
#if !defined TASKBAR_COLOR_LOW
	#define TASKBAR_COLOR_LOW (0xB9280AFF)
#endif

static enum E_TASKBAR_INFO
{
	taskTitle[MAX_LENGTH_TITLE],
	taskDesc[MAX_LENGTH_MESSAGE],
	taskTime[32],
	taskIcon[MAX_LENGTH_ICON],
	taskProgress,
	taskMaxProgress,
	taskOrder,
	taskBarColor,
	bool:taskAutoColor,
	bool:taskActive
};

static g_TaskbarInfo[MAX_PLAYERS][MAX_TASKBAR][E_TASKBAR_INFO];
static PlayerText:g_TaskbarTD[MAX_PLAYERS][(MAX_TEXTDRAW_TASKBAR * MAX_TASKBAR)];
static PlayerBar:g_TaskbarBar[MAX_PLAYERS][MAX_TASKBAR];
static g_TaskbarOrderCounter[MAX_PLAYERS];
static g_TaskbarCount[MAX_PLAYERS];

#define INVALID_TASKBAR_ID			(-1)

#define TaskInfo::%0[%1][%2]	g_TaskbarInfo[%1][%2][%0]
#define TaskTD::[%1][%2]		g_TaskbarTD[%1][%2]
#define TaskBar::[%1][%2]		g_TaskbarBar[%1][%2]

#if !defined _NOT_TD_CHAR_WIDTH_DEFINED
#define _NOT_TD_CHAR_WIDTH_DEFINED
static stock const TDCharacterDefaultWidth[4] = {27, 20, 27, 20};
static const TDCharacterWidth[4][176] = {
	{
		 0,  12,  12,  12,  12,  12,  12,  12,
		12,  12,  12,  12,  12,  12,  12,  12,
		12,  12,  12,  12,  12,  12,  12,  12,
		12,  12,  12,  12,  12,  12,  12,  12,
		12,  13,  13,  28,  28,  28,  28,   8,
		17,  17,  30,  28,  28,  12,   9,  21,
		28,  14,  28,  28,  28,  28,  28,  28,
		28,  28,  13,  13,  30,  30,  30,  30,
		10,  25,  23,  21,  24,  22,  20,  24,
		24,  17,  20,  22,  20,  30,  27,  27,
		26,  26,  24,  23,  24,  31,  23,  31,
		24,  23,  21,  28,  33,  33,  14,  28,
		10,  11,  12,   9,  11,  10,  10,  12,
		12,   7,   7,  13,   5,  18,  12,  10,
		12,  11,  10,  12,   8,  13,  13,  18,
		17,  13,  12,  30,  30,  37,  35,  37,
		25,  25,  25,  25,  33,  21,  24,  24,
		24,  24,  17,  17,  17,  17,  27,  27,
		27,  27,  31,  31,  31,  31,  11,  11,
		11,  11,  11,  20,   9,  10,  10,  10,
		10,   7,   7,   7,   7,  10,  10,  10,
		10,  13,  13,  13,  13,  27,  12,  30
	}, {
		 0,  15,  15,  15,  15,  15,  15,  15,
		15,  15,  15,  15,  15,  15,  15,  15,
		15,  15,  15,  15,  15,  15,  15,  15,
		15,  15,  15,  15,  15,  15,  15,  15,
		15,   9,  17,  27,  20,  34,  23,  12,
		12,  12,  21,  20,  12,  14,  12,  15,
		23,  15,  21,  21,  21,  21,  21,  21,
		20,  21,  12,  12,  24,  24,  24,  19,
		10,  22,  19,  19,  22,  16,  19,  24,
		22,  11,  16,  21,  15,  28,  24,  27,
		20,  25,  19,  19,  18,  23,  23,  31,
		23,  19,  21,  21,  13,  35,  11,  21,
		10,  19,  20,  14,  20,  19,  13,  20,
		19,   9,   9,  19,   9,  29,  19,  21,
		19,  19,  15,  15,  14,  18,  19,  27,
		20,  20,  17,  21,  17,  20,  15,  15,
		22,  22,  22,  22,  29,  19,  16,  16,
		16,  16,  11,  11,  11,  11,  27,  27,
		27,  27,  23,  23,  23,  23,  20,  19,
		19,  19,  19,  30,  14,  19,  19,  19,
		19,   9,   9,   9,   9,  21,  21,  21,
		21,  18,  18,  18,  18,  24,  19,  19
	}, {
		 0,  15,  23,  15,  21,  21,  21,  21,
		21,  21,  20,  21,  12,  12,  24,  24,
		24,  19,  10,  22,  19,  19,  22,  16,
		19,  24,  22,  11,  16,  21,  15,  28,
		12,  13,  13,  28,  37,  28,  30,   8,
		17,  17,  30,  28,  28,  12,   9,  21,
		27,  16,  27,  27,  27,  27,  27,  27,
		27,  27,  18,  13,  30,  30,  30,  30,
		10,  29,  26,  25,  28,  26,  25,  27,
		28,  12,  24,  25,  24,  30,  27,  29,
		26,  26,  25,  26,  25,  26,  28,  32,
		27,  26,  26,  28,  33,  33,  10,  28,
		10,  29,  26,  25,  28,  26,  25,  27,
		28,  12,  24,  25,  24,  30,  27,  29,
		26,  26,  25,  26,  25,  26,  28,  32,
		27,  26,  26,  30,  30,  37,  35,  37,
		29,  29,  29,  29,  33,  25,  26,  26,
		26,  26,  14,  14,  14,  14,  29,  29,
		29,  29,  26,  26,  26,  26,  21,  29,
		29,  29,  29,  33,  25,  26,  26,  26,
		26,  14,  14,  14,  14,  29,  29,  29,
		29,  26,  26,  26,  26,  25,  25,  30
	}, {
		 0,   9,   9,  18,  18,  18,  18,  18,
		18,  18,  18,  19,  19,  19,   0,   9,
		 9,   9,   9,  18,  18,  18,  18,  18,
		18,  18,  18,  19,  19,  19,   0,   9,
		15,  10,  17,  27,  20,  34,  23,  10,
		15,  15,  21,  20,  12,  14,   9,  15,
		20,  18,  19,  19,  21,  19,  19,  19,
		19,  19,  16,  12,  24,  24,  24,  21,
		10,  19,  19,  19,  20,  19,  16,  19,
		19,   9,  19,  20,  14,  29,  19,  19,
		19,  19,  19,  19,  21,  19,  20,  32,
		21,  19,  19,  21,  13,  35,  10,  21,
		10,  19,  19,  19,  20,  19,  16,  19,
		19,   9,  19,  20,  14,  29,  19,  19,
		19,  19,  19,  19,  21,  19,  20,  32,
		21,  19,  19,  21,  17,  20,  15,  15,
		19,  19,  19,  19,  29,  19,  19,  19,
		19,  19,   9,   9,   9,   9,  19,  19,
		19,  19,  19,  19,  19,  19,  19,  19,
		19,  19,  19,  29,  19,  19,  19,  19,
		19,   9,   9,   9,   9,  19,  19,  19,
		19,  19,  19,  19,  19,  21,  21,  19
	}
};
#endif // _NOT_TD_CHAR_WIDTH_DEFINED

//      _    _  ____   ____  _  __ _____
//     | |  | |/ __ \ / __ \| |/ // ____|
//     | |__| | |  | | |  | | ' /| (___
//     |  __  | |  | | |  | |  <  \___ \
//     | |  | | |__| | |__| | . \ ____) |
//     |_|  |_|\____/ \____/|_|\_\_____/
//
//

// Convierte caracteres latinos extendidos al equivalente de codificacion
// interna usada por los textdraws de SA-MP (GTA charset).
// Parametros:
//   text[] - Texto de entrada con caracteres latinos (UTF-8 / ISO-8859)
// Retorna: string con los caracteres convertidos listo para usar en textdraws
static stock TextDrawFixedLatinoText(const text[])
{
	new string[1024];
	new len = strlen(text);
	if(len >= sizeof(string)) len = sizeof(string) - 1;

	for(new i = 0; i < len; i++)
	{
		switch(text[i] & 0xFF)
		{
			case 0xE0: string[i] = 151;
			case 0xE1: string[i] = 152;
			case 0xE2: string[i] = 153;
			case 0xE4: string[i] = 154;
			case 0xC0: string[i] = 128;
			case 0xC1: string[i] = 129;
			case 0xC2: string[i] = 130;
			case 0xC4: string[i] = 131;
			case 0xE8: string[i] = 157;
			case 0xE9: string[i] = 158;
			case 0xEA: string[i] = 159;
			case 0xEB: string[i] = 160;
			case 0xC8: string[i] = 134;
			case 0xC9: string[i] = 135;
			case 0xCA: string[i] = 136;
			case 0xCB: string[i] = 137;
			case 0xEC: string[i] = 161;
			case 0xED: string[i] = 162;
			case 0xEE: string[i] = 163;
			case 0xEF: string[i] = 164;
			case 0xCC: string[i] = 138;
			case 0xCD: string[i] = 139;
			case 0xCE: string[i] = 140;
			case 0xCF: string[i] = 141;
			case 0xF2: string[i] = 165;
			case 0xF3: string[i] = 166;
			case 0xF4: string[i] = 167;
			case 0xF6: string[i] = 168;
			case 0xD2: string[i] = 142;
			case 0xD3: string[i] = 143;
			case 0xD4: string[i] = 144;
			case 0xD6: string[i] = 145;
			case 0xF9: string[i] = 169;
			case 0xFA: string[i] = 170;
			case 0xFB: string[i] = 171;
			case 0xFC: string[i] = 172;
			case 0xD9: string[i] = 146;
			case 0xDA: string[i] = 147;
			case 0xDB: string[i] = 148;
			case 0xDC: string[i] = 149;
			case 0xF1: string[i] = 174;
			case 0xD1: string[i] = 173;
			case 0xBF: string[i] = 175;
			case 0xA1: string[i] = 64;
			default: string[i] = text[i];
		}
	}
	string[len] = EOS;
	return string;
}

public OnPlayerConnect(playerid)
{
	g_TaskbarCount[playerid] = 0;
	g_TaskbarOrderCounter[playerid] = 0;

	for(new i = 0; i < MAX_TASKBAR; i++)
	{
		TaskInfo::taskActive[playerid][i]      = false;
		TaskInfo::taskOrder[playerid][i]       = 0;
		TaskInfo::taskProgress[playerid][i]    = 0;
		TaskInfo::taskMaxProgress[playerid][i] = 100;
		TaskInfo::taskBarColor[playerid][i]    = TASKBAR_COLOR_FULL;
		TaskInfo::taskAutoColor[playerid][i]   = true;
		TaskInfo::taskTitle[playerid][i][0]    = EOS;
		TaskInfo::taskDesc[playerid][i][0]     = EOS;
		TaskInfo::taskTime[playerid][i][0]     = EOS;
		TaskInfo::taskIcon[playerid][i][0]     = EOS;

		TaskBar::[playerid][i] = INVALID_PLAYER_BAR_ID;

		new base_idx = i * MAX_TEXTDRAW_TASKBAR;
		for(new j = 0; j < MAX_TEXTDRAW_TASKBAR; j++)
		{
			TaskTD::[playerid][base_idx + j] = PlayerText:INVALID_TEXT_DRAW;
		}
	}

	#if defined NTSB_OnPlayerConnect
		return NTSB_OnPlayerConnect(playerid);
	#else
		return true;
	#endif
}

#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect NTSB_OnPlayerConnect
#if defined NTSB_OnPlayerConnect
	forward NTSB_OnPlayerConnect(playerid);
#endif

public OnPlayerDisconnect(playerid, reason)
{
	HideAllTaskBars(playerid);
	g_TaskbarOrderCounter[playerid] = 0;

	#if defined NTSB_OnPlayerDisconnect
		return NTSB_OnPlayerDisconnect(playerid, reason);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect NTSB_OnPlayerDisconnect
#if defined NTSB_OnPlayerDisconnect
	forward NTSB_OnPlayerDisconnect(playerid, reason);
#endif


//      _____ _   _ _______ ______ _____  _   _          _
//     |_   _| \ | |__   __|  ____|  __ \| \ | |   /\   | |
//       | | |  \| |  | |  | |__  | |__) |  \| |  /  \  | |
//       | | | . ` |  | |  |  __| |  _  /| . ` | / /\ \ | |
//      _| |_| |\  |  | |  | |____| | \ \| |\  |/ ____ \| |____
//     |_____|_| \_|  |_|  |______|_|  \_\_| \_/_/    \_\______|
//
//

// Devuelve el color de la barra segun el porcentaje de progreso actual.
// Verde (66-100%), Amarillo (33-65%), Rojo (0-32%).
// Parametros:
//   progress - Porcentaje entero (0 a 100)
// Retorna: color en formato RGBA (0xRRGGBBAA)
static stock Internal_GetProgressColor(progress)
{
	if(progress >= 66) return TASKBAR_COLOR_FULL;		// Verde:   66-100%
	if(progress >= 33) return TASKBAR_COLOR_MEDIUM;		// Amarillo: 33-65%
	return TASKBAR_COLOR_LOW;							// Rojo:     0-32%
}

// Busca el primer slot libre disponible para una nueva taskbar.
// Itera los slots de 0 a MAX_TASKBAR-1 y retorna el primero inactivo.
// Parametros:
//   playerid - ID del jugador
// Retorna: indice del slot libre (0 a MAX_TASKBAR-1), o -1 si todos estan ocupados
static stock Internal_GetFreeTaskSlot(playerid)
{
	for(new i = 0; i < MAX_TASKBAR; i++)
	{
		if(!TaskInfo::taskActive[playerid][i])
		{
			return i;
		}
	}
	return -1;
}

// Calcula cuantas lineas ocupa un texto en el area de descripcion del taskbar.
// Tiene en cuenta saltos explicitos (~n~) y el ajuste automatico por ancho
// maximo (~120px, 1200 unidades de fuente 1 proporcional).
// Parametros:
//   text[] - Texto de descripcion a medir
// Retorna: numero de lineas (minimo 1)
static stock Internal_GetTaskDescLines(const text[])
{
	new size = 0, lines = 1, i = 0, c;
	new maxWidth = 1200; // ancho en unidades de fuente 1 (~120px * 10)
	while ((c = text[i]) != '\0')
	{
		if (c == '~' && text[i + 1] != '\0' && text[i + 2] == '~')
		{
			if (text[i + 1] == 'n') { lines++; size = 0; }
			i += 3;
			continue;
		}
		new charW;
		if(c >= 0 && c < 176) charW = TDCharacterWidth[1][c];
		else charW = 20;

		if(size + charW > maxWidth) { lines++; size = charW; }
		else size += charW;
		i++;
	}
	return lines;
}

// Crea los textdraws y la barra de progreso de una taskbar especifica.
// Genera 6 elementos visuales: fondo, barra de titulo, titulo, descripcion,
// icono y tiempo. Los posiciona dinamicamente segun PosY (desplazamiento
// acumulado desde la base) y ajusta la altura total segun las lineas de descripcion.
// Parametros:
//   playerid       - ID del jugador
//   taskid         - Indice del slot de taskbar
//   index_textdraw - Indice base en el array g_TaskbarTD para este slot
//   Float:PosY     - Desplazamiento vertical acumulado (0.0 = primera taskbar)
// Retorna: true siempre (creacion completada)
static stock Internal_CreateTaskBarTextDraws(playerid, taskid, index_textdraw, Float:PosY)
{
	// Calcular lineas de la descripcion para posicionamiento dinamico
	new descLines = Internal_GetTaskDescLines(TaskInfo::taskDesc[playerid][taskid]);
	if(descLines < 1) descLines = 1;

	// Alturas base:
	// Header (titulo): ~13px
	// Descripcion: descLines * 10.5px aprox
	// Zona inferior (icono + barra + tiempo): ~26px
	new Float:descHeight  = float(descLines) * 10.5;
	new Float:totalHeight = 13.0 + descHeight + 26.0; // header + desc + bottom

	// Posiciones verticales dinamicas
	new Float:headerY     = 330.000 - PosY;
	new Float:descY       = 343.000 - PosY;
	new Float:bottomZoneY = descY + descHeight + 6.0;
	new Float:iconY       = bottomZoneY;
	new Float:timeY       = bottomZoneY + 2.0;
	new Float:barY        = bottomZoneY + 6.0;

	// [0] Fondo principal - altura dinamica
	TaskTD::[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 570.000000, headerY, "_");
	PlayerTextDrawFont(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawLetterSize(playerid, TaskTD::[playerid][index_textdraw], 0.000000, totalHeight / 10.0);
	PlayerTextDrawTextSize(playerid, TaskTD::[playerid][index_textdraw], 0.000000, 120.000000);
	PlayerTextDrawSetOutline(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawSetShadow(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawAlignment(playerid, TaskTD::[playerid][index_textdraw], 2);
	PlayerTextDrawColor(playerid, TaskTD::[playerid][index_textdraw], -1);
	PlayerTextDrawBackgroundColor(playerid, TaskTD::[playerid][index_textdraw], 255);
	PlayerTextDrawBoxColor(playerid, TaskTD::[playerid][index_textdraw], 128);
	PlayerTextDrawUseBox(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawSetProportional(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, TaskTD::[playerid][index_textdraw]);

	// [1] Barra de titulo
	index_textdraw++;
	TaskTD::[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 570.000000, headerY, "_");
	PlayerTextDrawFont(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawLetterSize(playerid, TaskTD::[playerid][index_textdraw], 0.000000, 1.15);
	PlayerTextDrawTextSize(playerid, TaskTD::[playerid][index_textdraw], 0.000000, 120.000000);
	PlayerTextDrawSetOutline(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawSetShadow(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawAlignment(playerid, TaskTD::[playerid][index_textdraw], 2);
	PlayerTextDrawColor(playerid, TaskTD::[playerid][index_textdraw], -1);
	PlayerTextDrawBackgroundColor(playerid, TaskTD::[playerid][index_textdraw], 255);
	PlayerTextDrawBoxColor(playerid, TaskTD::[playerid][index_textdraw], 0x3A3A3A80);
	PlayerTextDrawUseBox(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawSetProportional(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, TaskTD::[playerid][index_textdraw]);

	// [2] Titulo
	index_textdraw++;
	TaskTD::[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 570.000000, headerY, TaskInfo::taskTitle[playerid][taskid]);
	PlayerTextDrawFont(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawLetterSize(playerid, TaskTD::[playerid][index_textdraw], 0.200000, 1.15);
	PlayerTextDrawTextSize(playerid, TaskTD::[playerid][index_textdraw], 0.000000, 120.000000);
	PlayerTextDrawSetOutline(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawSetShadow(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawAlignment(playerid, TaskTD::[playerid][index_textdraw], 2);
	PlayerTextDrawColor(playerid, TaskTD::[playerid][index_textdraw], -1);
	PlayerTextDrawBackgroundColor(playerid, TaskTD::[playerid][index_textdraw], 255);
	PlayerTextDrawBoxColor(playerid, TaskTD::[playerid][index_textdraw], 128);
	PlayerTextDrawUseBox(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawSetProportional(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, TaskTD::[playerid][index_textdraw]);

	// [3] Descripcion - posicion Y fija bajo el header, TextSize Y cubre las lineas
	index_textdraw++;
	TaskTD::[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 510.000000, descY, TaskInfo::taskDesc[playerid][taskid]);
	PlayerTextDrawFont(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawLetterSize(playerid, TaskTD::[playerid][index_textdraw], 0.174998, 1.000000);
	PlayerTextDrawTextSize(playerid, TaskTD::[playerid][index_textdraw], 630.000000, descHeight);
	PlayerTextDrawSetOutline(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawSetShadow(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawAlignment(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, TaskTD::[playerid][index_textdraw], -1);
	PlayerTextDrawBackgroundColor(playerid, TaskTD::[playerid][index_textdraw], 255);
	PlayerTextDrawBoxColor(playerid, TaskTD::[playerid][index_textdraw], 50);
	PlayerTextDrawUseBox(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawSetProportional(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, TaskTD::[playerid][index_textdraw]);

	// [4] Icono - se desplaza dinamicamente bajo la descripcion
	index_textdraw++;
	TaskTD::[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 515.000000, iconY, TaskInfo::taskIcon[playerid][taskid]);
	PlayerTextDrawFont(playerid, TaskTD::[playerid][index_textdraw], 4);
	PlayerTextDrawLetterSize(playerid, TaskTD::[playerid][index_textdraw], 0.600000, 2.000000);
	PlayerTextDrawTextSize(playerid, TaskTD::[playerid][index_textdraw], 13.000000, 13.000000);
	PlayerTextDrawSetOutline(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawSetShadow(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawAlignment(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, TaskTD::[playerid][index_textdraw], -1);
	PlayerTextDrawBackgroundColor(playerid, TaskTD::[playerid][index_textdraw], 255);
	PlayerTextDrawBoxColor(playerid, TaskTD::[playerid][index_textdraw], 50);
	PlayerTextDrawUseBox(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawSetProportional(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, TaskTD::[playerid][index_textdraw]);

	// [5] Tiempo - se desplaza dinamicamente bajo la descripcion
	index_textdraw++;
	TaskTD::[playerid][index_textdraw] = CreatePlayerTextDraw(playerid, 565.000000, timeY, TaskInfo::taskTime[playerid][taskid]);
	PlayerTextDrawFont(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawLetterSize(playerid, TaskTD::[playerid][index_textdraw], 0.174998, 1.000000);
	PlayerTextDrawTextSize(playerid, TaskTD::[playerid][index_textdraw], 630.000000, 0.000000);
	PlayerTextDrawSetOutline(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawSetShadow(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawAlignment(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawColor(playerid, TaskTD::[playerid][index_textdraw], -1);
	PlayerTextDrawBackgroundColor(playerid, TaskTD::[playerid][index_textdraw], 255);
	PlayerTextDrawBoxColor(playerid, TaskTD::[playerid][index_textdraw], 50);
	PlayerTextDrawUseBox(playerid, TaskTD::[playerid][index_textdraw], 0);
	PlayerTextDrawSetProportional(playerid, TaskTD::[playerid][index_textdraw], 1);
	PlayerTextDrawShow(playerid, TaskTD::[playerid][index_textdraw]);

	// [6] Barra de progreso - se desplaza dinamicamente bajo la descripcion
	new Float:percent = (float(TaskInfo::taskProgress[playerid][taskid]) * 100.0) / float(TaskInfo::taskMaxProgress[playerid][taskid]);
	TaskBar::[playerid][taskid] = CreatePlayerProgressBar(playerid, 530.000000, barY, 35.000000, 3.000000, TaskInfo::taskBarColor[playerid][taskid], 100.000000, 0);
	SetPlayerProgressBarValue(playerid, TaskBar::[playerid][taskid], percent);
	ShowPlayerProgressBar(playerid, TaskBar::[playerid][taskid]);

	return true;
}

// Destruye todos los textdraws y barras de progreso activos del jugador,
// luego los recrea ordenados por taskOrder (mas recientes arriba) con
// posicionamiento vertical dinamico segun las lineas de cada descripcion.
// Parametros:
//   playerid - ID del jugador
// Retorna: cantidad de taskbars que fueron recreadas, o false si no hay ninguna activa
static stock Internal_UpdateAllTaskBars(playerid)
{
	// Destruir todos los textdraws y progressbars existentes
	for(new i = 0; i < MAX_TASKBAR; i++)
	{
		new base_idx = i * MAX_TEXTDRAW_TASKBAR;
		for(new j = 0; j < MAX_TEXTDRAW_TASKBAR; j++)
		{
			if(TaskTD::[playerid][base_idx + j] == PlayerText:INVALID_TEXT_DRAW) continue;
			PlayerTextDrawDestroy(playerid, TaskTD::[playerid][base_idx + j]);
			TaskTD::[playerid][base_idx + j] = PlayerText:INVALID_TEXT_DRAW;
		}

		if(TaskBar::[playerid][i] != INVALID_PLAYER_BAR_ID)
		{
			DestroyPlayerProgressBar(playerid, TaskBar::[playerid][i]);
			TaskBar::[playerid][i] = INVALID_PLAYER_BAR_ID;
		}
	}

	if(g_TaskbarCount[playerid] == 0) return false;

	// Recolectar slots activos
	new sortedTasks[MAX_TASKBAR];
	new sortedCount = 0;

	for(new i = 0; i < MAX_TASKBAR; i++)
	{
		if(TaskInfo::taskActive[playerid][i])
		{
			sortedTasks[sortedCount++] = i;
		}
	}

	// Bubble sort descendente por taskOrder (mas recientes primero)
	for(new i = 0; i < sortedCount - 1; i++)
	{
		for(new j = i + 1; j < sortedCount; j++)
		{
			if(TaskInfo::taskOrder[playerid][sortedTasks[i]] < TaskInfo::taskOrder[playerid][sortedTasks[j]])
			{
				new temp   = sortedTasks[i];
				sortedTasks[i] = sortedTasks[j];
				sortedTasks[j] = temp;
			}
		}
	}

	// Recrear taskbars en orden con altura dinamica por descripcion
	new Float:PosY = 0.0;
	for(new i = 0; i < sortedCount; i++)
	{
		new t = sortedTasks[i];
		Internal_CreateTaskBarTextDraws(playerid, t, (t * MAX_TEXTDRAW_TASKBAR), PosY);
		// Altura dinamica: header(13) + desc(lineas*10.5) + bottom(20) + margen(5)
		new descL = Internal_GetTaskDescLines(TaskInfo::taskDesc[playerid][t]);
		if(descL < 1) descL = 1;
		PosY += 13.0 + (float(descL) * 10.5) + 26.0 + 5.0;
	}
	return sortedCount;
}

//      _____ _   _ _______ ______ _____  ______      _____ ______
//     |_   _| \ | |__   __|  ____|  __ \|  ____/\   / ____|  ____|
//       | | |  \| |  | |  | |__  | |__) | |__ /  \ | |    | |__
//       | | | . ` |  | |  |  __| |  _  /|  __/ /\ \| |    |  __|
//      _| |_| |\  |  | |  | |____| | \ \| | / ____ \ |____| |____
//     |_____|_| \_|  |_|  |______|_|  \_\_|/_/    \_\_____|______|
//
//

/*
	ShowTaskBar
	Crea y muestra una nueva barra de tarea en pantalla.

	Parametros:
		playerid    - ID del jugador
		title[]     - Titulo de la tarea (soporta codigos de color ~r~, ~y~, etc.)
		description[] - Descripcion de la tarea
		time[]      - Texto del tiempo a mostrar (ej: "05:00 min")
		maxprogress - Valor maximo de progreso (por defecto 100)
		icon[]      - Nombre del sprite a usar como icono (ej: "LD_GRAV:timer")
		progress    - Progreso inicial (0 a maxprogress, por defecto 0)

	Retorna:
		ID de la taskbar creada (0 a MAX_TASKBAR-1)
		-1 si no hay slots disponibles

	Ejemplo:
		new taskid = ShowTaskBar(playerid, "~y~Mineria", "~w~Extrayendo mineral", "05:00 min", 100, "LD_GRAV:timer", 0);
*/
stock ShowTaskBar(playerid, const title[], const description[], const time[] = "", maxprogress = 100, const icon[] = "LD_GRAV:timer", progress = 0)
{
	if(maxprogress <= 0) maxprogress = 100;
	if(progress < 0)     progress = 0;
	if(progress > maxprogress) progress = maxprogress;

	new taskid = Internal_GetFreeTaskSlot(playerid);
	if(taskid == -1) return -1;

	new Float:percent = (float(progress) * 100.0) / float(maxprogress);

	strmid(TaskInfo::taskTitle[playerid][taskid], TextDrawFixedLatinoText(title),       0, MAX_LENGTH_TITLE);
	strmid(TaskInfo::taskDesc[playerid][taskid],  TextDrawFixedLatinoText(description), 0, MAX_LENGTH_MESSAGE);
	strmid(TaskInfo::taskTime[playerid][taskid],  time, 0, 32);
	strmid(TaskInfo::taskIcon[playerid][taskid],  icon, 0, MAX_LENGTH_ICON);
	TaskInfo::taskProgress[playerid][taskid]    = progress;
	TaskInfo::taskMaxProgress[playerid][taskid] = maxprogress;
	TaskInfo::taskActive[playerid][taskid]      = true;
	TaskInfo::taskOrder[playerid][taskid]       = g_TaskbarOrderCounter[playerid]++;
	TaskInfo::taskAutoColor[playerid][taskid]   = true;
	TaskInfo::taskBarColor[playerid][taskid]    = Internal_GetProgressColor(floatround(percent));

	g_TaskbarCount[playerid]++;

	Internal_UpdateAllTaskBars(playerid);

	return taskid;
}

/*
	HideTaskBar
	Oculta y destruye una taskbar especifica, liberando su slot.

	Parametros:
		playerid - ID del jugador
		&taskid  - ID de la taskbar a ocultar (se resetea a INVALID_TASKBAR_ID)

	Retorna:
		true si se oculto correctamente
		false si el taskid es invalido o ya estaba oculta

	Ejemplo:
		HideTaskBar(playerid, taskid);
*/
stock HideTaskBar(playerid, &taskid)
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	TaskInfo::taskActive[playerid][taskid]      = false;
	TaskInfo::taskTitle[playerid][taskid][0]    = EOS;
	TaskInfo::taskDesc[playerid][taskid][0]     = EOS;
	TaskInfo::taskTime[playerid][taskid][0]     = EOS;
	TaskInfo::taskIcon[playerid][taskid][0]     = EOS;
	TaskInfo::taskProgress[playerid][taskid]    = 0;
	TaskInfo::taskMaxProgress[playerid][taskid] = 0;
	TaskInfo::taskOrder[playerid][taskid]       = 0;
	TaskInfo::taskBarColor[playerid][taskid]    = 0;
	TaskInfo::taskAutoColor[playerid][taskid]   = false;

	taskid = INVALID_TASKBAR_ID;

	if(g_TaskbarCount[playerid] > 0)
		g_TaskbarCount[playerid]--;

	Internal_UpdateAllTaskBars(playerid);
	return true;
}

/*
	UpdateTaskBarProgress
	Actualiza el valor de progreso de la barra y ajusta su color si autoColor esta activo.

	Parametros:
		playerid - ID del jugador
		taskid   - ID de la taskbar
		progress - Nuevo valor (0 a taskMaxProgress)

	Retorna:
		true si se actualizo correctamente
		false si el taskid es invalido

	Nota:
		El progreso es relativo al maximo establecido en ShowTaskBar.
		Si maxprogress = 20, la barra estara llena cuando progress = 20.

	Ejemplo:
		UpdateTaskBarProgress(playerid, taskid, 50); // 50 de 100 = 50%
*/
stock UpdateTaskBarProgress(playerid, taskid, progress)
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	new maxprogress = TaskInfo::taskMaxProgress[playerid][taskid];
	if(maxprogress <= 0) maxprogress = 100;

	if(progress < 0)           progress = 0;
	if(progress > maxprogress) progress = maxprogress;

	TaskInfo::taskProgress[playerid][taskid] = progress;

	new Float:percent  = (float(progress) * 100.0) / float(maxprogress);
	new percentInt     = floatround(percent);

	if(TaskInfo::taskAutoColor[playerid][taskid])
	{
		new color = Internal_GetProgressColor(percentInt);
		TaskInfo::taskBarColor[playerid][taskid] = color;
		SetPlayerProgressBarColour(playerid, TaskBar::[playerid][taskid], color);
	}

	SetPlayerProgressBarValue(playerid, TaskBar::[playerid][taskid], percent);
	return true;
}

/*
	UpdateTaskBarMaxProgress
	Actualiza el valor maximo de progreso de la barra.
	Si el progreso actual supera el nuevo maximo, se ajusta automaticamente.

	Parametros:
		playerid    - ID del jugador
		taskid      - ID de la taskbar
		maxprogress - Nuevo valor maximo (debe ser > 0)

	Retorna:
		true si se actualizo correctamente
		false si el taskid es invalido

	Ejemplo:
		UpdateTaskBarMaxProgress(playerid, taskid, 50); // Nuevo maximo: 50
*/
stock UpdateTaskBarMaxProgress(playerid, taskid, maxprogress)
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	if(maxprogress <= 0) maxprogress = 100;
	TaskInfo::taskMaxProgress[playerid][taskid] = maxprogress;

	new currentProgress = TaskInfo::taskProgress[playerid][taskid];
	if(currentProgress > maxprogress)
	{
		currentProgress = maxprogress;
		TaskInfo::taskProgress[playerid][taskid] = currentProgress;
	}

	return UpdateTaskBarProgress(playerid, taskid, currentProgress);
}

/*
	UpdateTaskBarTime
	Actualiza el texto de tiempo mostrado en la taskbar.

	Parametros:
		playerid - ID del jugador
		taskid   - ID de la taskbar
		time[]   - Nuevo texto del tiempo (ej: "02:30 min")

	Retorna:
		true si se actualizo correctamente
		false si el taskid es invalido

	Ejemplo:
		UpdateTaskBarTime(playerid, taskid, "02:30 min");
*/
stock UpdateTaskBarTime(playerid, taskid, const time[])
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	strmid(TaskInfo::taskTime[playerid][taskid], time, 0, 32);

	new index_td = (taskid * MAX_TEXTDRAW_TASKBAR) + 5;
	PlayerTextDrawSetString(playerid, TaskTD::[playerid][index_td], TaskInfo::taskTime[playerid][taskid]);
	return true;
}

/*
	UpdateTaskBarDescription
	Actualiza el texto de descripcion de la taskbar.

	Parametros:
		playerid      - ID del jugador
		taskid        - ID de la taskbar
		description[] - Nueva descripcion (soporta codigos de color)

	Retorna:
		true si se actualizo correctamente
		false si el taskid es invalido

	Ejemplo:
		UpdateTaskBarDescription(playerid, taskid, "~g~!Tarea completada!");
*/
stock UpdateTaskBarDescription(playerid, taskid, const description[])
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	strmid(TaskInfo::taskDesc[playerid][taskid], TextDrawFixedLatinoText(description), 0, MAX_LENGTH_MESSAGE);

	Internal_UpdateAllTaskBars(playerid);
	return true;
}

/*
	UpdateTaskBarIcon
	Actualiza el icono (sprite) de la taskbar.

	Parametros:
		playerid - ID del jugador
		taskid   - ID de la taskbar
		icon[]   - Nombre del nuevo sprite (ej: "LD_BEAT:chit", "LD_GRAV:timer")

	Retorna:
		true si se actualizo correctamente
		false si el taskid es invalido

	Ejemplo:
		UpdateTaskBarIcon(playerid, taskid, "LD_BEAT:chit");
*/
stock UpdateTaskBarIcon(playerid, taskid, const icon[])
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	strmid(TaskInfo::taskIcon[playerid][taskid], icon, 0, MAX_LENGTH_ICON);

	new index_td = (taskid * MAX_TEXTDRAW_TASKBAR) + 4;
	PlayerTextDrawSetString(playerid, TaskTD::[playerid][index_td], TaskInfo::taskIcon[playerid][taskid]);
	return true;
}

/*
	SetTaskBarColor
	Establece un color personalizado para la barra de progreso.
	Si autocolor es true, ignora el color dado y usa el automatico segun el porcentaje.

	Parametros:
		playerid  - ID del jugador
		taskid    - ID de la taskbar
		color     - Color en formato RGBA (ej: 0xFF0000FF para rojo fijo)
		autocolor - true para color automatico segun progreso (por defecto false)

	Retorna:
		true si se actualizo correctamente
		false si el taskid es invalido

	Ejemplo:
		SetTaskBarColor(playerid, taskid, 0xFF0000FF, false); // Rojo fijo
		SetTaskBarColor(playerid, taskid, 0, true);           // Auto-color activado
*/
stock SetTaskBarColor(playerid, taskid, color, bool:autocolor = false)
{
	if(!IsTaskBarActive(playerid, taskid))      return false;
	if(!TaskInfo::taskActive[playerid][taskid]) return false;

	TaskInfo::taskAutoColor[playerid][taskid] = autocolor;

	if(autocolor)
	{
		new Float:percent = (float(TaskInfo::taskProgress[playerid][taskid]) * 100.0) / float(TaskInfo::taskMaxProgress[playerid][taskid]);
		color = Internal_GetProgressColor(floatround(percent));
	}

	TaskInfo::taskBarColor[playerid][taskid] = color;
	SetPlayerProgressBarColour(playerid, TaskBar::[playerid][taskid], color);

	return true;
}

/*
	IsTaskBarActive
	Verifica si una taskbar esta activa.

	Parametros:
		playerid - ID del jugador
		taskid   - ID de la taskbar a verificar

	Retorna:
		true si esta activa
		false si no esta activa o el taskid es invalido

	Ejemplo:
		if(!IsTaskBarActive(playerid, taskid)) { }
*/
stock IsTaskBarActive(playerid, taskid)
{
	if(!(0 <= playerid < MAX_PLAYERS)) return false;
	if(taskid <= INVALID_TASKBAR_ID || taskid >= MAX_TASKBAR) return false;
	return TaskInfo::taskActive[playerid][taskid];
}

/*
	GetActiveTaskBarsCount
	Obtiene la cantidad de taskbars activas del jugador.

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de taskbars activas (0 a MAX_TASKBAR)

	Ejemplo:
		new count = GetActiveTaskBarsCount(playerid);
*/
stock GetActiveTaskBarsCount(playerid)
{
	if(!(0 <= playerid < MAX_PLAYERS)) return 0;
	return g_TaskbarCount[playerid];
}

/*
	HideAllTaskBars
	Oculta y destruye todas las taskbars activas del jugador.

	Parametros:
		playerid - ID del jugador

	Retorna:
		Cantidad de taskbars que fueron ocultadas

	Ejemplo:
		new count = HideAllTaskBars(playerid);
*/
stock HideAllTaskBars(playerid)
{
	if(!(0 <= playerid < MAX_PLAYERS)) return 0;
	new count = 0;
	for(new i = 0; i < MAX_TASKBAR; i++)
	{
		if(TaskInfo::taskActive[playerid][i])
		{
			new tid = i;
			HideTaskBar(playerid, tid);
			count++;
		}
	}
	return count;
}

/*
	GetTaskBarProgress
	Obtiene el progreso actual y el maximo de una taskbar.

	Parametros:
		playerid     - ID del jugador
		taskid       - ID de la taskbar
		&maxprogress - Variable de salida donde se guardara el progreso maximo

	Retorna:
		Progreso actual (0 a maxprogress)
		-1 si el taskid es invalido

	Ejemplo:
		new maxprog;
		new progress = GetTaskBarProgress(playerid, taskid, maxprog);
		// progress = 50, maxprog = 100  ?  50%
*/
stock GetTaskBarProgress(playerid, taskid, &maxprogress = 0)
{
	if(!IsTaskBarActive(playerid, taskid))      return -1;
	if(!TaskInfo::taskActive[playerid][taskid]) return -1;

	maxprogress = TaskInfo::taskMaxProgress[playerid][taskid];
	return TaskInfo::taskProgress[playerid][taskid];
}

// # END :D
