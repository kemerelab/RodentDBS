% This function calculates the DAC value to set in the software for DS4432,
% given the reference resistnace and desired output current.
%
% DAC = DS4432_DAC(Rfs, Iout);
% Inputs:   Rfs - number>0, in k Ohms;
%           Iout in uA;
% Outputs:  DAC - 2-byte hex;

function DAC = DS4432_DAC(Rfs, Iout)

if (Iout>0)&&(Rfs>0)
    Vrfs=0.997;
    Ifs=(Vrfs/Rfs)*(127/16);
    DAC_val=Iout*127/(Ifs*1000);
    DAC='10000000';
    bin_string=dec2bin(round(DAC_val),7);
    if length(bin_string)==7
        DAC(2:8)=bin_string;
        DAC=dec2hex(bin2dec(DAC));
        disp(sprintf('\nSet DAC to 0x%sh \n', DAC));
    else 
        disp(sprintf('\nError: Required Iout is out of range!\n', DAC));
    end
elseif (Iout>0)
    disp(sprintf('\nError: Rfs must be positive! \n'));
else
    disp(sprintf('\nError: Iout must be positive! \n'));
end
end

