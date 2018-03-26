clc; close all; clear all;

[fn, fdir] = uigetfile('*.txt');
fid = fopen([fdir fn], 'r');
fclose(fid);

disp('_________________________________________________________________');
disp([' Script to plot statistics from NS-3 RLC traces ']);
disp([' Mohamed Shoeb & Ashraf Khresheh']);
disp([' April 2015 ']);
disp('_________________________________________________________________');

count = 1; countr = 1;

disp(' ');
disp([' ** Reading file : ', fn]);

DlRlcStats = importdata([fdir fn], '\t');
ss         = size(DlRlcStats);

disp(' ** Plotting ..');

sched_name = 'PF Scheduler';
%sched_name = 'Proportionally Fair Scheduler';

disp(' ');
no_of_ues = input('  [?] Number os users : ');

for i = 1:no_of_ues
    
d_base(i) = input(['  [?] Enter streaming rate for UE IMSI ', num2str(i), ' : ']);

end

for k = 1:no_of_ues
    
    if (k > 1)
    clear TT;
    clear TD;
    clear RD;
    clear total_tx;
    clear total_rx;
    clear DD;
    clear fullfill_v_tx;
    clear fullfill_v_rx Dv;
    clear DDx;
    clear stall;
    clear DL_temp;
    end
    
    tcount = 1;
    
    for i = 1:ss(1)
        
        if (DlRlcStats(i, 4) == k)
            
        DL_temp(tcount, 1) = DlRlcStats(i, 1);
        DL_temp(tcount, 2) = DlRlcStats(i, 2);
        DL_temp(tcount, 3) = DlRlcStats(i, 8);
        DL_temp(tcount, 4) = DlRlcStats(i, 10);
        
        tcount = tcount + 1;
        
        end
        
    end

% Initialize --------------------------------------------------------------

D     = d_base(k);
s     = size(DL_temp);
DD(1) = 0;
TD(1) = DL_temp(1, 3);
RD(1) = DL_temp(1, 4);

% Start cumulative allocation calculations---------------------------------

for i = 2:s(1)-1
    
    TT(i) = DL_temp(i, 1);
    TD(i) = TD(i-1) + DL_temp(i, 3);
    RD(i) = RD(i-1) + DL_temp(i, 4);
    
end

ftime = DL_temp(2, 1);
ltime = DL_temp(s(1)-1, 1);
    
% Initialize --------------------------------------------------------------

total_tx(1) = 0;
total_rx(1) = 0;
start_t     = 1;

% Start air-time calculations ---------------------------------------------

for i = 2:s(1)-1
    
    this_t   = DL_temp(i, 1);
    this_t_f = floor(this_t);
    
    if (this_t_f ~= start_t)
        
        start_t = this_t_f;
        total_tx(start_t) = 0;
        total_rx(start_t) = 0;
        
    end
    
    if (this_t_f == start_t)
        
        total_tx(start_t) = total_tx(start_t) + DL_temp(i, 3);
        total_rx(start_t) = total_rx(start_t) + DL_temp(i, 4);
        DDx(start_t)      = D * start_t;
        
    end 
    
end

% Start stall calculations ------------------------------------------------

fullfill_v_tx = DDx - cumsum(total_tx);
fullfill_v_rx = DDx - cumsum(total_rx);

tt = ftime:0.001:ltime;    
Dv = D * ones(1, length(tt)) ./ 1000;

% Plot --------------------------------------------------------------------

figure;
plot(tt, cumsum(Dv), '-xb');
hold on;
plot(TT, TD, '-or');
hold on;
plot(TT, RD, '-xg');
grid on;
xlabel('Time [sec]');
ylabel('Cumulative Allocation [Bytes]');
legend('Demanded', 'Transmitted', 'Received');
title(['Cucumlative allocation for the ', sched_name, ' for UE IMSI ', num2str(k)]);

figure;
subplot(2,1,1);
bar(1:start_t, total_tx);
xlabel('Time [sec]');
ylabel('Tx Bytes');
grid on;
title('Air-time');
subplot(2,1,2);
bar(1:start_t, total_rx);
xlabel('Time [sec]');
ylabel('Rx Bytes');
grid on;
title(['Air-time for the ', sched_name]);

figure;
plot(1:start_t, fullfill_v_tx, '-o');
hold on;
plot(1:start_t, fullfill_v_rx, '-xr');
xlabel('Time [sec]');
ylabel('Stalls');
grid on;
title(['No. of video stalls for the ', sched_name, ' for UE IMSI ', num2str(k)]);

for i = 1:length(DDx)
   
    stall(i) = 0;
    if (fullfill_v_rx > 0)
    stall(i) = 1;
    end
    
end

figure;
subplot(2,1,1);
stem(1:length(DDx), stall);
xlabel('Time [sec]');
ylabel('Stall Event');
grid on;
title(['Instantaneous stall events for the ', sched_name, ' for UE IMSI ', num2str(k)]);
subplot(2,1,2);
stem(1:length(DDx), cumsum(stall));
xlabel('Time [sec]');
ylabel('Stall Event');
grid on;
title(['Cucumlative stalls for the ', sched_name, ' for UE IMSI ', num2str(k)]);

end

disp('_________________________________________________________________');
disp(' Done ..');
disp('_________________________________________________________________');