_______________________Run Green Scheduler under NS-3_____________________________


01  - Copy the folders gpm, greenmacscheduler and subfolders to the ns-3.22/src folder

02  - Copy and replace the the attached header files and cc files with the corresponding files in ns-3.22/src/lte

03  - Copy the source file "Green_Pro.cc" to the  ns-3.22 /scratch folder

04  - ./waf --run scratch/Green_Pro.cc

05  - Program will dump the following trace files:
    
     (DlMacStats.txt ,DlPdcpStats.txt , DlRlcStats.txt , DlRsrpSinrStats.txt)

06  - Use the attached matlab script file Plot_Rlc_Stats.m to plot the DlRlcStats.txt file

07  - Run the matlab script file

08  - Open the file DlRlcStats.txt which can be found in the ns-3.22 folder

09  - After you load the trace file, the script will prompt you to enter the number of UEs, and video sizes for each UE

10 - Figures will pop up showing the results
