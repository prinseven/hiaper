% OLPredict.m
% -----
%
% Description:
%
% This is the main file for the Doppler prediction algorithm.
%
% Predicts the Doppler frequency between GPS transmitters and a receiver by
% interpolating the IGS orbits, determining the positions and velocities of
% each, and performs other necessary calculations.
%
% Data Bit support is provided by PSRBitGen.m.
%
% Inputs:
%
%     IGS orbit, various user defined parameters
%
% Outputs:
%
%     doppler freq, data saved for later processing
%
% Assumptions/References:
%
%     January 1, 2000 at 1200, JD = 2,451,545.0
%     January 1, 2000 at 1200, MJD = 51,544.5, diff = 2,400,000.5
%     January 6, 1980 at 0000, gpst = 0, MJD = 44244, serial date = 723186
%
% Dependencies:
%
%     current dir : LLAtoECEF
%     current dir : apxread
%     current dir : sp3read
%     current dir : TransTimeAdjust
%     current dir : OrbitInterp
%     current dir : ClockInterp
%     current dir : ECEFtoENU
%     current dir : DopplerPredict_Dynamic
%     current dir : skyplot
%     toolbox     : map\map\deg2rad.m
%     toolbox     : map\map\rad2deg.m
%
%     Other Files:  Applanix (apx) file (conditional), SP3 file with
%     satellite orbit data.
%
% Modification History:
%
%    Spring 2006   Brian D. Ventre   Original version
%    Summer 2008   Tyler D. Lulich   Update to moving receiver

% Clear memory, command window, close open figures, set formats for
% display, and add relevent file paths.
clear all; close all; clc
format compact; format long g;
addpath(genpath(fullfile(cd,'apx_files')));
addpath(genpath(fullfile(cd,'igs_files')));
%addpath(genpath(fullfile(cd,'nrs_files')));

% Inform the user a job has started.
disp('******************OLPredict starting a job.***********************')

% Which SV?
if (exist('svs_to_predict','var') == 0)
 
    svs_to_predict = [12];
end

% Enter the desired starting time.
% format:  gps_start_time = [GPS week, SOW];
week = 1467;
startSOW = 500400; 
%PRN 5,12: 500940 starboard
%PRN 15:  515703 starboard 
gps_start_time = [week startSOW];

% How long is our data set?
data_length_sec = 44*60;

% What's the spacing [ms]?
% Using 1 ms will write to file for use in PSR.
% Max = 1000.
% spacing_ms = 1000;
spacing_ms = 1;
% 'true' if rcvr was fixed during recording.
stationary_rcvr_flag = 0;

if stationary_rcvr_flag
        Location = 'CIVL';
        fprintf('Static Position: %s\n',Location);
else
    fprintf('Dynamic Receiver Position.\n');
end

% If above false, define the name of the Applanix file containing pos/vel
% data of the receiver.
% apx_filename = sprintf('GPSINS_apx_SFRF_mssc_diff_053.txt');
 apx_filename = sprintf('staraero_2008-053.txt');
% apx_filename = sprintf('topsen_2008-053.txt');

% 'true' if we want to plot elevation, skyplot.
skyplot_flag = 1;

% 'true' if we want to plot doppler, flight path (very high memory usage).
plot_flag = 1;

% if 'true': save important variables for later.
savefile_flag = 1;

% Define orbit product type (ultra-rapid 'igu', rapid 'igr', final 'igs').
% http://igscb.jpl.nasa.gov/components/prods_cb.html
orbit_type = 'igs';

% How many samples should we use to form the interpolation polynomial?
%  See Schenwerk's paper from gps source dot com; 10 is sufficient.
NumSamp = 10;

