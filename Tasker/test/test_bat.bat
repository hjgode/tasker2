@echo off

ECHO ############## START ###############
echo  MAKE SHURE THE RADIOS ARE OFF TO 
echo  AVOID AUTOMATIC TIME SYNCS!
echo .
PAUSE Press any key to start

ECHO ############## START ############### >test_run.txt

echo Importing test reg keys...
echo Importing test reg keys... >>test_run.txt
pregutl @tasker2.reg >>test_run.txt

pdel \tasker2.exe.log.txt >NUL

ECHO TEST1...
ECHO 	set time manually to 01.01.2003 12:00
echo 	run tasker2.exe
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

ECHO TEST2...
ECHO 	set time manually to 01.02.2009 12:00
echo 	run tasker2.exe
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

ECHO TEST3...
ECHO 	set time manually to 02.12.2011 15:00
ECHO 	tasker.exe AppRunAfterTimeChange
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

ECHO TEST4...
ECHO 	set time manually to 02.12.2011 19:52
ECHO 	tasker.exe AppRunAfterTimeChange
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

ECHO TEST5...
echo 	simulates a scheduler call at correct time
echo 	set time manually to 02.12.2011 19:55
echo 	tasker.exe -s task2
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

ECHO TEST6...
echo 	simulates a scheduler call at correct time
echo 	set time manually to 02.12.2011 20:00
echo 	tasker.exe -s task1
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

ECHO TEST7...
echo 	simululates a scheduler call at correct time
echo 	set time manually to 03.12.2011 0600
echo 	tasker.exe -k task1
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

ECHO TEST8...
echo 	simululates a scheduler call at correct time
echo 	set time manually to 03.12.2011 0605
echo 	tasker.exe -k task2

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


ECHO TEST9...
echo 	simululates a delayed call
echo 	set time manually to 04.12.2011 1500
ECHO 	tasker2.exe -s task2  				
ECHO 	tasker2.exe -s task1  				
ECHO 	tasker2.exe -k task1  				
ECHO 	tasker2.exe -k task2  				
echo 	tasker2.exe AppRunAfterTimeChange  
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

ECHO TEST10...
echo 	simululates a premature TimeChangeEvent
echo 	set time manually to 04.12.2011 0000
echo 	tasker.exe AppRunAfterTimeChange
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

ECHO TEST11...
echo 	simululates another premature TimeChangeEvent
echo 	set time manually to 01.12.2011 0100
echo 	tasker.exe AppRunAfterTimeChange
ECHO ++++++++++++++ TEST10 ++++++++++++++++ >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
echo ###	simululates a past time change  >>test_run.txt
echo set time manually to 01.12.2011 0100  >>test_run.txt
echo 	tasker2.exe AppRunAfterTimeChange  >>test_run.txt
ECHO ------------------------------------- >>test_run.txt
prun \SetDateTime.exe 201112010100
prun \tasker2.exe AppRunAfterTimeChange
ECHO "############# Result:" >>test_run.txt
pregutl HKLM\Software\Tasker >>test_run.txt
ECHO *************** LOG  **************** >>test_run.txt
pget -f \tasker2.exe.log.txt
type tasker2.exe.log.txt >>test_run.txt
pdel \tasker2.exe.log.txt
ECHO ---------------TEST10  --------------- >>test_run.txt
ECHO .>>test_run.txt

findstr /g:find_start.txt tasker2.exe.log.txt

@echo on
