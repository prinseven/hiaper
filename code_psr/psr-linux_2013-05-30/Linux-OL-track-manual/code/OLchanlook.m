% if (exist('do_not_clear_olchanlook','var') == 1)
%     close all
%     clear all
%     clc
% end

close all
clear all
clc

%what type of occultation(setting or rising)?
forward = input('type of tracking(0 for backward)? ');
% Load the channel and PRN numbers.
sv_chans = load('../Output/sv_chans.dat');
fprintf('Available PRNs: ');
fprintf('%02d ',sv_chans(:,2));
fprintf('\n');

% Which PRN?
if (exist('PRN','var') == 0)
    PRN = 3;
end

% Create a filename for each of the plots.
filename_olchan = sprintf('../Plots/PRN%02dOLchan',PRN);

% Create an PRN specific handle each figure title.
figure_title = sprintf('PRN %02d',PRN);

% How many samples per msec?
NumSamplesPerMs = 10000;

% Find the equivalent closed-loop channel.
index = find(sv_chans(:,2) == PRN);
CLChannel = sv_chans(index,1)

% Let's see if we have the .mat file available.
matfile = 0;
fid = fopen(sprintf('../Output/OLchanlookSV%02d.mat',PRN));
if (fid ~= -1)
    matfile = input('Use .mat file (0 for no)? ');
    if (matfile(1) ~= 0)
        fclose(fid);
        disp('Using presaved .mat file...');
        load(sprintf('../Output/OLchanlookSV%02d.mat',PRN));
    end
end

if (matfile==0)
    if (forward~=0)
    
        % Extract the Open Loop (OL) data.
        A = load(sprintf('../Output/OLPredictSV%02d.out',PRN));
        len = floor((length(A)-40)/20)*20;
        ms=A(:,2);
        start_ms=ms(1);
        start_ind=mod(start_ms,20);
        start_ind=20-start_ind;
        % Index values
        Wki              = 1;
        MSOWi            = 2;
        HeaderMSOWi      = 3;
        Freqi            = 4;
        Ii               = 5;
        Qi               = 6;
        PhiRi            = 7;
        PhiNCOi          = 8;
        DataBiti         = 9;
        SamplesPerCyclei = 10;
        EdgeIndexi       = 11;
    
        % Extract individual data vectors
        GPS_Wk          = A(start_ind+12:len+start_ind+11,Wki);
        GPS_MSOW        = A(start_ind+12:len+start_ind+11,MSOWi);
        Header_MSOW     = A(start_ind+12:len+start_ind+11,HeaderMSOWi);
        Freq            = A(start_ind+12:len+start_ind+11,Freqi);
        I               = A(start_ind+12:len+start_ind+11,Ii);
        Q               = A(start_ind+12:len+start_ind+11,Qi);
        PhiResidual     = A(start_ind+12:len+start_ind+11,PhiRi);
        PhiNCO          = A(start_ind+12:len+start_ind+11,PhiNCOi);
        DataBit         = A(start_ind+12:len+start_ind+11,DataBiti);
        SamplesPerCycle = A(start_ind+12:len+start_ind+11,SamplesPerCyclei);
        EdgeIndex       = A(start_ind+12:len+start_ind+11,EdgeIndexi);
        clear A
        save(sprintf('../Output/OLPredictSV%02d.mat',PRN),'GPS_Wk','GPS_MSOW',...
            'Header_MSOW','Freq','I','Q','PhiResidual','PhiNCO','DataBit',...
            'SamplesPerCycle','EdgeIndex')
    
        %  For Carrier Phase In Chanxx.dat
        %%%%##############################################
        %Extract closed loop (CL) data.
        B = load(sprintf('../Output/chan%02d.dat',CLChannel));
        lenCL = floor(length(B)/20)*20;
        starti = find(GPS_MSOW(1) == B(:,14));
        endi = find(GPS_MSOW== B(end,14));
    
        DopplerCL       = B(starti:endi,1);
        CarrierPhaseCL  = B(starti:endi,3);
        ICL             = B(starti:endi,6);
        QCL             = B(starti:endi,7);
        EdgeIndexCL     = B(starti:endi,12);
        GPS_MSOWCL      = B(starti:endi,14);
  
