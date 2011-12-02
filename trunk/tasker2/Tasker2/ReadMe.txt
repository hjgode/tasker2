========================================================================
    CONSOLE APPLICATION : Tasker2 Project Overview
========================================================================

Tasker2 - a tool to schedule start and kill applications

v2.33
	added code to recognize pre-time launches, where the task to launch is in the future
	
v2.32
	removed UpdateAll codes
	
v2.31
	reversed change of createNextSchedule
	
	correct reg format:
		REGEDIT4

		[HKLM\Software\Tasker]
		"dbgLevel":DWORD=0
		"maxDelay":DWORD=1
		"Version":DWORD=231
		
		[HKLM\Software\Tasker\Task1]
		"active":DWORD=1
		"exe":="\Windows\fexplore.exe"
		"args":="\My Documents"
		"start":="0430"
		"startOnAConly":DWORD=0;
		"stop":="0530"
		"interval" :="2400"
		;informational use only:
		"NextStart":="201111041430"
		"NextStop" :="201111041530"

v2.30
	changed getSTfromString to not return an error for '0000' (start/stop times)
	changed createNextSchedule to use createDelayedNextSchedule
	changed regReadDbgLevel to write always 0 as dbgLevel
	
v2.29
	regRW.cpp: changed all access to registry.h/cpp and use direct reg access
	changed for APP_RUN_AFTER_TIME_CHANGE and APP_RUN_AFTER_TZ_CHANGE:
		reinit used current time
		check again for valid Date
	
v2.28
	added nclog msg in RegReadAll, depends on new registry key DbgLevel. RegReadAll uses dbgLevel>4
	added regReadDbgLevel to control which msgs are logged, currently dbgLevel=5 will make RegRead msgs visible
	changed code to use only one CurrentTime
	moved check for mutex to very beginning of main()
		so, if an instance is already running, 
		the launch time and the time tasker2 will continue to run may be different!
	changed mutex code and use initial ownership
		found in log that lines were mixed between concurrent processes, now the lines
		show that one process really waits until the other ended:
	added OpenKey(szCurrentKey) before every attempt to read from registry
		there was an issue that the global subkey changed for no reason
	
v2.27
	moved checkForVaildDate after mutex check
	added scheduleAllEvents before exit on non-valid date
	
v2.26
	added date time validation: if before 11.2011 just exit
	added code to nclog to identify running instance, log lines start with module handle
	added code to recognize Time and TimeZone changes: RunAppAtTimeChangeEvents()
	fixed getNextTime, else 'do while' changed to "while (!isNewer(stStart1, stBegin));// (uStart>uBegin);"
	fixed SYSTEMTIME DT_Add(SYSTEMTIME& Date, short Years, short Months, short Days, short Hours, 
	 short Minutes, short Seconds, short Milliseconds) for non-zero Days argument
	cosmetic changes
	
v2.25
	added registry based parameter maxDelay (minutes) for delayed schedule recognition
	"maxDelay":DWORD=1
	
v2.24
	changed getNextTime for calculation if schedule is in past

v2.23
	changed createDelayedNextSchedule for past and future delayed schedules
	
v2.22
	added createDelayedNextSchedule
	added getNextTime
	changed delayed schedule calculation for next start time
	example:
		time is 01.01.2003, 12:00
		first tasker2:
			next start is 01.01.2003 12:11:42 / interval 10min
		time is 01.02.2009, 00:00 
		tasker2 run
			next start is 01.02.2009 12:05 <= Problem, task is far in future
		WITH NEW CALC we get:
			next start is 01.02.2009 00:05
			
	added fixSystemTime to fix wrong start/stop entries
	
v2.21
	changed isNewer to NOT use seconds in compare
	
v2.20
	changed time diff code - deleayed schedule recognition
	
	changed way to calc next schedule
	
	changed registry usage!
	REGEDIT4

	[HKLM\Software\Tasker\Task1]
	"active":DWORD=1
	"exe":="\Windows\fexplore.exe"
	"arg":="\My Documents"
	"start":="0430"
	"NextStart":="201111041430"
	"startOnAConly":DWORD=0;
	"stop":="0530"
	"NextStop" :="201111041530"
	"interval" :="2400"
	
	start and stop entries will be used to calc next start/stop and will be updated by tasker2
	NextStart and NextStop are now only for information
	
	version number in reg is now unused
	updateall flag in registry is now unused
	
v2.10a
	added reg and code to not start a task if not on AC power

	startOnAConly: default is FALSE, 
		if =1 then TRUE and exe will only started if device is on AC power
		if not 1 then FALSE and exe will be started regardless of external power
		
	<pre>
	REGEDIT4

	[HKLM\Software\Tasker\Task1]
	"active":DWORD=1
	"exe":="\Windows\fexplore.exe"
	"arg":="\My Documents"
	"NextStart":="201111041423"
	"startOnAConly":DWORD=0;
	"NextStop" :="201111041523"
	"interval" :="2400"
	</pre>
	
	gug fix: changed calculating next schedule to not add interval two times
	
