function [ y, u, v ] = yuyv_to_yuv( filename, interleave )
%YUYV_TO_YUV Summary of this function goes here
%   Detailed explanation goes here

% Read raw sensor data
fp = fopen(filename, 'rb');
A = fread(fp, '*uint8');
fclose(fp);

if interleave == 0
    % Decode and reinterleave pixels into planes (more standard)
    % Can be viewed with "ffplay -i <outfile> -s 1280x1024"
    %result = reshape(A(1:2:end), 1280, []);  % Brightness component
    y = A(1:2:end); numel(y)  % Debug watch: image(reshape(y, 1280, []))
    u = A(2:4:end); numel(u)
    v = A(4:4:end); numel(v)

    % Write planes
    fp = fopen(strcat(filename, '.yuv'), 'wb');
    fwrite(fp, y, '*uint8');
    fwrite(fp, u, '*uint8');
    fwrite(fp, v, '*uint8');
    fclose(fp);
elseif interleave == 1
    stripes = reshape(A, 4, []);  % YUYV columns
    A = stripes([1,3,2,4], :);  % Convert into YYUV
    
    fp = fopen(strcat(filename, '.yuv'), 'wb');
    fwrite(fp, reshape(A, 1, []), '*uint8');
    fclose(fp);
elseif interleave == 2
    % NV12 format
    y  = A(1:2:end);
    uv = A(2:2:end);

    % Write planes
    fp = fopen(strcat(filename, '.yuv'), 'wb');
    fwrite(fp, y, '*uint8');
    fwrite(fp, uv, '*uint8');
    fclose(fp);
end

end