if stationary_rcvr_flag
    switch (Location)
        case {'CIVL'} % Updated 23.apr.09 tdl
            rcvr_lat_rad   = deg2rad(40.4304200); 
            rcvr_lon_rad  = deg2rad(-86.9148483);
            rcvr_alt_m     = 183.7;
            rcvr_vel_ms    = [0, 0, 0];
        case {'AgFarmDirect'} % Updated 14Dec06 tdl 
            rcvr_lat_rad   = deg2rad(40.47517822);
            rcvr_lon_rad  = deg2rad(273.00791630);
            rcvr_alt_m     = 212.7896;
            rcvr_vel_ms    = [0, 0, 0];
        case {'RF05'} % a general location for debugging
            rcvr_lat_rad   = deg2rad(33.1729806100783);
            rcvr_lon_rad  = deg2rad(-83.2011608389817);
            rcvr_alt_m     = 12500;
            rcvr_vel_ms    = [0, 0, 0];
        otherwise
            error('Unknown location (or unprogrammed switch...)');
    end
    [rcvr_pos_m] = LLAtoECEF([rcvr_lat_rad, rcvr_lon_rad, rcvr_alt_m]);
else
    
    % Read and parse the applanix file.
    [apx_clk rcvr_pos rcvr_vel rcvr_lat rcvr_lon rate] = apxread(startSOW, data_length_sec, apx_filename);

    if spacing_ms/1000 ~= rate
        % Straight line approximation to interpolate the apx data to the sample
        % rate.
        sample_rate = 1000/spacing_ms; % [Hz]

        fprintf('Interpolating Applanix data to %dHz...\n', sample_rate)

        rcvr_pos_m = zeros(length(apx_clk)*sample_rate,1);
        rcvr_vel_ms = zeros(length(apx_clk)*sample_rate,1);
        rcvr_lat_rad = zeros(length(apx_clk)*sample_rate,1);
        rcvr_lon_rad = zeros(length(apx_clk)*sample_rate,1);

        diff_pos = diff(rcvr_pos,1,1);
        diff_vel = diff(rcvr_vel,1,1);
        diff_lat = diff(rcvr_lat);
        diff_lon = diff(rcvr_lon);

        % HACK: Manually create an extra difference value to homogenize matrix size.
        diff_pos(end+1,:) = diff_pos(end,:)+diff_pos(end,:)-diff_pos(end-1,:);
        diff_vel(end+1,:) = diff_vel(end,:)+diff_vel(end,:)-diff_vel(end-1,:);
        diff_lat(end+1,:) = diff_lat(end)+diff_lat(end)-diff_lat(end-1);
        diff_lon(end+1,:) = diff_lon(end)+diff_lon(end)-diff_lon(end-1);

        frac_pos = diff_pos/sample_rate;
        frac_vel = diff_vel/sample_rate;
        frac_lat = diff_lat/sample_rate;
        frac_lon = diff_lon/sample_rate;

        space = 0;
        for lcv = 1:length(rcvr_pos)
            for lcv2 = 1:sample_rate
                for lcv3 = 1:3
                    rcvr_pos_m(lcv2+space,lcv3) = rcvr_pos(lcv,lcv3) + frac_pos(lcv,lcv3)*(lcv2-1);
                    rcvr_vel_ms(lcv2+space,lcv3) = rcvr_vel(lcv,lcv3) + frac_vel(lcv,lcv3)*(lcv2-1);
                    if lcv3 == 1
                        rcvr_lat_rad(lcv2+space) = rcvr_lat(lcv) + frac_lat(lcv)*(lcv2-1);
                        rcvr_lon_rad(lcv2+space) = rcvr_lon(lcv) + frac_lon(lcv)*(lcv2-1);
                    end
                end
            end
            space = space + sample_rate;
        end
    else
        rcvr_pos_m = rcvr_pos;
        rcvr_vel_ms = rcvr_vel;
        rcvr_lat_rad = rcvr_lat;
        rcvr_lon_rad = rcvr_lon;
    end
end