v2.10

	changed registry and code:
	<pre>
	REGEDIT4

	[HKLM\Software\Tasker\Task1]
	"active":DWORD=1
	"exe":="\Windows\fexplore.exe"
	"arg":="\My Documents"
	"NextStart":="201111041423"
	"NextStop" :="201111041523"
	"interval" :="2400"
	</pre>

	v2.10 changed and added some log msg
	
	v2.10 is able to schedule tasks with interval more than 24 hours as now the date is used too (limit is 99:59)
	
	v2.10 will update existing reg values to new datetime strings, you do not need to add them manually, but feel
	free to do so.
	
	v2.10 will recognize delayed schedules (many schedules after hibernate) and re-schedules then for next 
	possible interval

	//changed code to recognize flood schedules aka delayed schedules due to device has been in hibernate mode in between
	
	//################################################################################################################
	//if the device has been hibernated in between, all scheduled tasks will be launched at once in no specific order
	//now, disregard these schedules and create new ones
	//recognition of a after-hibernate schedule wave can be done by comparing the scheduled time with the current time
	//if they dont match in between some minutes, the schedules are to be ignored
	//but what time should I use for the new schedules? Possibly a schedule was issued for 02:00 and interval 0100, so
	//it would start at 3:00, 4:00 and so on
	//now after hibernate it may be 16:30, but the last schedule was for 04:00 (see registry)
	//I have to add the interval to the scheduled time until it is later than the current time!
	//the new schedule would than be 17:00 (added 0100 to 04:00 until > currentTime)
	//################################################################################################################

	v2.10 added full data time control using reversed date/time values (see NextStart and NextStop)
	v2.10 NextStart and NextStop uses datetime format yyyyMMddHHmm, ie 201111090935 for 09. nov. 2011 09:35
	
	v2.10 has also some minro bug fixes
	
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++	

Tasker2 is based on HowToRunAnAppPeriodically[http://www.hjgode.de/wp/2009/07/14/howto-run-an-application-periodically/]. 
Tasker2 will start and kill other applications using the event notification list of windows mobile devices. I will call 
this list a schedule list.

If you have an application that you will run every day or every hour and stop (kill) it after some time, 
tasker2 maybe of help.

As Tasker2 uses the event notification list of the OS, the tasks wil be executed also if the device is in
suspended mode. Tasker2 itself will not run all the time, it will only run shortly to do tasks and re-schedule 
them. Tasker2 will setup new schedules to itself in the event notification list based on your settings.

Tasker2 is controlled via the registry. You can define up to 10 tasks. Each task holds the exe to start/kill,
the args to use, the next scheduled start and kill time, the interval between the schedules and the option 
to enable/disable a task.

Every task is defined as a subkey in the registry. You can setup up to 10 tasks. The subkeys have to 
called continously from 1 to 10 (task1 to task10).

[obsolete, see version 2.10]
Example for a task entry
<pre>
REGEDIT4

[HKLM\Software\Tasker\Task1]
"active":DWORD=1
"exe":="\Windows\fexplore.exe"
"arg":="\My Documents"
"start":="1423"
"stop":="1523"
"interval":="2400"
</pre>

with these settings, \Windows\fexplore.exe will be started with the argument "\My Documents" every 
24 hours at 14:23 and stopped every 24 hours at 15:23.

The Start and Stop time will be updated on each start or kill schedule. So they reflect the next time
the schedule will happen.

[obsolete, see version 2.10]
Example 2
<pre>
REGEDIT4

[HKLM\Software\Tasker\Task1]
"active":DWORD=1
"exe":="\Windows\fexplore.exe"
"arg":="\My Documents"
"start":="1423"
"stop":="1523"
"interval":="2400"

[HKLM\Software\Tasker\Task2]
"active":DWORD=1
"exe":="\Windows\notepad.exe"
"arg":="\My Documents"
"start":="0100"
"stop":="0130"
"interval":="0030"
</pre>

with these settings, \Windows\fexplore.exe will be started with the argument "\My Documents" every 
24 hours at 14:23 and stopped every 24 hours at 15:23. The second task starts \Windows\notepad.exe with "\My Documents" 
at 01:00 and stop it at 01:30 and that will be rescheduled every 30 minutes.


Tasker2 reschedules itself for every schedule and so updates the start and stop times and does start or kill the 
specified executable.

An event notification entry done by Tasker2 will look like this:
<pre>"\Windows\Tasker2.exe" "-s task1"</pre>
to start app defined for task1 and re-schedule task
-or-
<pre>"\Windows\Tasker2.exe" "-k task1"</pre>
to kill app defined for task1 and re-schedule task

You just need to setup the registry one time and then launch tasker2.exe one time. Tasker2 will then clear all tasker 
schedules and setup new start and kill schedules for every active task. It does not matter, if you launch Tasker2 
accidently as all information is held in the registry and all tasker schedules are cleared and created from scratch in
the notification database based on the registry. If you want Tasker2 to update all schedules and save changed times to
the registry after a first call of Tasker2 you have to set HKLM\Software\Tasker\UpdateAll to 1.

Tasker arguments

You normally dont need to run tasker with any argument. But if you want to remove a task or clear all tasks from the
event notification list, you can use -r or -c.
<pre>
-s taskX	start executable of task X
-k taskX	kill executable of task X
-r taskX	remove task X from event notification list and disable it for further schedules
-a taskX	add task X to event notification list and enable it for further schedules
-c			clear all tasker entries from event notification list
[noArgs]	re-schedule all tasks into event notification list, update times in reg if HKLM\Software\Tasker\UpdateAll==1
-d			dump all notifications list entries
</pre>

Notes

Tasker2 writes a log file to the same dir it was started from.

If you move Tasker2.exe after an initial run. The schedules will not meet the new exe location. You should run "tasker2.exe -c" 
first and then move the app. Then set registry HKLM\Software\Tasker\UpdateAll==1 and start Tasker2.exe one time to place all
schedules with the new exe location in the notification list.

The difference between start and stop times should be at least the interval value. Do not setup a start for 13:00 and a stop for 13:30 
and then set the interval to 0020 (20 minutes). You will get 
start schedules for 13:20, 13:40 ... and 
stop schedules for 13:50, 14:10 ... 
The schedules will overlap then and the first schedule is at 13:20, stop at 13:30. You see the interval between start and stop is not 20 
minutes as defined but 10 minutes here.

	
/////////////////////////////////////////////////////////////////////////////s