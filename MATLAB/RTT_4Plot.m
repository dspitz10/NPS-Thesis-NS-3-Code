%% Process RTT Values from CSV Files 
clear all; close all; clc; 

folderNames = {'telemetryData-app2'; 'fileTransfer-app3'};
bad = 0; 
if bad 
    folderNames{1} = 'telemetryData-app2_bad'; 
end 
errRate = ["0.001/","0.050/","0.100/"];
ER = [0.001, 0.050, 0.100];
errTitles = ["0.01%","5.0%","10.0%"]; 
numFigs = 2; 
figures = gobjects(1,numFigs); 
colors = ["#ff7f0e","red","blue"];
labelLetter = ["a.", "b.", "c.", "d.", "e.", "f."];

maxKeepAlive = 7200000; 
numRows = 2;
numCols = 2; 

saveFig = 0; 
annoText = false; % display annotation title between subplot rows 
Print = 0; % print Files as they are loaded 
binLimits = 1; %toggle 'BinLimits' Option for Histogram
titlePrint = 0; % Toggle titles on figures
BL2 = 1; 


erSel = 2; 
testRst = 2; 
test = 1;
BL = 1; 
base = 1; 
Leo = 1;  % For LEO Configuration
Lan = 0; 

for loopNum=1:4
    
    
    folderName = folderNames{test};
    satConf = "GEO"; 
    
    baseline = 'Combined';
    
    folderPath = '/home/drew/Thesis/RTTdata/ErrRate';
    
    if (base)
        folderPath = '/home/drew/Thesis/baselineRTTdata/ErrRate';
        baseline = 'Baseline';
    end 
    
    if (Leo) 
        folderPath = '/home/drew/Thesis/RTTdata/LEO/ErrRate';
        satConf = "LEO";
        if (base)
            baseline = 'Baseline';
            folderPath = '/home/drew/Thesis/baselineRTTdata/LEO/ErrRate'; 
        end 
    end

    folderPath = strcat(folderPath,errRate(erSel));
    
    folderPath = strcat(folderPath,folderName);

    if (Lan)
        satConf = "Starlink";
        baseline = "Baseline";
        folderPath = "/home/Thesis/LANForgeExperiments/ErrRate";
    end 

    files = dir(fullfile(folderPath, '*.csv'));
    
    allFiles = length(files); 
    % if loopNum ~= 4
    %     fileOffset = 0; 
    % else 
    %     fileOffset = 10; 
    % end 

    fileOffset = 0; 

    fprintf("Processing Files for %s %s %s Error Rate: %s\n",satConf,baseline,folderName,errTitles(erSel))
    % Extract the numerical parts of the filenames
    fileNums = zeros(1, allFiles);
    for i = 1:allFiles-fileOffset
        fileName = files(i).name;
        numStr = regexp(fileName, '_(\d+)_', 'tokens'); % Extract numbers
        if ~isempty(numStr)
            fileNums(i) = str2double(numStr{1}{1});
        end
    end
    
    % Sort files based on the extracted numbers
    [~, sortedIndices] = sort(fileNums);
    files = files(sortedIndices);
    Ntrials = num2str(max(sortedIndices));
    numTrials = strcat(Ntrials," Trials");
    
    % Initialize a cell array to hold the tables
    tableArray = cell(1, length(files));
    Time = cell(1, length(files)); 
    Bytes = cell(1,length(files));
    t_bytes = cell(1,length(files));
    t_average = cell(1,length(files));
    
    for t = 1:length(files) 
        fileName = files(t).name; 
    
        if isequal(fileName, '.') || isequal(fileName, '..')
            continue;
        end
    
        % Full file path
        fullFilePath = fullfile(folderPath, fileName);
    
        % Display or process each file
        
        if (Print)
            fprintf("Processing File: %s\n",fullFilePath);
        end 
    
        % Load the CSV file into a table
        dataTable = readtable(fullFilePath);
    
        % Store the table in the cell array
        tableArray{t} = dataTable;
    
        % Read in RTT data from DataTable 
        RTT{t}  = dataTable{:,6};
    end 

    fprintf("All Files from %s :%d\n",folderPath,allFiles);