%       GPS_MSOW=GPS_MSOW-440326;
%       GPS_MSOWCL=GPS_MSOWCL-440326;
    %   For Carrier Phase In Chanxx.dat
    %%%%##############################################
        clear B
        % break
        % Create the CL complex correlator.
    else
    
        % Extract the Open Loop (OL) data.
        A = load(sprintf('../Output/OLPredictSV%02d.out',PRN));
        len = floor((length(A)-40)/20)*20;
        ms=A(:,2);
        start_ms=ms(end);
        start_ind=mod(start_ms,20);
        start_ind=20-start_ind;
        % Index values
        Wki              = 1;
        MSOWi            = 2;
        HeaderMSOWi      = 3;
        Freqi            = 4;
        Ii               = 5;
        Qi               = 6;
        PhiRi            = 7;
        PhiNCOi          = 8;
        DataBiti         = 9;
        SamplesPerCyclei = 10;
        EdgeIndexi       = 11;
    
        % Extract individual data vectors
        GPS_Wk          = flipud(A(:,Wki));
        GPS_Wk          = GPS_Wk(start_ind+12:len+start_ind+11,1);
        GPS_MSOW        = flipud(A(:,MSOWi));
        GPS_MSOW        = GPS_MSOW(start_ind+12:len+start_ind+11,1);
        Header_MSOW     = flipud(A(:,HeaderMSOWi));
        Header_MSOW     = Header_MSOW(start_ind+12:len+start_ind+11,1);
        Freq            = flipud(A(:,Freqi));
        Freq            = Freq(start_ind+12:len+start_ind+11,1);
        I               = flipud(A(:,Ii));
        I               = I(start_ind+12:len+start_ind+11,1);
        Q               = flipud( A(:,Qi));
        Q               = Q(start_ind+12:len+start_ind+11,1);
        PhiResidual     = flipud(A(:,PhiRi));
        PhiResidual     = PhiResidual(start_ind+12:len+start_ind+11,1);
        PhiNCO          = flipud(A(:,PhiNCOi));
        PhiNCO          = PhiNCO(start_ind+12:len+start_ind+11,1);
        DataBit         = flipud(A(:,DataBiti));
        DataBit         = DataBit(start_ind+12:len+start_ind+11,1);
        SamplesPerCycle = flipud(A(:,SamplesPerCyclei));
        SamplesPerCycle = SamplesPerCycle(start_ind+12:len+start_ind+11,1);
        EdgeIndex       = flipud(A(:,EdgeIndexi));
        EdgeIndex       = EdgeIndex(start_ind+12:len+start_ind+11,1);
        clear A
        save(sprintf('../Output/OLPredictSV%02d.mat',PRN),'GPS_Wk','GPS_MSOW',...
            'Header_MSOW','Freq','I','Q','PhiResidual','PhiNCO','DataBit',...
            'SamplesPerCycle','EdgeIndex')
    
        %  For Carrier Phase In Chanxx.dat
        %%%%##############################################
        %Extract closed loop (CL) data.
        B = load(sprintf('../Output/chan%02d.dat',CLChannel));
        lenCL = floor(length(B)/20)*20;
        CL_msow=B(:,14);
%         starti = find(GPS_MSOW(1) == B(:,14));
        endi = find(GPS_MSOW(end)== B(:,14));
    
        DopplerCL       = B(lenCL-endi:endi,1);
        CarrierPhaseCL  = B(lenCL-endi:endi,3);
        ICL             = B(lenCL-endi:endi,6);
        QCL             = B(lenCL-endi:endi,7);
        EdgeIndexCL     = B(lenCL-endi:endi,12);
        GPS_MSOWCL      = B(lenCL-endi:endi,14);
  
   %     GPS_MSOW=GPS_MSOW-440326;
%     GPS_MSOWCL=GPS_MSOWCL-440326;
    %  For Carrier Phase In Chanxx.dat
    %%%%##############################################
        clear B
    end
    CorrCL = ICL + QCL*1i;
    CorrCLAmp = abs(CorrCL);
    CorrCLPhase = angle(CorrCL);
    
    % Create the complex correlator along with phase and amplitude.
    Corr = I + Q*1i;
    CorrAmp = abs(Corr);
    CorrPhase = angle(Corr);
    
    % Calculate the total phase (from residual and NCO).
    TotalPhase = PhiResidual + PhiNCO;
    
    % Use the data bit to "fix" the correlators.  Use the unwrap
    % function to try to fix the discontinuities
    CorrCorrect = Corr./(DataBit);
    CorrCorrectPhase = unwrap(angle(CorrCorrect));%unwrap(angle(CorrCorrect));
    
    %CorrCorrectCL = CorrCL./DataBit;
    %CorrCorrectCLPhase = unwrap(angle(CorrCorrectCL));
    % Check CorrCorrectPhase and the OL residual phase.  If they don't
    % match, we have a problem (possibly).  Note that we drop the first
    % sample on the residual phase because of the n+1 delay in the
    % processing.
    PhiResidual=PhiResidual-PhiResidual(2);
    MaxAbsDeviation = max(abs(CorrCorrectPhase(1:end-1)-PhiResidual(2:end)));
    MaxDeviationOK = MaxAbsDeviation <= 1e-2;
    
    % Reduce data rate by summing or average every 20 data points together
    % (coherent over a data bit because we start on a data bit edge).
    % We sum over the OL, CL, and corrected OL correlators.
    len_r           = floor(length(Corr)/20);
    CorrR           = zeros(len_r,1);
    CorrCorrectR    = zeros(len_r,1);
    CorrCLR         = zeros(len_r,1);
    GPS_MSOWR       = zeros(len_r,1);
    for ii = 1:len_r
        start = (ii-1)*20 + 1;
        stop  = (ii-1)*20 + 20;
        CorrR(ii)           = sum(Corr(start:stop));
        CorrCorrectR(ii)    = sum(CorrCorrect(start:stop));
