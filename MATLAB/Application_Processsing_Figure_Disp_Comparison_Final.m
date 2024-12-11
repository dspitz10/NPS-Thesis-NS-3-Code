%% Application Throughput Graphs 

%% Data Loading
clear all 
close all
clc 

folderNames = {'NRTV-app1'; 'telemetryData-app2'; 'fileTransfer-app3'};
bad = 0; 
if bad 
    folderNames{2} = 'telemetryData-app2_bad'; 
end 

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

saveFig = false; 
testRst = 1; 
BL  = 1; 
binLimits = 1;
Print = false;
Leo = 1; 

for f = 1:numFigs
    % Create a new figure
    figures(f) = figure;
end 

erSel = 1; %errRate = ["0.001/","0.050/","0.100/"];


% load csv files
for loopNum=1:4
    
    if (loopNum < 4)
        test = loopNum; 
        base = true;

    else 
        test = testRst;
        testRst = testRst + 1; 
        base = false; 
    end 

    folderName = folderNames{test};
    
    satConf = "GEO"; 
    
    baseline = 'Combined';
   
    
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
    
        % Load the CS%s for %s TesV file into a table
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

    %% Data Processing and Graphs
    % close all
    
    fprintf("\n------Data Analysis for %s Simulation: %s------\n\n",baseline,folderName);

    mapLoc = ["southeast","northeast","northeast"];
    
    % Histogram of Throughput Averages 
    % print to see what we can get
    averages = []; 
    for i=1:length(t_average)
        % fprintf("t_average{%d}: ",i);
        % disp(t_average{i}(1))
        averages = [averages, t_average{i}(1)];
    end

    unfiltered_avg = mean(averages); 
    fprintf("Unfiltered Average: %.4f for %s at %s Error\n",unfiltered_avg,folderName,errTitles(erSel));
    
    if binLimits
        if erSel == 1 
            binLims_thput = {[1130 1165] [450 1645] [0 2750] [1130 1165] [0 2000] [0 2200]};
        elseif erSel == 2 
            binLims_thput = {[1075 1105] [0 100] [0 100] [1075 1105] [0 100] [0 100]};
        elseif erSel == 3 
            binLims_thput = {[1017 1051] [0 90] [0 150] [1020 1050] [0 100] [0 100]};
        end 

        if Leo
            if erSel == 1 
                binLims_thput = {[1137 1170] [1000 2000] [950 4000] [1137 1170] [1000 2750] [1000 4000]};
            elseif erSel == 2 
                binLims_thput = {[1078 1110] [50 170] [70 150] [1075 1116] [60 150] [80 140]};
            elseif erSel == 3 
                binLims_thput = {[1017 1051] [20 250] [0 100] [1020 1050] [0 100] [0 100]};
            end  
        end 


    updatedIndices = averages <= binLims_thput{loopNum}(2) & averages >= binLims_thput{loopNum}(1);
        
        if loopNum ==4 
             updatedIndices= averages >= binLims_thput{loopNum}(1);
        end  
        filtered_averages = averages(updatedIndices); 
    else
        filtered_averages = hampel(averages); 
    end 

    % filtered_averages = hampel(averages);
    % if Leo
    %     filtered_averages = hampel(averages,3,3); 
    % end
    % 
    % if loopNum == 4 && erSel ~= 3 && ~Leo
    %     filtered_averages = averages;
    % end 
    
    % 
    % figure;
    % plot(time,bytes,Linewidth=2);
    % title("OnOff Throughput")
    % index = 14; 
    % fprintf("Plot Graphs at index %d and Average for %s simulation: %s -- %s\n",index,baseline,folderName,satConf)
    % 
    % % t_bps = t_bytes{i}*8/1000;
    % figure(1); 
    % set(gcf,"Position", [100 100 1500 1200])
    % if loopNum < 4
    %     subplot(4,1,loopNum)
    %     t_time = 0:1:length(t_bytes{index})-1;
    %     plot(t_time,t_bytes{1,index}*8/1000,'Color',colors(loopNum),'LineStyle','-')
    %     % plot(t_time,t_average{1,i2})
    %     if titlePrint
    %         title(sprintf("%s. Simulation: %s Run: %d Throughput",baseline,folderName,index));
    %     end
    % 
    %     legend(sprintf("Throughput in kbps Avg Throughput: %.2f kbps",t_average{1,index}(1)),'Location', mapLoc(test))
    %     xlim([0 length(t_bytes{index})])
    %     xlabel(sprintf("%s.      Seconds",labelLetter(loopNum)));
    %     ylabel("kbps")
    % 
    %     subplot(4,1,4)
    %     hold on;
    %     if (loopNum ==1)
    %         yyaxis right
    %         plot(t_time,t_bytes{1,index}*8/1000,'Color',colors(loopNum),'LineStyle','-');
    %         ylabel("App 1 in kbps",'Color','k')
    %         ax = gca; % Get current axes
    %         ax.YColor = 'k'; % Set y-axis color
    %         % legend("Throughput: "+folderNames(1)
    %     elseif loopNum ==2
    %         yyaxis left
    %         plot(t_time,t_bytes{1,index}*8/1000,'Color',colors(loopNum),'LineStyle','-');
    %     elseif loopNum ==3 
    %         yyaxis left
    %         ax = gca; % Get current axes
    %         ax.YColor = 'k'; % Set y-axis color
    %         plot(t_time,t_bytes{1,index}*8/1000,'Color',colors(loopNum),'LineStyle','-');
    %         xlabel("d.      Seconds");
    %         ylabel("Apps 2 & 3 in kbps",'Color','k');
    %     end 
    %     xlim([0 length(t_bytes{index})]);
    %     legend("Throughput: "+folderNames(2),"Throughput: "+folderNames(3),"Throughput: "+folderNames(1),Location="east");
    %     hold off; 
    % end 
    % 
    % figurePath = '/home/drew/matlab_code/Thesis/figures/';
    % 
    % if (Leo)
    %    figurePath = strcat(figurePath,baseline,'/',satConf,'/ErrRate',errRate(erSel));
    % else 
    %     figurePath = strcat(figurePath,baseline,'/ErrRate',errRale', 'log')
    %     yLimits = ylim;
    %     ylim([1 max(yLimite(erSel))]);
    % end
    % 
    % runSel = 9;
    % % t_bytes_avg = avgFunc(t_bytes);
    % t_time = 0:1:length(t_bytes{1,runSel})-1;
    % 
    % globalAverage = repmat(mean(averages),size(t_time));%repmat(sum(t_bytes_avg)*8/1000/max(t_time),size(t_bytes_avg));
    % fprintf("%s Simulation: %s Overall Average for %s: %.2fkbps\n",baseline,folderName,numTrials,globalAverage(1))
    % 
    % % thputGraphs_app3_100T_0.001ER
    % if loopNum == 6
    %     figurePath = strcat(figurePath,'throughput/ThputGraphs/','comparison','/');
    %     thputGraphName = strcat('thputGraphs_','comparison','_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
    % 
    %     % thputGraphName = strcat(figurePath,'thputGraphs_',folderName,'_',Ntrials,'.png');
    %     thputGraphName = strcat(figurePath,thputGraphName); 
    %     if (saveFig)
    %         fprintf("Saving %s To:\n%s\n",thputGraphName,figurePath)
    %         saveas(gcf,thputGraphName);
    %         fprintf("Success\n");
    %     else 
    %         fprintf("Enable saveFig to Save Figures ");
    %     end
    % 
    % end


    %% Throughput Plots

    fprintf("Plot First 3 graphs and Average for %s\n",folderName)
        
        index = 9; 
        if loopNum ==3 
            index = 7;
        end 
        
        figure(1)
        set(gcf,'Position',[100 100 1200 900]);
        
        % t_bps = t_bytes{i}*8/1000;
        if loopNum < 4
            grid on;
            t_time = 0:1:length(t_bytes{index})-1;
            subplot(4,1,loopNum);
            hold on; 
            plot(t_time,t_bytes{1,index}*8/1000)
            plot(t_time,t_average{1,index})
            % title(sprintf("%s %s Run: %d Throughput",folderName,errRate(erSel),index));
            legend("Throughput in kbps",sprintf("Avg Throughput: %.2fkbps",t_average{1,index}(1)),'Location', mapLoc(3))
            if loopNum == 1
                legend("Throughput in kbps",sprintf("Avg Throughput: %.2fkbps",t_average{1,index}(1)),'Location', mapLoc(1))
            end 

            xlim([0 length(t_bytes{index})])
            % xlim([0 205])

            xlabel({sprintf('Throughput in kbps'),labelLetter(loopNum)}, ...
                    'HorizontalAlignment', 'center', ...
                    'FontWeight', 'normal');
            ylabel("kbps")

            subplot(4,1,4); 
            hold on; 
            if loopNum == 1
                yyaxis right
                plot(t_time,t_bytes{1,index}*8/1000)
            else 
                yyaxis left
                plot(t_time,t_bytes{1,index}*8/1000,'LineWidth',1.5)
                % plot(t_time,t_average{1,index})
                % title(sprintf("%s %s Run: %d Throughput",folderName,errRate(erSel),index));
                % xlim([0 length(t_bytes{index})])
                xlabel({sprintf('Throughput in kbps'),labelLetter(4)}, ...
                        'HorizontalAlignment', 'center', ...
                        'FontWeight', 'normal');
                ylabel("kbps")
                xlim([0 205])
            end 
            legend(folderNames{2},folderNames{3},folderNames{1})

        end 

        
        
     

    %% Histogram of Throughput Averages
    % 
    % Nbins = 50;
    % subtitleText = strcat(folderName," ", errTitles(erSel));
    % 
    % % figure('Position',[100 1001 900 600])
    % figure(2);
    % set(gcf,"Position", [100 100 1200 900])
    % subplot(numRows,numCols,loopNum)
    % avgThput = mean(averages);
    % avgThput_f = mean(filtered_averages);
    % if loopNum == 4 
    %     fprintf("Loop 4 histo\n")
    % end 
    % if binLimits
    %     histogram(filtered_averages,'BinLimits',binLims_thput{loopNum},'NumBins',Nbins); %,binEdges{test}
    % else
    %     histogram(filtered_averages,Nbins); 
    % end
    % 
    % xlabel({sprintf('Throughput in kbps'),labelLetter(loopNum)}, ...
    %             'HorizontalAlignment', 'center', ...
    %             'FontWeight', 'normal'); 
    % ylabel("Counts")
    % hold on;
    % yLimits = ylim;
    % plot([avgThput_f avgThput_f], yLimits, 'r--', 'LineWidth', 2);
    % side = 'left'; 
    % % if Leo && loopNum == 4 && erSel ==3
    % %     side = 'right'; 
    % % end 
    % 
    % % Add a text label for the average
    % text(avgThput_f + 3, yLimits(2) * 0.9, sprintf('Mean: %.2f kbps', avgThput_f), ...
    %     'HorizontalAlignment', side, 'Color', 'red', 'FontSize', 12);
    % hold off;
    % if titlePrint
    %     title(subtitleText);
    % end 
    % 
    % fprintf("Filtered Average: %.4f for %s at %s Error\n",avgThput_f,folderName,errTitles(erSel));
    % 
    % % Display large Figure of Average Throughput for that run. 
    % 
    % figure('Position',[100 100 1200 900]); 
    % histogram(filtered_averages,Nbins); 
    % 
    % if binLimits
    %     histogram(filtered_averages,'BinLimits',binLims_thput{loopNum},'NumBins',Nbins);
    % end 
    % 
    % 
    % xlabel("Throughput in kbps")
    % ylabel("Counts")
    % 
    % hold on;
    % yLimits = ylim;
    % plot([avgThput_f avgThput_f], yLimits, 'r--', 'LineWidth', 2);
    % side = 'left'; 
    % if Leo && loopNum == 4 && erSel ==3
    %     side = 'right'; 
    % end 
    % 
    % % Add a text label for the average
    % text(avgThput_f + 3, yLimits(2) * 0.9, sprintf('Mean: %.2f kbps', avgThput_f), ...
    %     'HorizontalAlignment', side, 'Color', 'red', 'FontSize', 12);
    % hold off;
    % if titlePrint
    %     title(subtitleText)
    % end 
    % 
    % 
    % 
    % % title(sprintf("%s Filtered Distribution of Average Throughput Simulation: %s for %s at %s Error Rate",baseline, folderName, numTrials,errRate(erSel)))
    % if (loopNum == 3 && titlePrint)
    %     annoTextRow1_T = sprintf("%s Single Distribution of Average Throughput for %s at %s Error Rate",satConf, numTrials, errTitles(erSel)); 
    %     annotation('textbox', [0.1, 0.925, 0.8, 0.05], 'String', annoTextRow1_T, ...
    %     'EdgeColor', 'none', 'HorizontalAlignment', 'center', 'FontSize', 15);
    % end 
    % 
    % if (loopNum >5 && titlePrint)
    %     annoTextRow2_T = sprintf("%s Multi-Application Filtered Distribution of Average Throughput for %s at %s Error Rate",satConf, numTrials, errTitles(erSel)); 
    %     annotation('textbox', [0.1, 0.925/2, 0.8, 0.05], 'String', annoTextRow2_T, ...
    %     'EdgeColor', 'none', 'HorizontalAlignment', 'center', 'FontSize', 15);
    % end 
    % 
    % % sgt1 = sgtitle(sprintf("%s Filtered Distribution of Average Throughput for %s at %s Error Rate",baseline, numTrials, errTitles(erSel)));
    % % sgt1.Position(2) = sgt1.Position(2) + 0.02;
    % 
    % figurePath = '/home/drew/matlab_code/Thesis/figures/';
    % 
    % if (Leo)
    %    figurePath = strcat(figurePath,baseline,'/',satConf,'/ErrRate',errRate(erSel));
    % else 
    %     figurePath = strcat(figurePath,baseline,'/ErrRate',errRate(erSel));
    % end
    % 
    % thputHistName = strcat(figurePath,'throughput/histograms/',folderName,'/hist-',folderName,'_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
    % if (saveFig)
    %     saveas(gcf,thputHistName)
    %     fprintf("Success\n");
    % else 
    %     fprintf("Enable saveFig to Save Figures\n\n")
    % end
    % 
    % fprintf("--Standard Deviation of Throughput Averages: %.2f--\n",std(filtered_averages));
    % fprintf("--Variance of Throughput Averages %.2f--\n\n",var(filtered_averages));
    % 

    
    % %% Inter Arrival Times
    % % l = 0; 
    % interArrData = cell(1, length(Time));
    % for j = 1:length(Time)
    %     if length(Time{j}) > 1
    %         interArrData{j} = diff(Time{j});
    %     else
    %         interArrData{j} = []; % Handle cases where Time{j} has less than 2 elements
    %     end
    % end
    % disp("Done with Jitter Calculation")
    % 
    % % Calculate Average Jitter 
    % % Max interarrival Time (latency)
    % if Leo && erSel == 3
    %     fprintf("Filter out packets greater than maxKeepAlive = %d\n",maxKeepAlive); 
    %     for j=1:length(Time)
    %         validIndices = interArrData{j} <= maxKeepAlive; %filter out after 2hr keep alive time
    %         interArrData{j} = interArrData{j}(validIndices); 
    %     end 
    % end 
    % %% Calculate Jitter: 
    % 
    % for k=1:length(interArrData)
    %     avgJitter(k) = var(interArrData{k}); 
    % end 
    % 
    % filtered_avgJitter = hampel(avgJitter);
    % 
    % % if Leo && erSel == 3 
    % %     filtered_avgJitter = hampel(avgJitter,6,4);
    % % end
    % 
    % if Leo && erSel == 3
    %     fprintf("Filtering Jitter values----\n")
    %     validIndices = avgJitter < 150; 
    %     filtered_avgJitter = avgJitter(validIndices); 
    % end 
    % 
    % if loopNum == 4 && erSel ~= 3 && ~Leo 
    %     filtered_avgJitter = avgJitter; 
    % end 
    % 
    % avgJ = mean(avgJitter)*1000;
    % avgJ_f = mean(filtered_avgJitter*1000);
    % 
    % 
    % Nbins = 50; 
    % figure(3);
    % set(gcf,"Position", [100 100 1200 900])
    % subplot(numRows,numCols,loopNum)    
    % x = 0.5; % X position in normalized units
    % y = 1.03; % Y position in normalized units (above the title)
    % histogram(filtered_avgJitter*1000,'NumBins',Nbins)
    % hold on;
    % xlabel({sprintf('Time in msec'),labelLetter(loopNum)}, ...
    %             'HorizontalAlignment', 'center', ...
    %             'FontWeight', 'normal'); 
    % yLimits = ylim;
    % plot([avgJ_f avgJ_f], yLimits, 'r--', 'LineWidth', 2);
    % 
    % % Add a text label for the average
    % text(avgJ_f+0.00005, yLimits(2) * 0.9, sprintf('Mean: %.2f msec', avgJ_f), ...
    %     'HorizontalAlignment', 'left','Color', 'red', 'FontSize', 12);
    % hold off;
    % ylabel("Counts")
    % if titlePrint
    %     title(subtitleText)
    % end 
    % 
    % if (loopNum ==3 )
    %     annoTextRow1_J = sprintf("%s %s Filtered Distribution of Average Jitter for %s at %s Error Rate",satConf,baseline, numTrials, errTitles(erSel)); 
    %     annotation('textbox', [0.1, 0.925, 0.8, 0.05], 'String', annoTextRow1_J, ...
    %     'EdgeColor', 'none', 'HorizontalAlignment', 'center', 'FontSize', 15);
    % end
    % 
    % if (loopNum > 5)
    %     annoTextRow2_J = sprintf("%s %s Filtered Distribution of Average Jitter for %s at %s Error Rate",satConf,baseline, numTrials, errTitles(erSel)); 
    %     annotation('textbox', [0.1, 0.925/2, 0.8, 0.05], 'String', annoTextRow2_J, ...
    %     'EdgeColor', 'none', 'HorizontalAlignment', 'center', 'FontSize', 15);
    % end 
    % 
    % figure('Position',[100 100 1200 900])
    % hold on;
    % histogram(filtered_avgJitter*1000,'BinLimits',[0.01525, .0165],'NumBins',80)
    % % Add a text label for the average
    % 
    % % yLimits = ylim;
    % % ylim([1 max(yLimits(2), 1e3)])
    % yLimits = ylim;  % Get new limits
    % 
    % plot([avgJ_f avgJ_f], yLimits, 'r--', 'LineWidth', 2);
    % text(avgJ_f+0.0000, yLimits(2) * 0.9, sprintf('Mean: %.2f msec', avgJ_f), ...
    %     'HorizontalAlignment', 'left', 'Color', 'red', 'FontSize', 12);
    % % set(gca, 'YScale', 'log')
    % ax = gca;
    % text(ax.XLim(2)/2, ax.YLim(1)-0.1*(ax.YLim(2)-ax.YLim(1)), labelLetter(loopNum), ...
    % 'HorizontalAlignment', 'center', 'FontWeight', 'bold') 
    % hold off;
    % xlabel("Time in msec")
    % ylabel("Counts")
    % 
    % fprintf("Average Filtered Jitter: %.6fmsec\n",avgJ_f);
    % 
    % figurePath = '/home/drew/matlab_code/Thesis/figures/';
    % 
    % if (Leo)
    %    figurePath = strcat(figurePath,baseline,'/',satConf,'/ErrRate',errRate(erSel));
    % else 
    %     figurePath = strcat(figurePath,baseline,'/ErrRate',errRate(erSel));
    % end
    % 
    % if loopNum == 6
    %     jitterHistName = strcat('Jitter-',folderName,'_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
    %     fprintf("Saving %s\n",jitterHistName);
    %     jitterHistName = strcat(figurePath,'jitter/',folderName,'/Jitter-',folderName,'_',Ntrials,'_',num2str(ER(erSel)),'ER.png');
    %     if (saveFig)
    %         fprintf("Saving Figure To: %s",jitterHistName);
    %          saveas(gcf,jitterHistName);
    %          fprintf("Success\n");
    %     else 
    %         fprintf("Enable saveFig to Save Figures ")
    %     end
    % end 
    % 
    % fprintf("--Standard Deviation of Jitter Averages: %.2f--\n",std(filtered_avgJitter));
    % fprintf("--Variance of Jitter Averages %.2f--\n",var(filtered_avgJitter));   
    
    fprintf("----Done Processing %s\n",folderName);
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