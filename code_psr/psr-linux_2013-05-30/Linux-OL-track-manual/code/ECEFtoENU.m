function [ E N U ] = ECEFtoENU(phi, lam, XYZ_coords)

ECEFtoENU_mat = [
    -sin(lam)           cos(lam)          0
    -sin(phi)*cos(lam) -sin(phi)*sin(lam) cos(phi)
     cos(phi)*cos(lam)  cos(phi)*sin(lam) sin(phi)];

ENU_coords = ECEFtoENU_mat*XYZ_coords';

E = ENU_coords(1);
N = ENU_coords(2);
U = ENU_coords(3);

return

