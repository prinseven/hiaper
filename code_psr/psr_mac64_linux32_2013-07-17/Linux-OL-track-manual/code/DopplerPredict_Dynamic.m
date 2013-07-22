function [DopplerFreq CarrierPhase Range RangeDopp wavelength] = ...
    DopplerPredict_Dynamic(GPS_Wk, GPS_MSOW, Rcvr_Pos, Rcvr_Vel, Trans_Pos, ...
    Trans_Vel, Trans_Clk_Rate, spacing_ms, options)

% This function will return an Nx1 vector of predicted Doppler frequencies,
% where N indicates the total number of data points.
%
% Inputs:
%  Time (referenced exclusively to GPS time):
%   GPS_Wk   : Nx1 vector of GPS week
%   GPS_MSOW : Nx1 vector of GPS millisecond of week
%  Receiver information (drawn from GPS/INS results):
%   Rcvr_Pos : Nx3 array of receiver positions in WGS-84 ECEF XYZ coordinates
%   Rcvr_Vel : Nx3 array of receiver velocities in WGS-84 ECEF XYZ coordinates
%  Transmitter (GPS satellite) information (drawn from IGS orbit data):
%   Trans_Pos : Nx3 array of transmitter positions in WGS-84 ECEF XYZ coordinates
%   Trans_Vel : Nx3 array of transmitter velocities in WGS-84 ECEF XYZ coordinates
%  Transmitter clock information (drawn from IGS orbit data):
%   Trans_Clk_Rate : Nx1 array of transmitter clock rate in seconds/second
%  Options:
%   options : Structure of options (format to be determined)
%
% Outputs:
%  DopplerFreq : Nx1 vector of predicted Doppler frequencies, based on
%  geometric and climatological models.
%
% Brian Ventre, Spring 2006

% Check to see if we have a stationary receiver.  If so, fill out Rcvr_Pos
% and Rcvr_Vel with copies of themselves in order to maintain the matrix
% math below.
if (size(Rcvr_Pos,1) ~= size(Trans_Pos,1))
    if (size(Rcvr_Pos,1) == 1 && size(Rcvr_Vel,1) == 1)
        % Need to stretch Rcvr_Pos and Rcvr_Vel.
        Rcvr_Pos = repmat(Rcvr_Pos,size(Trans_Pos,1),1);
        Rcvr_Vel = repmat(Rcvr_Vel,size(Trans_Pos,1),1);
    else
        error('Transmitter and receiver matrices are different sizes');
    end
end

% Constants
c = 2.99792458e8; % m/s
freqL1 = 1575.42e6; % Hz
freqL2 = 1227.60e6; % Hz

% Wavelength is a function of the clock rate (because frequency is affected
% by the transmitting clock's rate).
wavelength = c ./ (freqL1 * (1 + Trans_Clk_Rate)); % units = (m/s)/(cyc/s+cyc/s*s/s) = m/cyc

% Position vector from receiver to transmitter.
R_rt_m = 0*Trans_Pos;
r_rt_m = 0*Trans_Pos;
for ii = 1:size(Rcvr_Pos,2)
    R_rt_m(:,ii) = Trans_Pos(:,ii) - Rcvr_Pos(:,ii);
end

% Range, distance between receiver and transmitter.
mag_R_rt_m = sqrt(dot(R_rt_m,R_rt_m,2));
% Unit vector pointing from receiver to transmitter.
for ii = 1:size(Rcvr_Pos,2)
    r_rt_m(:,ii) = R_rt_m(:,ii) ./ mag_R_rt_m;
end

% Dot product of each velocity with the vector from receiver to transmitter
% (line of sight velocity)
Vlos_rcvr_ms = dot(r_rt_m,Rcvr_Vel,2);
Vlos_trans_ms = dot(r_rt_m,Trans_Vel,2);

% Total line of sight velocity along vector from receiver to transmitter
Vlos_total_ms = Vlos_trans_ms - Vlos_rcvr_ms;

% Convert from line of sight velocity to Doppler frequency.
DopplerFreq = -Vlos_total_ms ./ wavelength;

% Calculate carrier phase via division of the magnitude of the position
% vector from receiver to transmitter by wavelength.
CarrierPhase = mag_R_rt_m ./ wavelength; % units = m/(m/cyc) = cyc

% Range and range derived Doppler frequency.
Range = mag_R_rt_m;
diffRange = diff(Range);
derivRange = diffRange/(spacing_ms/1000);
RangeDopp = -derivRange ./ wavelength(2:end);