%         CorrCLR(ii)         = sum(CorrCL(start:stop));
        GPS_MSOWR(ii)       = mean(GPS_MSOW(start:stop));
        GPS_MSOWR(ii)       =GPS_MSOWR(ii)-mod(GPS_MSOWR(ii),20);
    end
    
    % Extract properties of each correlator.
    CorrAmpR = abs(CorrR);
    CorrPhaseR = angle(CorrR);
    
    CorrCorrectAmpR = abs(CorrCorrectR);
    CorrCorrectPhaseR = unwrap(angle(CorrCorrectR));
    CorrCLAmpR = abs(CorrCLR);
    CorrCLPhaseR = angle(CorrCLR);
end


%figure(10)
%subplot(3,1,1);plot(GPS_MSOW(1:end),unwrap(CorrCLPhase(1:end)-CorrCLPhase(1))/2/pi,'.')
%subplot(3,1,2);plot(GPS_MSOW(1:end),unwrap(CorrPhase(1:end)-CorrPhase(1))/2/pi,'b.',GPS_MSOW(1:end),unwrap(CorrPhase(1:end)-CorrPhase(1))/2/pi,'r.')
%subplot(3,1,3);plot(GPS_MSOW(1:end),CorrCorrectPhase/2/pi,'.')

%plot(GPS_MSOW(1:200:end),(CorrCLPhase(1:200:end)-CorrCLPhase(1))/2/pi,'.')

%++++++++++++++++++++++++++++++++++++++++++++++++++
% Nice Plots

% Define MATLAB Serial Date offset (used for nice plots).
SerialDateGPSTstart = 723186;
DaysPerWeek = 7;
gps_start_sd = SerialDateGPSTstart + DaysPerWeek*GPS_Wk(1);

% Create a vector of times at some spacing (usually 1 millisecond).
gps_sow_vec = GPS_MSOW/1000;
gps_sd_vec = gps_sow_vec/3600/24 + gps_start_sd;
% For reduced rate.
gps_sow_vecR = GPS_MSOWR/1000;
gps_sd_vecR = gps_sow_vecR/3600/24 + gps_start_sd;

% Parameters for nice plot axis labels.
num_divisions = 6;
AxisTime = datenum([gps_sd_vec(1) gps_sd_vec(end)]);
ticksize_date = diff(AxisTime)/num_divisions;
tickmarks = AxisTime(1):ticksize_date:AxisTime(2);

AxisTimeCL = datenum([gps_sd_vec(1) gps_sd_vec(length(GPS_MSOW))]);
ticksize_dateCL = diff(AxisTimeCL)/8;
tickmarksCL = AxisTimeCL(1):ticksize_dateCL:AxisTimeCL(2);
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
%++++++++++++++++++++++++++++++++++++++++++++++++++

% Save the variables so we won't have to do this again!
save(sprintf('../Output/OLchanlookSV%02d.mat',PRN))
savefile = sprintf('../Output/OLChanlookSV%02d_%s.mat',PRN,DMY);
save(savefile,'DopplerCL','Freq','PhiResidual','GPS_MSOW','PRN','GPS_Wk',...
    'gps_sd_vec','gps_sd_vecR','AxisTime','ticksize_date','tickmarks',...
    'AxisTimeCL','ticksize_dateCL','tickmarksCL')

% Check if post-processed phase matches the OL phase calculated using the
% 4-quadrant phase extraction in the PSR.  Inform user of result, print
% figure if necessary.
%-------------------------------------------
if (MaxDeviationOK)
    disp('Post-processed phase matches the OL calculated phase');
