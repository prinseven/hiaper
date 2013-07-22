function [LLA] = ECEFtoLLA(XYZ_m)
% function [LLA] = ECEFtoLLA(XYZ_m)
%
% Takes ECEF coordinates X, Y, and Z (all in meters) and converts to WGS-84
% latitude, longitude and height (rad, rad, m).
%
% Inputs:
%  XYZ_m : Nx3 vector of X, Y, Z positions in ECEF (meters)
%
% Outputs:
%  LLA : Nx3 vector of latitude (phi, in rad), longitude (lam, in rad), and
%  altitude (in m)
%
% Brian Ventre, Spring 2006

% Extract inputs.
X_m = XYZ_m(:,1);
Y_m = XYZ_m(:,2);
Z_m = XYZ_m(:,3);

% WGS-84 constants.
a = 6378137;
b = 6356752.3142;
e1 = (a^2 - b^2) / a^2;
e2 = (a^2 - b^2) / b^2;

% Calculate the radius of parallel.
P = sqrt(X_m.^2 + Y_m.^2);

% Calculate the intermediate quantity.
theta = atan(Z_m*a./P./b);

% Calculate the geodetic coordinates.
phi_rad = atan((Z_m + e2*b*(sin(theta)).^3) ./ (P - e1*a*(cos(theta)).^3));
lam_rad = atan2(Y_m,X_m);

% Calculate the radius of curvature.
N = a^2 ./ sqrt(a^2*(cos(phi_rad)).^2 + b^2*(sin(phi_rad)).^2);

% Calculate the ellipsoidal height.
h_m = P ./ cos(phi_rad) - N;

LLA = [phi_rad,lam_rad,h_m];
