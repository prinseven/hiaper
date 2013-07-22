clear
clc
close all

points = 128;
magnitude = round(points/(2*pi));
magnitude = 32;
%function [dummy] = gen_sine(points)



%generate data
vect = linspace(0,2*pi,points+1);

num = sin(vect)*magnitude;
num = round(num);

num2 = cos(vect)*magnitude;
num2 = round(num2);


% figure(1)
% plot(num)
% axis equal
% hold on; grid on
% plot(num2,'r')




fp = fopen('channel_sine.h','w');
fprintf(fp,'DATA_TYPE Sinewave[%d] = {\n',points);


for lcv = 1:length(num)-1
    fprintf(fp,'%d,\n',num(lcv));
end

fprintf(fp,'};');

fprintf(fp,'\n\nDATA_TYPE nSinewave[%d] = {\n',points);


for lcv = 1:length(num)-1
    fprintf(fp,'%d,\n',-num(lcv));
end

fprintf(fp,'};');


fprintf(fp,'\n\nDATA_TYPE Coswave[%d] = {\n',points);


for lcv = 1:length(num2)-1
    fprintf(fp,'%d,\n',num2(lcv));
end

fprintf(fp,'};');

fclose(fp);



fp = fopen('Complex_Sine.h','w')

fprintf(fp,'short Sine_Table[%d*4] = {\n',points);

for lcv = 1:length(num2)-1
    fprintf(fp,'%d,\n',num2(lcv));
    fprintf(fp,'%d,\n',num(lcv));
    fprintf(fp,'%d,\n',-num(lcv));
    fprintf(fp,'%d,\n',num2(lcv));    
end


fprintf(fp,'};');
fclose(fp);