%%
    fprintf("\n------Data Analysis for %s Simulation: %s------\n\n",baseline,folderName);
    
    averages = []; 
    for i=1:length(RTT)
        averages = [averages, mean(RTT{i})];
    
    end 
    
    % filtered_averages = hampel(averages*1000);
    if binLimits 
        binLims = {[500 700] [600 1400] [700 1600] [450 750] [600 1500] [800 1650]};
        if BL2
            binLims = {[600 1400] [700 1600] [600 1500] [800 1650]}; 
        end 
        if Leo 
            binLims = {[60 160] [150 350] [200 800] [50 155] [150 750] [150 800]}; 
            if BL2
                binLims = {[150 350] [100 750] [150 880] [150 800]}; 
            end    
        end 
        %     updatedIndices = averages <= binLims_thput{loopNum}(2) & averages >= binLims_thput{loopNum}(1);

        updatedIndices = averages*1000 <= binLims{loopNum}(2) & averages*1000 >=binLims{loopNum}(1);
         
        filtered_averages = averages(updatedIndices);
        filtered_averages = filtered_averages*1000; % convert to msec

        % updatedIndices = averages*1000 <= binLimsJ{BL}(2);
        % updatedAverages = filtered_aveages(updatedIndices); 
        % 
        % avgUpdatedJ = mean(updatedAvgerages*1000);

    else
        averages = averages*1000;   
    
        filtered_averages = hampel(averages); 
    end
    

    %% unfiltered Averages 

    

    Nbins = 50;
    subtitleText = strcat(folderName," ",errTitles(erSel),"-  Unfiltered");
    figure(1);
    set(gcf,'Name',"Unfiltered")
    set(gcf,"Position", [100 100 1500 800]) 
    subplot(numRows,numCols,loopNum)
    avgRTT = mean(averages);
    histogram(averages,'Numbins',Nbins)
    hold on; 
    yLimits1 = ylim;
    xlabel({sprintf('Time in msec'),labelLetter(loopNum)}, ...
                'HorizontalAlignment', 'center', ...
                'FontWeight', 'normal') 
    ylabel("Counts")
    plot([avgRTT avgRTT], yLimits1, 'r--', 'LineWidth', 2);
     % Add a text label for the average
    text(avgRTT + 3, yLimits1(2) * 0.9, sprintf('Mean: %.4f msec', avgRTT), ...
        'HorizontalAlignment', 'left', 'Color', 'red', 'FontSize', 12);
    hold off;
    if titlePrint
        title(subtitleText)
    end 
    

    %% Filtered averages
    
   

    
    subtitleText = strcat(satConf," ",folderName," ",errTitles(erSel),"-  Filtered");
    figure(2);
    set(gcf,'Name',"Filtered")
    set(gcf,"Position", [100 100 1500 800]) 
    subplot(numRows,numCols,loopNum)
    avgRTT_f = mean(filtered_averages);
    Nbins = 50; 
    histogram(filtered_averages,Nbins);
    if binLimits
        histogram(filtered_averages,'BinLimits',binLims{loopNum},'Numbins',Nbins);
    end
     
    hold on; 
    yLimits1 = ylim;
    xlabel({sprintf('Time in msec'),labelLetter(loopNum)}, ...
                'HorizontalAlignment', 'center', ...
                'FontWeight', 'normal') 
    ylabel("Counts")

    %-----------------------------------------------------------------

    plot([avgRTT_f avgRTT_f], yLimits1, 'r--', 'LineWidth', 2);
     % Add a text label for the average
    text(avgRTT_f + 3, yLimits1(2) * 0.9, sprintf('Mean: %.4f msec', avgRTT_f), ...
        'HorizontalAlignment', 'left', 'Color', 'red', 'FontSize', 12);
    %-----------------------------------------------------------------

    hold off;
    if titlePrint
        title(subtitleText)
    end 
    
    figure('Position',[100 100 1200 900])
    avgRTT_f = mean(filtered_averages);
    histogram(filtered_averages,Nbins);
     if binLimits
        histogram(filtered_averages,'BinLimits',binLims{loopNum},'Numbins',Nbins)
    end
     
    hold on; 
    yLimits2 = ylim; 
    xlabel("Time in msec")
    ylabel("Counts")
    plot([avgRTT_f avgRTT_f], yLimits2, 'r--', 'LineWidth', 2);

     % Add a text label for the average
        text(avgRTT_f +0, yLimits2(2) * 0.9, sprintf('Mean: %.4f msec', avgRTT_f), ...
            'HorizontalAlignment', 'left', 'Color', 'red', 'FontSize', 12);
        hold off;
        if titlePrint 
            title(subtitleText)
        end 
    
     figurePath = '/home/drew/matlab_code/Thesis/figures/RTT/';
     figurePathLan = '/home/drew/matlab_code/Thesis/figures/RTT/LANforge/';
        
     if (Leo)
           figurePath = strcat(figurePath,baseline,'/',satConf,'/ErrRate',errRate(erSel));
     else 
            figurePath = strcat(figurePath,baseline,'/ErrRate',errRate(erSel));
     end
    
    if (saveFig)
        rttHistName = strcat(figurePath,folderName,'/RTT-',folderName,'_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
        if Lan
            rttHistName = strcat(figurePathLan,'RTT-','LANforge','_',NTrials,'_',num2str(ER(erSel)),'ER.png'); 
        end
        fprintf("Saving File To: %s",rttHistName);     
        saveas(gcf,rttHistName);
        fprintf("Success\n");
    else 
        fprintf("Enable saveFig to Save Figures\n")
    end

    if loopNum < 2
        erSel = erSel +1; 
    end % loopNum == 3
        
    if loopNum >= 2
        test = 2; 
        erSel = testRst;
        testRst = testRst +1; 
    end


    fprintf("Done Processing Data From: %s for %s Test\n",folderName,satConf);

end 
