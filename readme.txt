_______________________Run Green Scheduler under NS-3_____________________________


1  - Copy the folders gpm, greenmacscheduler and subfolders to the ns-3.22/src folder.


2  - Copy and replace the the attached header files and cc files with the corresponding files in ns-3.22/src/lte


3  - Copy the source file "Green_Pro.cc" to the  ns-3.22 /scratch folder


4  - ./waf --run scratch/Green_Pro.cc


5  - Program will dump the following trace files:
    
     (DlMacStats.txt ,DlPdcpStats.txt , DlRlcStats.txt , DlRsrpSinrStats.txt)

6  - Use the attached matlab script file Plot_Rlc_Stats.txt to plot the DlRlcStats.txt file

7  - Run the matlab script file

8  - Browes to the DlRlcStats.txt file which can be found in ns-3.22 folder

9  - After you load the trace file, the script will prompt you to enter the number of UEs, and video Sizes for each UE

10 - Figures will pop up showing the results



