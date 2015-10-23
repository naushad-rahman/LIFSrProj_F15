Vin = [   4.4,      5,    5.5,      6,    6.5,      7,     7.1,   7.5,       8,    8.5,    9,     9,  9.5,     10,   10.5,   10.5,      11,      11,      11,    11.5,    11.9, 12];
Vlas= [  1.74,   1.76,   1.78,    1.8,   1.82,   1.83,    1.83,  1.85,    1.86,   1.88, 1.89,  1.89, 1.91,   1.93,    2.2,   1.94,     2.6,       2,    1.97,     2.2,    2.10, 2.82];
Po  = [.05e-6, .09e-6, .14e-6, .16e-6, .19e-6, .61e-6, 1.34e-6, 16e-6, 37.7e-6, 200e-6, 1e-3, .7e-3, 2e-3, 2.9e-3, 3.4e-3, 1.9e-3,  3.3e-3, 2.66e-3, 2.50e-3, 3.09e-3, 3.12e-3, 2.96e-3];
R = 50;
I = (Vin - Vlas)./R;

figure(1);
plot(Vin, Po);
title('Laser Characterization');
xlabel('Vin');
ylabel('Optical Power');

figure(2);
plot(I, Po);
title('Laser Characterization');
xlabel('Iin');
ylabel('Optical Power');