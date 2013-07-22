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




fp = fopen('Real_Sine.h','w')

fprintf(fp,'short Sine_Table[%d*4] = {\n',points);

for lcv = 1:length(num2)-1
    fprintf(fp,'%d,\n',num(lcv));
    fprintf(fp,'%d,\n',0);
    fprintf(fp,'%d,\n',num2(lcv));
    fprintf(fp,'%d,\n',0);    
end


fprintf(fp,'};');
fclose(fp);