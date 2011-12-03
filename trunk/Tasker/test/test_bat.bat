@echo off

ECHO ############## START ############### >test_run.txt

pdel \tasker2.exe.log.txt >NUL

ECHO ++++++++++++++ TEST1 ++++++++++++++++ >>test_run.txt
pregutl HKLM\Software\Tasker  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
ECHO set time manually to 01.01.2003 12:00 >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 200301011200
prun \tasker2.exe
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST1  --------------- >>test_run.txt
ECHO .>>test_run.txt

ECHO ++++++++++++++ TEST2 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
ECHO set time manually to 01.02.2009 12:00 >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 200902011200
prun \tasker2.exe
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST2  --------------- >>test_run.txt
ECHO .>>test_run.txt

ECHO ++++++++++++++ TEST3 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
ECHO set time manually to 02.12.2011 15:00 >>test_run.txt
ECHO 	tasker.exe AppRunAfterTimeChange   >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112021500
prun \tasker2.exe AppRunAfterTimeChange
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST3  --------------- >>test_run.txt
ECHO .>>test_run.txt

ECHO ++++++++++++++ TEST4 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
ECHO set time manually to 02.12.2011 19:52 >>test_run.txt
ECHO 	tasker.exe AppRunAfterTimeChange   >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112021952
prun \tasker2.exe AppRunAfterTimeChange
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST4  --------------- >>test_run.txt
ECHO .>>test_run.txt

ECHO ++++++++++++++ TEST5 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simulates a scheduler call at correct time  >>test_run.txt
echo set time manually to 02.12.2011 19:55  >>test_run.txt
echo 	tasker.exe -s task2  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112021955
prun \tasker2.exe -s task2
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST5  --------------- >>test_run.txt
ECHO .>>test_run.txt

ECHO ++++++++++++++ TEST6 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simulates a scheduler call at correct time  >>test_run.txt
echo set time manually to 02.12.2011 20:00  >>test_run.txt
echo 	tasker.exe -s task1  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112022000
prun \tasker2.exe -s task1
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST6  --------------- >>test_run.txt
ECHO .>>test_run.txt

REM ###	simululates a scheduler call at correct time
REM set time manually to 03.12.2011 0600
REM 	tasker.exe -k task1

ECHO ++++++++++++++ TEST7 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simulates a scheduler call at correct time  >>test_run.txt
echo set time manually to 03.12.2011 0600  >>test_run.txt
echo 	tasker.exe -k task1  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112030600
prun \tasker2.exe -k task1
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST7  --------------- >>test_run.txt
ECHO .>>test_run.txt

rem ###	simululates a scheduler call at correct time
rem set time manually to 03.12.2011 0605
rem 	tasker.exe -k task2

ECHO ++++++++++++++ TEST8 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simulates a scheduler call at correct time  >>test_run.txt
echo set time manually to 03.12.2011 0605  >>test_run.txt
echo 	tasker.exe -k task1  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112030605
prun \tasker2.exe -k task2
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST8  --------------- >>test_run.txt
ECHO .>>test_run.txt

rem ###	simululates a delayed call
rem set time manually to 04.12.2011 1500
rem	tasker.exe -s task2

ECHO ++++++++++++++ TEST9 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simululates a delayed call  >>test_run.txt
echo set time manually to 04.12.2011 1500  >>test_run.txt
ECHO 	tasker2.exe -s task2  				>>test_run.txt
ECHO 	tasker2.exe -s task1  				>>test_run.txt
ECHO 	tasker2.exe -k task1  				>>test_run.txt
ECHO 	tasker2.exe -k task2  				>>test_run.txt
echo 	tasker2.exe AppRunAfterTimeChange  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112041500
prun \tasker2.exe -s task2
prun \tasker2.exe -s task1
prun \tasker2.exe -k task1
prun \tasker2.exe -k task2
prun \tasker2.exe AppRunAfterTimeChange
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST9  --------------- >>test_run.txt
ECHO .>>test_run.txt

rem ###	simululates a premature TimeChangeEvent
rem set time manually to 04.12.2011 0000
rem 	tasker.exe AppRunAfterTimeChange

ECHO ++++++++++++++ TEST10 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simululates a delayed call  >>test_run.txt
echo set time manually to 04.12.2011 0000  >>test_run.txt
echo 	tasker2.exe AppRunAfterTimeChange  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112040000
prun \tasker2.exe AppRunAfterTimeChange
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST10  --------------- >>test_run.txt
ECHO .>>test_run.txt

@echo on