else
    % Hmm, things don't seem to match, possible error somewhere?
       disp('Post-processed phase does not match the OL calculated phase!');
       figure; hold on; grid on;
       plot(gps_sd_vec(1:end-1),CorrCorrectPhase(1:end-1)/2/pi,'r.','MarkerSize',2);
       plot(gps_sd_vec(1:end-1),PhiResidual(2:end)/2/pi,'k.','MarkerSize',2);
       v = axis; axis([AxisTime,v(3:4)])
       xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
       ylabel('Phase (cycles)')
       set(gca,'xtick',tickmarks)
       datetick('x','HH:MM:SS','keeplimits','keepticks')
       title(sprintf('PRN%02d - Post-processed phase and OL phase - %s', PRN, DMY))
       legend('Post-processed','Residual with 1^{st} sample dropped','Location','Best')
       print('-dpdf','-r300',[filename_olchan,sprintf('_%s_phase_error',DMY)])
end
%-------------------------------------------

% Phase from Correlators, wrapped and unwrapped, 1KHz.
%-------------------------------------------
figure

subplot(3,1,1); hold on; grid on;
plot(gps_sd_vec,(CorrPhase)/2/pi,'.','MarkerSize',2);
v = axis; axis([AxisTime,v(3:4)])
ylabel('Phase (cycles)')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Uncorrected correlator phase (data bits present) (',figure_title,')'])

subplot(3,1,2); hold on; grid on;
plot(gps_sd_vec,angle(CorrCorrect)/2/pi,'.','MarkerSize',2);
v = axis; axis([AxisTime,v(3:4)])
ylabel('Phase (cycles)')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Corrected correlator phase (data bits present) (',figure_title,')'])
%xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])


subplot(3,1,3); hold on; grid on;
plot(gps_sd_vec,CorrCorrectPhase/2/pi,'.','MarkerSize',2);
v = axis; axis([AxisTime,v(3:4)])
ylabel('Phase (cycles)')
xlabel('GPS Time    [hh:mm:ss]')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Residual Phase (data bits removed and unwrapped) (',figure_title,')'])
print('-dpdf','-r300',[filename_olchan,'_phase_1kHz'])

%-------------------------------------------

% Plot Amplitude, 1kHz
%-------------------------------------------
% figure; hold on; grid on;
% subplot(2,1,1); hold on; grid on;
% plot(gps_sd_vec,CorrPhase/2/pi,'.','MarkerSize',2);
% v = axis; axis([AxisTime,v(3:4)])
% ylabel('Phase (cycles)')
% set(gca,'xtick',tickmarks)
% datetick('x','HH:MM:SS','keeplimits','keepticks')
% date = datestr(gps_sd_vec(1)); DMY = date(1:11);
% 
% 
% subplot(2,1,2); hold on; grid on;
% plot(gps_sd_vec,CorrAmp,'.','MarkerSize',2);
% v = axis; axis([AxisTime,v(3:4)])
% ylabel('Amplitude')
% set(gca,'xtick',tickmarks)
% datetick('x','HH:MM:SS','keeplimits','keepticks')
% title(['Amplitude, 1kHz data rate (', figure_title,')'])
% xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
% print('-dpdf','-r300',[filename_olchan,'_amp_1kHz'])
%-------------------------------------------

%-------------------------------------------
figure; grid on;
if(forward)
    plot(gps_sd_vec(1:length(GPS_MSOWCL)),DopplerCL,'b.','MarkerSize',2);
else
    plot(gps_sd_vec(end-length(GPS_MSOWCL)+1:end),DopplerCL,'b.','MarkerSize',2);
end
hold on;
plot(gps_sd_vec,Freq,'r.','MarkerSize',2);
%plot(gps_sd_vec(1:length(GPS_MSOWCL)),New_Dopp,'w.','MarkerSize',1);

v = axis; axis([AxisTime,v(3:4)])
%xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
xlabel('GPS Time    [hh:mm:ss]')
ylabel('Doppler frequency (Hz)')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Doppler Frequency (',figure_title,')'])
legend('CL','Model')
%legend('CL','Predicted')
print('-dpdf','-r300',[sprintf(filename_olchan,'_doppler_model&CL')])
%-------------------------------------------

%-------------------------------------------
%TDL 19 SEP 06
if(forward)
    Doppler_diff = DopplerCL - Freq(1:length(GPS_MSOWCL));
else
    Doppler_diff = DopplerCL - Freq(end-length(GPS_MSOWCL)+1:end);