% Read in the appropriate SP3 file.
gps_start_wk = gps_start_time(1);
gps_start_day = floor(gps_start_time(2)/3600/24);
sp3_filename = sprintf('%s%04d%1d.sp3',orbit_type,gps_start_wk,gps_start_day);
[sv_XYZ_m, sv_clk_s, gps_wk, gps_sec, header_info] = sp3read(sp3_filename);

% Check to be sure we aren't going to run over that single sp3 file.
gps_end_time = gps_start_time+[0 data_length_sec];
if (gps_end_time(2) > 3600*24*7)
    gps_end_time(1) = gps_end_time(1) + 1;
    gps_end_time(2) = gps_end_time(2) - 3600*24*7;
    error('Ending time is into the following GPS week...');
elseif(floor(gps_end_time(2)/3600/24) ~= gps_start_day)
    error('Ending time is into the following GPS day...');
end

% Create a vector of times at some spacing (usually 1 millisecond).
gps_msow_vec = [(gps_start_time(2)*1e3):spacing_ms:(gps_end_time(2)*1e3-1)]';
gps_wk_vec = ones(size(gps_msow_vec))*gps_start_wk;
gps_sow_vec = gps_msow_vec/1000;
GPS_SOW_Range=gps_sow_vec ;
% For skyplot with stationary rcvr, fill vectors of rcvr info for math
% continuity.
if skyplot_flag && stationary_rcvr_flag
    rcvr_pos_m = repmat(rcvr_pos_m,length(gps_msow_vec), 1);
    rcvr_lat_rad = repmat(rcvr_lat_rad,length(gps_msow_vec), 1);
    rcvr_lon_rad = repmat(rcvr_lon_rad,length(gps_msow_vec), 1);
    rcvr_vel_ms = repmat(rcvr_vel_ms,length(gps_msow_vec), 1);
end
    
% Define MATLAB Serial Date offset (used for nice plots).
SerialDateGPSTstart = 723186;
DaysPerWeek = 7;
gps_start_sd = SerialDateGPSTstart + DaysPerWeek*week;
gps_sd_vec = gps_sow_vec/3600/24 + gps_start_sd;

% Parameters for nice plot axis labels.
num_divisions = 6;
AxisTime = datenum([gps_sd_vec(1) gps_sd_vec(end)]);
ticksize_date = diff(AxisTime)/num_divisions;
tickmarks = AxisTime(1):ticksize_date:AxisTime(2);

% Check to see if we're going over a specific number of satellites or all of
% the ones in the sp3 file.
if isempty(svs_to_predict)
    % All SVs in the file.
    svs_to_predict = header_info.sat_id;
end

% Should we make a skyplot?
if skyplot_flag
    lcv = 1;
    color_str = {'k','r','g','b','m','c','y'};
end

