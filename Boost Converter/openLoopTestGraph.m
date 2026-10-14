% Corrected data: 5, 27 and 50 ohm loads
Rload = [5, 27, 50];  % ohms

% Vin = 2.5 V
D_2p5    = [45, 65, 60];                  % %
IFB_2p5  = [195.02, 138.18, 84.28];      % peak, mV
IFBF_2p5 = [401.80, 244.02, 148.08];     % peak, mV

% Vin = 3.5 V
D_3p5    = [54, 43, 39];                  % %
IFB_3p5  = [325.50, 97.02, 64.68];       % peak, mV
IFBF_3p5 = [627.20, 168.56, 103.88];     % peak, mV

blue   = [0.00, 0.45, 0.74];
orange = [0.85, 0.33, 0.10];

figure('Color', 'w', 'Position', [200, 200, 950, 550]);
hold on;

% Solid = IFB; dashed = IFB_F
h1 = plot(Rload, IFB_2p5, '-o', ...
    'Color', blue, 'LineWidth', 2, 'MarkerSize', 8);
h2 = plot(Rload, IFBF_2p5, '--o', ...
    'Color', blue, 'LineWidth', 2, 'MarkerSize', 8);

h3 = plot(Rload, IFB_3p5, '-s', ...
    'Color', orange, 'LineWidth', 2, 'MarkerSize', 8);
h4 = plot(Rload, IFBF_3p5, '--s', ...
    'Color', orange, 'LineWidth', 2, 'MarkerSize', 8);

% Duty cycle labels: place one label beside each IFB point
for k = 1:numel(Rload)
    text(Rload(k) + 0.8, IFB_2p5(k) + 18, ...
        sprintf('D = %d%%', D_2p5(k)), ...
        'Color', blue, 'FontSize', 10);

    text(Rload(k) + 0.8, IFB_3p5(k) - 25, ...
        sprintf('D = %d%%', D_3p5(k)), ...
        'Color', orange, 'FontSize', 10);
end

xlabel('Load resistance, R_{load} (\Omega)');
ylabel('Peak voltage (mV)');
title('Measured IFB and IFB_F at Different Loads');

legend([h1, h2, h3, h4], ...
    {'IFB, Vin = 2.5 V', 'IFB_F, Vin = 2.5 V', ...
     'IFB, Vin = 3.5 V', 'IFB_F, Vin = 3.5 V'}, ...
    'Location', 'northeast');

xticks(Rload);
xlim([2, 57]);
ylim([0, 680]);
grid on;
box on;

exportgraphics(gcf, 'corrected_IFB_IFBF_vs_Rload.png', ...
    'Resolution', 300);