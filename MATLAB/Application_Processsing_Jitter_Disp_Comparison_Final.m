%% Application Throughput Graphs 

%% Data Loading
clear all 
close all
clc 

folderNames = {'NRTV-app1'; 'telemetryData-app2'; 'fileTransfer-app3'};
errRate = ["0.001/","0.050/","0.100/"];
ER = [0.001, 0.050, 0.100];
errTitles = ["0.01%","5.0%","10.0%"]; 
numFigs = 1; 
figures = gobjects(1,numFigs); 
titlePrint = 0; 
colors = ["#ff7f0e","red","blue"];
labelLetter = ["a.", "b.", "c.", "d.", "e.", "f."];

maxKeepAlive = 7200000; 
numRows = 2;
numCols = 3; 

saveFig = 0; 
annoText = false; % display annotation title between subplot rows 
Leo = 0; 

testRst = 1; 
test = 1;
BL = 1; 

for f = 1:numFigs
    % Create a new figure
    figures(f) = figure;
end 

erSel = 3; %errRate = ["0.001/","0.050/","0.100/"];


% load csv files
for loopNum=1:6
    
    if (loopNum < 4)
        erSel = loopNum; 
        base = true;
        config = "Single-App"; 
    else 
        erSel = testRst;
        testRst = testRst + 1; 
        base = false; 
        config = "Multi-App";
    end 

    % folderName = folderNames{test};
    folderName = folderNames{test};
    
    satConf = "GEO"; 
    
    baseline = 'Combined';
    Print = false;
    
    folderPath = '/home/drew/Thesis/csvData/ErrRate';
    
    if (base)
        folderPath = '/home/drew/Thesis/baselineCsvData/ErrRate';
        baseline = 'Baseline';
    end 
    
    if (Leo) 
        folderPath = '/home/drew/Thesis/csvData/LEO/ErrRate';
        satConf = "LEO";
        if (base)
            baseline = 'Baseline';
            folderPath = '/home/drew/Thesis/baselineCsvData/LEO/ErrRate'; 
        end 
    end
    
    
    folderPath = strcat(folderPath,errRate(erSel));
    
    folderPath = strcat(folderPath,folderName);
    
    files = dir(fullfile(folderPath, '*.csv'));
    
    allFiles = length(files); 
    fileNum = 7;
    % if loopNum ~= 4
    %     fileOffset = 0; 
    % else 
    %     fileOffset = 10; 
    % end 

    fileOffset = 0; 

    fprintf("Processing Files for %s %s App %d Error Rate: %s\n",satConf,baseline,test,errTitles(erSel))
    % Extract the numerical parts of the filenames
    fileNums = zeros(1, fileNum);
    for i = 1:allFiles-fileOffset
        fileName = files(i).name;
        numStr = regexp(fileName, 'RUN-(\d+)_', 'tokens'); % Extract numbers
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
    
        Time{t}  = dataTable{:,1};
        Bytes{t} = dataTable{:,2};

        % if loopNum == 4 && length(files) ~= 1000 && satConf =="GEO"
        %     Bytes = Bytes(11:end);
        %     Time = Time(11:end);
        % end
    
        [t_bytes{t} t_average{t}] = adjustThput(Time{1,t}, Bytes{1,t}); 

        
    
        % fprintf("Length of Time{%d}: %d\n",t,length(Time{t}))
        
    end 

   
    fprintf("Processing File: %s\n",fullFilePath);
    fprintf("Done Loading Data From: %s for %s Test\n",folderName,satConf)

    
    %% Inter Arrival Times
    % l = 0; 
    interArrData = cell(1, length(Time));
    for j = 1:length(Time)
        if length(Time{j}) > 1
            interArrData{j} = diff(Time{j});
        else
            interArrData{j} = []; % Handle cases where Time{j} has less than 2 elements
        end
    end
    disp("Done with Jitter Calculation")
    
    % Calculate Average Jitter 
    % Max interarrival Time (latency)
    if Leo && erSel == 3
        fprintf("Filter out packets greater than maxKeepAlive = %d\n",maxKeepAlive); 
        for j=1:length(Time)
            validIndices = interArrData{j} <= maxKeepAlive; %filter out after 2hr keep alive time
            interArrData{j} = interArrData{j}(validIndices); 
        end 
    end 
    %% Calculate Jitter: 

    for k=1:length(interArrData)
        avgJitter(k) = sqrt(var(interArrData{k})); 
    end 
    
    filtered_avgJitter = hampel(avgJitter);
    
    if Leo && erSel == 3
        fprintf("Filtering Jitter values----\n")
        validIndices = avgJitter < 150; 
        filtered_avgJitter = avgJitter(validIndices); 
    end 

    if loopNum == 4 && erSel ~= 3 && ~Leo 
        filtered_avgJitter = avgJitter; 
    end 

    avgJ = mean(avgJitter)*1000;
    avgJ_f = mean(filtered_avgJitter*1000);
    
    if (Leo == 1)
        binLims = {[3.65 3.783], [3.9 4.12]};
    end
    

    Nbins = 50; 
    figure(1);
    set(gcf,"Position", [100 100 1200 900])

    subtitleText = strcat(folderName," ",errTitles(erSel)," ",config);

    avgUpdatedJ = 0; 
    subplot(numRows,numCols,loopNum)    
    x = 0.5; % X position in normalized units
    y = 1.03; % Y position in normalized units (above the title)
    hold on;
    if loopNum <=6
        histogram(filtered_avgJitter*1000,'NumBins',Nbins)
        hold on; 
        yLimits = ylim;
        plot([avgJ_f avgJ_f], yLimits, 'r--', 'LineWidth', 2);
       
        text(avgJ_f+0.00005, yLimits(2) * 0.95, sprintf('Mean: %.4f msec', avgJ_f), ...
            'HorizontalAlignment', 'left','Color', 'red', 'FontSize', 12);
       
        hold off;
        xlabel({sprintf('Time in msec'),labelLetter(loopNum)}, ...
            'HorizontalAlignment', 'center', ...
            'FontWeight', 'normal')         
        ylabel("Counts")
        
        if titlePrint
            title(subtitleText)
        end 

    else
        updatedIndices = filtered_avgJitter*1000 <= binLims{BL}(2);% & filtered_avgJitter >= binLims{BL}(1);
        updatedJitter = filtered_avgJitter(updatedIndices); 

        avgUpdatedJ = mean(updatedJitter*1000); 

        hold on; 
        textVal = avgUpdatedJ; %(binLims{BL}(1)+ binLims{BL}(2)) /2 ;
        histogram(filtered_avgJitter*1000,'BinLimits',binLims{BL},'NumBins',Nbins)
        yLimits = ylim;
        plot([textVal textVal], yLimits, 'r--', 'LineWidth', 2);
       
        % Add a text label for the average
        text(textVal+0.00005, yLimits(2) * 0.95, sprintf('Mean: %.4f msec', avgUpdatedJ), ...
            'HorizontalAlignment', 'left','Color', 'red', 'FontSize', 12);
        
        xlabel({sprintf('Time in msec'),labelLetter(loopNum)}, ...
            'HorizontalAlignment', 'center', ...
            'FontWeight', 'normal')         
        ylabel("Counts")
        hold off;
        BL = BL+1; 

        if titlePrint
            title(subtitleText)
        end 


    end 
    
    
    if (loopNum ==3 && annoText)
        annoTextRow1_J = sprintf("%s %s Filtered Distribution of Average Jitter for %s at %s Error Rate",satConf,baseline, numTrials, errTitles(erSel)); 
        annotation('textbox', [0.1, 0.925, 0.8, 0.05], 'String', annoTextRow1_J, ...
        'EdgeColor', 'none', 'HorizontalAlignment', 'center', 'FontSize', 15);
    end
    
    if (loopNum > 5 && annoText)
        annoTextRow2_J = sprintf("%s %s Filtered Distribution of Average Jitter for %s at %s Error Rate",satConf,baseline, numTrials, errTitles(erSel)); 
        annotation('textbox', [0.1, 0.925/2, 0.8, 0.05], 'String', annoTextRow2_J, ...
        'EdgeColor', 'none', 'HorizontalAlignment', 'center', 'FontSize', 15);
    end 

    figure('Position',[100 100 1200 900])
    hold on;
    histogram(filtered_avgJitter*1000,'NumBins',80)
    % Add a text label for the average

    if loopNum > 3
        set(gca, 'YScale', 'log')
        yLimits = ylim;
        ylim([1 max(yLimits(2), 1e3)])
    end 

    yLimits = ylim;  % Get new limits


    plot([avgJ_f avgJ_f], yLimits, 'r--', 'LineWidth', 2);
    text(avgJ_f+0.00008, yLimits(2) * 0.9, sprintf('Mean: %.2f msec', avgJ_f), ...
        'HorizontalAlignment', 'left', 'Color', 'red', 'FontSize', 12);
    ax = gca;
    text(ax.XLim(2)/2, ax.YLim(1)-0.1*(ax.YLim(2)-ax.YLim(1)), labelLetter(loopNum), ...
    'HorizontalAlignment', 'center', 'FontWeight', 'bold') 
    hold off;
    xlabel("Time in msec")
    ylabel("Counts")
    
    if titlePrint
            title(subtitleText)
    end 
    
    
    fprintf("Average Filtered Jitter: %.6fmsec\n",avgJ_f);
    fprintf("Average Updated Filtered Jitter: %.6fmsec\n",avgUpdatedJ);

    
    figurePath = '/home/drew/matlab_code/Thesis/figures/';
    
    if (Leo)
       figurePath = strcat(figurePath,baseline,'/',satConf,'/ErrRate',errRate(erSel));
    else 
        figurePath = strcat(figurePath,baseline,'/ErrRate',errRate(erSel));
    end
    
    jitterHistName = strcat('Jitter-',folderName,'_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
    fprintf("Saving %s\n",jitterHistName);
    jitterHistName = strcat(figurePath,'jitter/',folderName,'/Jitter-',folderName,'_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
    if (saveFig)
         saveas(gcf,jitterHistName);
         fprintf("Success\n");
    else 
        fprintf("Enable saveFig to Save Figures ")
    end

    fprintf("--Standard Deviation of Jitter Averages: %.2f--\n",std(filtered_avgJitter));
    fprintf("--Variance of Jitter Averages %.2f--\n",var(filtered_avgJitter));
   
    
    fprintf("----Done Processing %s %s %s\n",satConf,folderName,errRate(erSel));
end 


%% Unused 

  
    % Save figures in thputGraphImags
    
        % thputGraphsImgs = cell(1,length(Time));
        % for i=1:length(Time)
        %     t_time = 0:1:length(t_bytes{i})-1;
        %     hold on; 
        %     plot(t_time,t_bytes{1,i}*8/1000)
        %     plot(t_time,t_average{1,i})
        %     title(sprintf("%s Simulation: %s Run: %d Throughput",baseline,folderName,i));
        %     legend("Throughput in kbps",sprintf("Avg Throughput: %.2fkbps",t_average{1,i}(1)),'Location', mapLoc(test))
        %     xlim([0 length(t_bytes{i})])
        %     xlabel("Seconds");
        %     ylabel("kbps")
        %     hold off
        %     % Define a unique filename for each figure
        %     thputFileName = sprintf('thputGraphImgs%d.fig', i); %i corresponds to run number
        % 
        %     % Save the figure
        %     savefig(thputFileName);
        % 
        %     % Store the filename in the cell array
        %     thputGraphsImgs{i} = thputFileName;
        % end 

          % if (errRate(erSel) == '0.001/')
    %     fprintf("Bin Edges for errRate %s\n",errRate);
    %     binEd1 = 1120:2:1180;% bin edges for App1 
    %     binEd2 = .4:0.02:1.2; %bin edges for App2 
    %     binEd3 = 600:50:2450; %bin Edges for App3
    % 
    % elseif (errRate(erSel) == '0.005/')
    %     fprintf("Bin Edges for errRate %s\n",errRate);
    %     binEd1 = 1080:2:1110;% bin edges for App1    
    %     binEd2 = 0:2:100; %bin edges for App2 
    %     binEd3 = 40:2:110; %bin Edges for App3
    % 
    % elseif (errRate(erSel) == '0.100/')
    %     fprintf("Bin Edges for errRate %s\n",errRate);
    %     binEd1 = 1080:2:1110;% bin edges for App1    
    %     binEd2 = 0:2:100; %bin edges for App2 
    %     binEd3 = 40:2:110; %bin Edges for App3
    % 
    % end 
    % 
    % if (baseline)
    %     if (errRate(erSel) == '0.001/')
    %         fprintf("Bin Edges for baseline-errRate %s\n",errRate);
    %         Ed1 = 0.000118:0.001:000125;% bin edges for App1    
    %         Ed2 = 1.5:0.075:4; %bin edges for App2 
    %         Ed3 = 1.5:0.1:3.5; %bin Edges for App3
    % 
    %     elseif (errRate(erSel) == '0.005/')
    %         fprintf("Bin Edges for baseline-errRate %s\n",errRate);
    %         Ed1 = 0:0.05:4;% bin edges for App1    
    %         Ed2 = 1.5:0.075:4; %bin edges for App2 
    %         Ed3 = 1.5:0.1:3.5; %bin Edges for App3
    % 
    %     elseif (errRate(erSel) == '0.100/')
    %         fprintf("Bin Edges for baseline-errRate %s\n",errRate);
    %         Ed1 = 0:0.05:4;% bin edges for App1    
    %         Ed2 = 1.5:0.075:4; %bin edges for App2 
    %         Ed3 = 1.5:0.1:3.5; %bin Edges for App3
    %     end 
    % end 
    % 
    % edges = {Ed1, Ed2, Ed3};

    
    % if (erSel == 1)
    %     fprintf("Bin Edges for errRate %s\n",errRate(erSel));
    %     binEd1 = 1120:2:1180;% bin edges for App1 
    %     binEd2 = 1000:50:2000; %bin edges for App2 
    %     binEd3 = 600:50:2450; %bin Edges for App3
    % 
    % elseif (erSel == 2)
    %     fprintf("Bin Edges for errRate %s\n",errRate(erSel));
    %     fprintf("Got here");
    %     binEd1 = 1080:2:1110;% bin edges for App1    
    %     binEd2 = 0:2:100; %bin edges for App2 
    %     binEd3 = 40:2:110; %bin Edges for App3
    % 
    % elseif (erSel == 3)
    %     fprintf("Bin Edges for errRate %s\n",errRate(erSel));
    %     binEd1 = 1050:2:1080;% bin edges for App1    
    %     binEd2 = 0:2:100; %bin edges for App2 
    %     binEd3 = 0:10:150; %bin Edges for App3
    % end 
    % 
    % if (base)
    %     if (errRate(erSel) == '0.001/')
    %         fprintf("Bin Edges for baseline\n");
    %         binEd1 = 1135:2:1165;% bin edges for App1 
    %         binEd2 = 1000:50:1700; %bin edges for App2 
    %         binEd3 = 700:50:2100; %bin Edges for App3
    % 
    %     elseif (errRate(erSel) == '0.005/')
    %         fprintf("Bin Edges for baseline\n");
    %         binEd1 = 1130:2:1160;% bin edges for App1    
    %         binEd2 = 250:10:500; %bin edges for App2 
    %         binEd3 = 250:15:600; %bin Edges for App3
    % 
    %     elseif (errRate(erSel) == '0.100/')
    %         fprintf("Bin Edges for baseline\n");
    %         binEd1 = 1020:5:1050;% bin edges for App1    
    %         binEd2 = 10:5:80; %bin edges for App2 
    %         binEd3 = 20:5:140; %bin Edges for App3
    %     end 
    % end 
    % 
    % binEdges = {binEd1, binEd2, binEd3};
    
    % figure('Position',[100 1001 900 600])
    % histogram(averages,binEdges); 
    % title(sprintf("Distribution of Average Throughput for 100 Simulations for %s at 0.001 Error Rate",folderName))