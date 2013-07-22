% fast_compare.m

% used to quickly compare doppler freq from OLPredict.m, PSR CL/OL, and NetRS

close all; clear all; clc; format compact; format long g;

setting = input('Type of occultation(0 for rising)?');

PRN_vec = [3];

%rinex_file = sprintf('pu07275-15-19.10o'); %Ferry flight-2010-FF04
rinex_file = sprintf('PU12200802_53.08o');
[Obs epochs sv_vec apcoords] = readrinex(rinex_file);

%some constants
fl1=1575.42e6;  %L1 frequency
fl2=1227.60e6;  %L2 frequency 
c = 2.99792458e8;   %speed of light
lam1=c/fl1; %L1 wavelength
lam2=c/fl2; %L2 wavelength

for lcv = 1:length(PRN_vec)
    
    SV = PRN_vec(lcv);
    
    load(sprintf('../Output/OLPredict_SV%02d.mat',SV));
    load(sprintf('../Output/OLChanlookSV%02d.mat',SV));
    
    OL_sow_vec = GPS_MSOW/1000;     %OL time vector
    CL_sow_vec = GPS_MSOWCL/1000;   %CL time vector
    GPS_sow_range=GPS_SOW_Range;    %Range time vector
    
    nrs_freq_hz = 5;
    OL_freq_hz = 1000;
    CL_freq_hz = 1000;
    
    rinexSVi =  find(sv_vec == SV);
    
    epochs(:,1) = str2double(rinex_file(5:8));
    nrs_sd_vec = datenum(epochs);
    NRS = rinex_file(1:4);
    
    % Parse the orbit type, GPS_week, and Day of Week from .sp3 file name.
    file_type = (sp3_filename(1:3));
    gps_week = str2double(sp3_filename(4:7));
    gps_dow = str2double(sp3_filename(8:9));
    
    % Convert the rinex epochs to second of the week (SOW).
    nrs_sow_vec = zeros(length(epochs(:,1)),1);
    for i = 1:length(epochs)
        nrs_sow_vec(i,1)=(epochs(i,6)+60*epochs(i,5)+3600*epochs(i,4))+gps_dow*24*3600;
    end
    %Common time vectors
    [OL_R_common_sow OL_R_sowi R_OL_sowi] = intersect(GPS_MSOW,GPS_MSOWR);    
    [NRS_Range_common_vec NRS_Ri R_NRSi]=intersect(nrs_sow_vec,GPS_sow_range);
    [OL_Range_common_vec OL_Ri R_OLi]=intersect(OL_R_common_sow/1000,GPS_sow_range);
    [CL_Range_common_vec CL_Ri R_CLi]=intersect(CL_sow_vec,GPS_sow_range);
    [NRS_OL_R_common NOi ONi]=intersect(NRS_Range_common_vec,OL_Range_common_vec);
    [NRS_OL_common_sow NRS_OL_sowi OL_NRS_sowi] = intersect(nrs_sow_vec, GPS_MSOWR/1000);

    %Phase
    IntPredDopp = (cumsum(Freq).*1/OL_freq_hz);
    IntCLDopp = (cumsum(DopplerCL).*1/CL_freq_hz);
    
    CLPhase = IntCLDopp(CL_Ri)-IntCLDopp(CL_Ri(1));
    OLPhase = IntPredDopp(OL_R_sowi) + CorrCorrectPhaseR(R_OL_sowi)/2/pi- IntPredDopp(1) - CorrCorrectPhaseR(1)/2/pi;
    nrsPhaseL1 = (Obs.L1(:,rinexSVi))*lam1;
    nrsPhaseL2 = (Obs.L2(:,rinexSVi))*lam2;
    nrsPhaseLC = fl1^2*nrsPhaseL1/(fl1^2-fl2^2)-fl2^2*nrsPhaseL2/(fl1^2-fl2^2);
    
    %excess phases
    CL_exc=-CLPhase*lam1-(Range(R_CLi)-Range(R_CLi(1)));
    
    OLPhase=OLPhase*lam1;
    OL_exc=-(OLPhase(OL_Ri)-OLPhase(OL_Ri(1)))-(Range(R_OLi)-Range(R_OLi(1)));
    
    NRS_range=nrsPhaseL1(NRS_Ri);
    nrs_start_ind=find(~isnan(NRS_range));
    Range_NRS=Range(R_NRSi);
    
    
    NRS_excL1=(nrsPhaseL1(NRS_Ri)-NRS_range(nrs_start_ind(1)))-(Range(R_NRSi)-Range_NRS(nrs_start_ind(1)));
    NRS_excL1=NRS_excL1-NRS_excL1(nrs_start_ind(1));
    deriv_tol=0.5;
    NRS_excL1= fix_jumps (NRS_excL1,NRS_Range_common_vec,deriv_tol);
    
    NRS_excL2=nrsPhaseL2(NRS_Ri)-Range(R_NRSi);
    NRS_excL2=NRS_excL2-NRS_excL2(1);
    NRS_excL2= fix_jumps (NRS_excL2,NRS_Range_common_vec,deriv_tol);
    NRS_excLC=nrsPhaseLC(NRS_Ri)-Range(R_NRSi);
    NRS_excLC=NRS_excLC-NRS_excLC(1);
    NRS_excLC= fix_jumps (NRS_excLC,NRS_Range_common_vec,deriv_tol);
    
    % Parameters for nice plots
    SerialDateGPSTstart = 723186;
    DaysPerWeek = 7;
    gps_start_sd = SerialDateGPSTstart + DaysPerWeek*gps_week;
    
    NRS_OL_SD_VEC = NRS_OL_common_sow/3600/24 + gps_start_sd;
    NRS_R_SD_VEC=NRS_Range_common_vec/3600/24 + gps_start_sd;
    OL_R_SD_VEC=OL_Range_common_vec/3600/24 + gps_start_sd;
    CL_R_SD_VEC=CL_Range_common_vec/3600/24 + gps_start_sd;
    NRS_OL_R_VEC=NRS_OL_R_common/3600/24 + gps_start_sd;
    date = datestr(NRS_R_SD_VEC(1)); DMY = date(1:11);
    
    num_divisions = 4;
    AxisTime = datenum([NRS_R_SD_VEC(1) NRS_R_SD_VEC(end)]);
    ticksize_date = diff(AxisTime)/num_divisions;
    tickmarks = AxisTime(1):ticksize_date:AxisTime(2);
    
    %NetRS, OL and CL excess phase plots
    figure; hold on; grid on;
    plot(NRS_R_SD_VEC,NRS_excL1-min(NRS_excL1))
