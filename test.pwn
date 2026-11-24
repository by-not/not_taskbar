#include <a_samp>
#include <progress2>
#define MAX_TASKBAR 3
#include "not_taskbar.inc"

main() {}

public OnPlayerSpawn(playerid)
{
	new tid = ShowTaskBar(playerid, "~y~Mission", "Deliver package", "05:00", 100, "LD_GRAV:timer");
	if(IsTaskBarActive(playerid, tid))
	{
		UpdateTaskBarProgress(playerid, tid, 50);
		UpdateTaskBarMaxProgress(playerid, tid, 100);
		UpdateTaskBarTime(playerid, tid, "02:30");
		UpdateTaskBarDescription(playerid, tid, "Almost there!");
		UpdateTaskBarIcon(playerid, tid, "LD_GRAV:timer");
		SetTaskBarColor(playerid, tid, 0x00FF00FF);
		new maxp, p = GetTaskBarProgress(playerid, tid, maxp);
		new c = GetActiveTaskBarsCount(playerid);
		#pragma unused p, maxp, c
		HideTaskBar(playerid, tid);
	}
	HideAllTaskBars(playerid);
	return 1;
}
