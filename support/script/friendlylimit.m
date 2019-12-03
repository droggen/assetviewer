%% friendlylimit
% Find human-friendly limit for plot y limits, where limits are multiples
% of ?.
%
function [yf1,yf2] = friendlylimit(y1,y2)

% yf1 = y1-1;
% yf2 = y2+1;

% Choose the spacing of ticklines with n. Spacing = power range / n
% typically n=10 (e.g. hundreds or thousands), n=5 (two hundreds or two
% thousands), n=4 (250, 2500), n=2 (500, 5000).
n=5;

%r = y2-y1;
%ts = 10^ceil(log10(r))/n;
%dr = ceil(y2/ts)*ts;
% yf1 = y1-dr;
% yf2 = y2+dr;

ts1 = 10^ceil(log10(abs(y1)))/n
ts2 = 10^ceil(log10(abs(y2)))/n
yf1 = floor(y1/ts1)*ts1
yf2 = ceil(y2/ts2)*ts2

% yf1 = y1-dr1
% yf2 = y2+dr2