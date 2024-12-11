function [t_bytes, t_average] = adjustThput(time,bytes)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here

k = 1;
t_bytes(k) = 0; 
temp = 0;
for i=1:length(time)
    if (time(i)<k) 
        t_bytes(k) = temp + bytes(i); 
        temp = t_bytes(k);
    else 
        k = k+1; 
        t_bytes(k) = bytes(i); 
        temp = t_bytes(k);
    end
end 

t_average = repmat(sum(bytes)*8/1000/max(time),size(t_bytes));

end