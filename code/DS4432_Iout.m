% This function calculates the output current from DS4432,
% given the reference resistnace and DAC setting.
%
% Iout = DS4432_Iout(Rfs,DAC);
% Inputs:   Rfs - number>0, in k Ohms;
%           DAC - 8-bit binary or 2-byte hex string;
% Outputs:  Iout in uA;

function Iout=DS4432_Iout(Rfs,DAC)
Vrfs=0.997;
Ifs=(Vrfs/Rfs)*(127/16);
if (length(DAC)==8)&&(Rfs>0)
    Iout=Ifs*(bin2dec(DAC(2:8)))/127*1000;
    disp(sprintf('\nIout = %d uA \n',Iout));
elseif (length(DAC)==2)&&(Rfs>0)
    DAC=dec2bin(hex2dec(DAC));
    Iout=Ifs*(bin2dec(DAC(2:8)))/127*1000;
    disp(sprintf('\nIout = %d uA \n',Iout));
elseif (length(DAC)==8)||(length(DAC)==2)
    disp(sprintf('\nError: Rfs must be positive! \n'));
else
    disp(sprintf('\nError: DAC must be an 8-bit binary or 2-byte hex! \n'));
end
end

