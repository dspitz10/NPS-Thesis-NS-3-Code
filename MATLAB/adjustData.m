function [adjusted_Time,Adjusted_Bytes] = adjustData(Time,Bytes,seconds)
%UNTITLED2 Summary of this function goes here
%   Detailed explanation goes here

adjusted_Time = cell(1,length(Time));
Adjusted_Bytes = cell(1,length(Time)); 

    for k = 1:length(Time)
    
        currentTime = Time{k};
    
        validIndices = currentTime >= seconds; 
    
        adjusted_Time{k} = Time{k}(validIndices); 
        Adjusted_Bytes{k} = Bytes{k}(validIndices);
    
    end 

end