% Loop over all of the satellites in the Doppler prediction algorithm.
for lcv2 = 1:length(svs_to_predict)
    % What's the corresponding index in our sp3 file?
    svi = find(header_info.sat_id == svs_to_predict(lcv2));
    if isempty(svi)
        fprintf('SP3 file has no information for SV%02d.\n', svs_to_predict(lcv2))
        fprintf('   ...continuing to next SV\n');
        continue
    end

    % Determine the corresponding time of transmission for each of the
    % receiver positions.
    [TimeOfTransmission] = TransTimeAdjust(rcvr_pos_m, gps_msow_vec/1000, ...
        sv_XYZ_m(:,:,svi), sv_clk_s(:,:,svi), gps_sec, NumSamp);

    % Interpolate the positions and velocities using some method.
    [SV_Pos_XYZ_m, SV_Vel_ms] =  OrbitInterp(TimeOfTransmission,...
        sv_XYZ_m(:,:,svi), gps_sec, NumSamp);

    % Find the clock rate, too.
    [SV_Clk_Bias_s, SV_Clk_Rate_ss] = ClockInterp(TimeOfTransmission,...
        sv_clk_s(:,:,svi), gps_sec, NumSamp);

    % Update the position and velocity of the SV by the rotation of the
    % ECEF frame during the transit time.
    wE = 7292115.0e-11; % rad/sec
    transit_time = gps_msow_vec/1000 - TimeOfTransmission; % sec
    for lcv3 = 1:length(transit_time)
        trans_matrix = [
            cos(wE*transit_time(lcv3)),  sin(wE*transit_time(lcv3)), 0
            -sin(wE*transit_time(lcv3)), cos(wE*transit_time(lcv3)), 0
            0,                           0,                          1];
        SV_Pos_XYZ_m(lcv3,:) = (trans_matrix * SV_Pos_XYZ_m(lcv3,:)')';
        SV_Vel_ms(lcv3,:)    = (trans_matrix * SV_Vel_ms(lcv3,:)')';
    end

    % If we have a stationary receiver, then let's make a sky plot, too.
    if skyplot_flag
        % Shift the origin of the ECEF coordinates of the SV to the
        % receiver's current location.
        SV_Pos_XYZ_m_shift = SV_Pos_XYZ_m - rcvr_pos_m;

        % Convert the ECEF satellite position to the ENU frame, then
        % calculate the azimuth and elevation.
        az = zeros(size(gps_msow_vec));
        el = zeros(size(gps_msow_vec));
        for lcv4 = 1:length(az)
            [SV_Pos_E SV_Pos_N SV_Pos_U] = ...
                ECEFtoENU(rcvr_lat_rad(lcv4),rcvr_lon_rad(lcv4),SV_Pos_XYZ_m_shift(lcv4,:));

            % Calculate the elevation and azimuth of this SV and plot it.
            az(lcv4,1) = atan2(SV_Pos_E, SV_Pos_N);
            el(lcv4,1) = atan2(SV_Pos_U, sqrt(SV_Pos_E^2 + SV_Pos_N^2));
        end
        AZ_all(:,lcv) = az;
        EL_all(:,lcv) = el;
        SVs(lcv) = svs_to_predict(lcv2);
        lcv = lcv + 1;

        % Give us a globe view, too.
        figure(1); hold on;
        color_ind = mod(lcv-1,length(color_str))+1;
        plot3(SV_Pos_XYZ_m(:,1),SV_Pos_XYZ_m(:,2),SV_Pos_XYZ_m(:,3),color_str{color_ind})
        plot3(SV_Pos_XYZ_m(end,1),SV_Pos_XYZ_m(end,2),SV_Pos_XYZ_m(end,3),[color_str{color_ind},'*'],'MarkerSize',5)

        figure(2); hold on; grid on;
        plot(gps_sd_vec, rad2deg(el),color_str{color_ind})
        legend_str{lcv2} = sprintf('SV%02d',svs_to_predict(lcv2));
    end

    % Do Doppler/Phase prediction.
    fprintf('Predicting Doppler for SV%02d...\n',svs_to_predict(lcv2))
    [DopplerFreq CarrierPhase Range RangeDopp Wavelength] = ...
        DopplerPredict_Dynamic(gps_wk_vec, gps_msow_vec, rcvr_pos_m, rcvr_vel_ms, ...
        SV_Pos_XYZ_m, SV_Vel_ms, SV_Clk_Rate_ss, spacing_ms, []);

    % How do the two Doppler Freqs compare?
    dopp_diff = DopplerFreq(2:end) - RangeDopp;
    fprintf('SV%02d: Average Doppler Difference %11.7f Hz\n',svs_to_predict(lcv2),mean(dopp_diff))
    if plot_flag
        figure; hold on; grid on; box on;
        plot(gps_sd_vec,DopplerFreq,'.b',gps_sd_vec(1:end-1),RangeDopp,'r')
        v = axis; axis([AxisTime,v(3:4)])
        xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
        ylabel('Doppler Frequency [Hz]')
        set(gca,'xtick',tickmarks)
        datetick('x','HH:MM:SS','keeplimits','keepticks')
        date = datestr(gps_sd_vec(1)); DMY = date(1:11);
        title(sprintf('SV%02d - Doppler Frequency',svs_to_predict(lcv2)))
        legend('VLOS Doppler','Range-derived Doppler','Location','Best');
        print('-dpdf','-r300',sprintf('../Plots/SV%02d-%s-PredictedDopp',svs_to_predict(lcv2),DMY))

        if spacing_ms ~= 1
            figure; hold on; box on;
            plot3(rcvr_pos_m(:,1),rcvr_pos_m(:,2),rcvr_pos_m(:,3))
            plot3(rcvr_pos_m(end,1),rcvr_pos_m(end,2),rcvr_pos_m(end,3),'r*','MarkerSize',5)
            xlabel('Feet')
            ylabel('Feet')
            zlabel('Feet')
            title('Actual Flight Path, ECEF XYZ')
        end
    end

    % Output the values to our prediction file.  Write a header for every
    % second, followed by 1000 values of Doppler (1 for each millisecond).
    % Binary output
    if (spacing_ms == 1)
        fprintf('Writing SV%02d Doppler prediction to binary file...\n',svs_to_predict(lcv2))
        outfilestr = sprintf('../Output/OLPredictSV%02d.in', svs_to_predict(lcv2));
        output_file = fopen(outfilestr, 'w+b');
        if (output_file == -1)
            error('Unable to open %s!', outfilestr)
        end
        for lcv5 = 1:data_length_sec
            start_ind = (lcv5-1)*1000+1;
            stop_ind = (lcv5)*1000;

            if (floor(gps_msow_vec(start_ind)/1000) ~= gps_msow_vec(start_ind)/1000)
                error('Oops');
            end
            fwrite(output_file, gps_wk_vec(start_ind), 'ushort');
            fwrite(output_file, gps_msow_vec(start_ind)/1000, 'ulong');
            fwrite(output_file, DopplerFreq(start_ind:stop_ind), 'double');
        end
        fclose(output_file);
    end

    % Create a matrix containing the phase predictions for each SV.
    CarrierPhase_mtrx(:,lcv2) = CarrierPhase;

    % Save only the necessary variables for later comparison.
    if savefile_flag
        save(sprintf('../Output/OLPredict_SV%02d',svs_to_predict(lcv2)),...
            'sp3_filename','svs_to_predict','CarrierPhase','Range','gps_sow_vec','GPS_SOW_Range')
    end

    fprintf('Done with SV%02d...\n',svs_to_predict(lcv2));
end

% Print the skyplot.
if skyplot_flag
    % Finish the 3-D globe view first.
    figure(1)
    [x y z] = ellipsoid(0,0,0,6378137,6378137,6356752.3142,24);
    surf(x,y,z)
    plot3(rcvr_pos_m(:,1),rcvr_pos_m(:,2),rcvr_pos_m(:,3))
    axis equal; colormap pink; grid on
    view(rcvr_pos_m(1,:))

    % Then, the elevation profiles
    figure(2)
    v = axis; axis([AxisTime,v(3:4)])
    xlabel([datestr(gps_sd_vec(1)) '    |-------- GPST --------|    ' datestr(gps_sd_vec(end))])
    ylabel('Elevation [deg]')
    set(gca,'xtick',tickmarks)
    datetick('x','HH:MM:SS','keeplimits','keepticks')
    date = datestr(gps_sd_vec(1)); DMY = date(1:11);
    title([sprintf('Elevation, %s, %d seconds',DMY,data_length_sec)])
    legend(legend_str,'Location','Best');
    print('-dpdf','-r300',sprintf('../Plots/elevation-profile-%s-SOW%d',DMY,startSOW))

    % Print the sky plot.
    skyplot(AZ_all,EL_all,SVs)
    title([sprintf('Skyplot, %s, %d seconds',DMY,data_length_sec)])
    print('-dpdf','-r300',sprintf('../Plots/skyplot-%s-SOW%06d',DMY,startSOW))
end

disp('******************OLPredict finished a job.******************')