%     plot(NRS_R_SD_VEC,NRS_excL2,'r')
%     plot(NRS_R_SD_VEC,NRS_excLC,'g')
    
    [m,io]=min(abs(NRS_R_SD_VEC-OL_R_SD_VEC(1)));
    plot(OL_R_SD_VEC,OL_exc-min(OL_exc),'m')
    
    [n,ic]=min(abs(NRS_R_SD_VEC-CL_R_SD_VEC(1)));
    plot(CL_R_SD_VEC,CL_exc+NRS_excL1(ic),'k')    
    datetick('x',13,'keepticks')
    ylabel('Excess Phase(m)')
    xlabel('GPS Time    [hh:mm:ss]')
    title(sprintf('PRN %02d Excess Phase',PRN))
    if(setting)
        legend('NRS L1','OL','location','NorthWest')
    else
        legend('NRS L1','OL','location','NorthEast')
    end
    print('-dpdf','-r300',sprintf('../Plots/PRN%02d-%s-phase-all-detrend',SV,DMY))
   %______________________________________________
   
T_CL  = CL_R_SD_VEC;
T_OL  = OL_R_SD_VEC;
T_NRS = NRS_R_SD_VEC;
OL = OL_exc+NRS_excLC(io);
CL = CL_exc+NRS_excLC(ic);
L1 = NRS_excL1;
L2 = NRS_excL2;
LC = NRS_excLC;

