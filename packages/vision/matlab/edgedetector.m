function lines = edgedetector( I, cfg, debug )
%EDGEDETECTOR Filter to find edges
%
%   Uses a standard hough transformation to find lines within an
%   intensity image. If debug is true, then it shows helpful graphs
%   for the calculations.
%
%   Returns up to 5 lines showing the edges in the intensity image.

% Identify defaults
if nargin > 3
    error('edgedetector takes a maximum of 3 arguments');
end

if nargin < 3
    debug = 0;
end

if nargin < 2
    cfg.minIntensity = .15;
end

% Canny edge detector
BW = edge(I, 'canny', cfg.minIntensity);
if debug
    figure('Name', 'Canny Edge Detection'), imshow(BW);
end

% Standard hough transform
[H,T,R] = hough(BW, 'ThetaResolution', 1, 'RhoResolution', 1);
P = houghpeaks(H, cfg.maxLines, 'threshold', ceil(0.3*max(H(:))));
if debug
    figure('Name', 'Hough Transform'), imshow(H,[],'XData',T,'YData',R,...
                                             'InitialMagnification','fit');
    xlabel('\theta'), ylabel('\rho');
    axis on, axis normal, hold on
    x = T(P(:,2)); y = R(P(:,1));
    plot(x,y,'s','color','red');
end

% Detect lines
lines = houghlines(BW, T, R, P);

if debug
    % Display the original image
    figure('Name', 'Hough Lines'), imshow(I);
    axis off, axis normal, hold on
    
    % Draw lines on the image
    for i=1:size(lines(:))
        xy = [lines(i).point1; lines(i).point2];
        plot(xy(:,1), xy(:,2), 'LineWidth', 2, 'Color', 'green');
    end
end

end