end
ave = mean(Doppler_diff);
figure; hold on; grid on;
plot(gps_sd_vec(1:length(GPS_MSOWCL)),Doppler_diff,'k.','MarkerSize',2);
v = axis; axis([AxisTimeCL,v(3:4)])
xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
ylabel('Doppler frequency (Hz)')
set(gca,'xtick',tickmarksCL)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['CL minus Model Doppler (',figure_title,')',sprintf('  [Average:  %1.4f Hz]',ave)])
print('-dpdf','-r300',[filename_olchan,'_doppler_diff'])
%-------------------------------------------


%-------------------------------------------
figure; hold on; grid on;
plot(gps_sd_vec,unwrap(PhiResidual/2/pi),'k.','MarkerSize',2);
v = axis; axis([AxisTime,v(3:4)])
xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
ylabel('Phase (cycles)')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Phase from OL Channel (1kHz data rate) (',figure_title,')'])
print('-dpdf','-r300',[filename_olchan,'_residual_phase_from_PSR'])
%-------------------------------------------

%-------------------------------------------
TotalDiffCodeCycle = sum(abs(mod(abs(EdgeIndex(1:length(GPS_MSOWCL))-EdgeIndexCL),NumSamplesPerMs)));
MaxDiffCodeCycle = 2;%max(abs(mod(abs(EdgeIndex(1:length(GPS_MSOWCL))-EdgeIndexCL),NumSamplesPerMs)));
if (TotalDiffCodeCycle > len*0.1 || MaxDiffCodeCycle > 1)
    disp('OL and CL code cycle edges appear to have a problem!')
    figure
    
    subplot(3,1,1); hold on; grid on;
    plot(gps_sd_vec(1:length(GPS_MSOWCL)),EdgeIndex(1:length(GPS_MSOWCL)),'.','MarkerSize',4)
    v = axis; axis([AxisTimeCL,v(3:4)])
    ylabel('OL Edge')
    set(gca,'xtick',tickmarksCL)
    datetick('x','HH:MM:SS','keeplimits','keepticks')
    date = datestr(gps_sd_vec(1)); DMY = date(1:11);
    title(['Difference in code cycle edges (',figure_title,')'])
    
    subplot(3,1,2); hold on; grid on;
    plot(gps_sd_vec(1:length(GPS_MSOWCL)),EdgeIndexCL,'.','MarkerSize',4)
    v = axis; axis([AxisTimeCL,v(3:4)])
    ylabel('CL Edge')
    set(gca,'xtick',tickmarksCL)
    datetick('x','HH:MM:SS','keeplimits','keepticks')
    date = datestr(gps_sd_vec(1)); DMY = date(1:11);
    
    subplot(3,1,3); hold on; grid on;
    plot(gps_sd_vec(1:length(GPS_MSOWCL)),EdgeIndex(1:length(GPS_MSOWCL))-EdgeIndexCL,'b.','MarkerSize',4);
    v = axis; axis([AxisTimeCL,v(3:4)])
    ylabel('Sample difference')
    set(gca,'xtick',tickmarksCL)
    datetick('x','HH:MM:SS','keeplimits','keepticks')
    date = datestr(gps_sd_vec(1)); DMY = date(1:11);
    
    
    xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(length(GPS_MSOWCL))])
    print('-dpdf','-r300',[filename_olchan,'_code_edge'])
else
    disp('OL and CL code cycle edges within parameters!')
end
%-------------------------------------------

% phase, 50 Hz data rate
%-------------------------------------------
figure; hold on; grid on;
plot(gps_sd_vecR,CorrCorrectPhaseR/2/pi,'k.','MarkerSize',2);
v = axis; axis([AxisTime,v(3:4)])
ylabel('Phase (cycles)')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Corrected Correlator Plot (data bits removed and unwrapped) 50Hz data rate (',figure_title,')'])
print('-dpdf','-r300',[filename_olchan,'_phase_corr'])
%-------------------------------------------

% amplitude, 50 Hz data rate
%-------------------------------------------
figure; hold on; grid on;
plot(gps_sd_vecR,CorrCorrectAmpR,'k.','MarkerSize',2);
v = axis; axis([AxisTime,v(3:4)])
ylabel('Amplitude')
set(gca,'xtick',tickmarks)
datetick('x','HH:MM:SS','keeplimits','keepticks')
date = datestr(gps_sd_vec(1)); DMY = date(1:11);
title(['Amplitude - 50Hz data rate (',figure_title,')'])
xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
print('-dpdf','-r300',[filename_olchan,'_raw_corr_50Hz'])
%-------------------------------------------

fprintf('...done PRN%02d\n',PRN)