fid =fopen('./T_excphase','w');

for k = 1:(length(NRS_R_SD_VEC))
fprintf(fid,'%12.10f %12.10f %12.10f %12.10f %12.10f %12.10f %12.10f %12.10f\n',[T_CL(k) T_OL(k) T_NRS(k) OL(k) CL(k) L1(k) L2(k) LC(k)]);
end
fclose(fid);
%_______________________________________________________
    %OL-CL compare
   
%     [ol_cl_vec oli cli]=intersect(OL_Range_common_vec,CL_Range_common_vec);
%     OL_CL_SD_VEC=ol_cl_vec/3600/24 + gps_start_sd;
%     OL_exc_c=OL_exc(oli)-OL_exc(oli(1));
%     CL_exc_c=CL_exc(cli)-CL_exc(cli(1));
%     
%     figure; hold on; grid on;  
%     plot(OL_CL_SD_VEC,OL_exc_c-CL_exc_c,'.','markersize',1)
%     datetick('x',13,'keepticks')
%     ylabel('Phase [m]')
%     xlabel('GPS Time    [hh:mm:ss]')
%     title(sprintf('PRN%02d Phase Difference(CL Phase - OL Phase)',PRN))
%     print('-dpdf','-r300',sprintf('PRN%02d_CL_OLphase',SV))
%     figure; hold on; grid on;
%     plot(OL_CL_SD_VEC(1:end-1),diff(OL_exc_c-CL_exc_c),'.','markersize',4)
%     datetick('x',13,'keepticks')
%     ylabel('d\Phi/dt [m/s]')
%     xlabel('GPS Time    [hh:mm:ss]')
%     title(sprintf('PRN%02d-diff(CL Phase - OL Phase)',PRN))
%     print('-dpdf','-r300',sprintf('../Plots/PRN%02d_diffCL_OLphase',SV))
    
    %NetRS - OL Compare   
    NRS_OL_c=nrsPhaseL1(NRS_OL_sowi);
    NrsOL_start_ind=find(~isnan(NRS_OL_c));
    NRS_OL_c=NRS_OL_c-NRS_OL_c(NrsOL_start_ind(1));
    OLPhase=circshift(OLPhase,0);
    OLPhase2=-OLPhase(OL_NRS_sowi);
    OL_NRS_c=OLPhase2-OLPhase2(NrsOL_start_ind(1));
    OL_NRS_diff=NRS_OL_c-OL_NRS_c;
    OL_NRS_diff=fix_jumps(flipud(OL_NRS_diff-OL_NRS_diff(NrsOL_start_ind(1))),NRS_OL_common_sow,0.5);

    figure; hold on; grid on;
    plot(NRS_OL_SD_VEC,flipud(OL_NRS_diff*100/lam1),'.','markersize',3)
    datetick('x',13,'keepticks')
    ylabel('Phase [m]')
    xlabel('GPS Time    [hh:mm:ss]')
    title(sprintf('PRN %02d NetRS Phase - OL Phase)',PRN))
    set(get(gcf,'CurrentAxes'));  
    print('-dpdf','-r300',sprintf('../Plots/PRN%02d-%s-NetRS-minus-OL-phase',SV,DMY))
    print('-dpng','-r300',sprintf('../Plots/PRN%02d-%s-NetRS-minus-OL-phase',SV,DMY))
    saveas(gcf,sprintf('PRN%02d-%s-NetRS-minus-OL-phase',SV,DMY),'fig')
   
    figure; hold on; grid on;
    plot(NRS_OL_SD_VEC(1:end-1),diff(NRS_OL_c-OL_NRS_c),'.','markersize',6)
    datetick('x',13,'keepticks')
    ylabel('d\Phi/dt [m/s]')
    xlabel('GPS Time    [hh:mm:ss]')
    title(sprintf('PRN %02d diff(NRS Phase - OL Phase)',PRN))
    print('-dpdf','-r300',sprintf('../Plots/PRN%02d-%s-diff-NRS-minus-OL-phase',SV,DMY))
    
end
