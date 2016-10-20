% Compile for Windows

% Change dir
this_dir = fileparts(mfilename('fullpath'));
cd(this_dir);

% Download ZeroMQ 3.2.4 from http://zeromq.org/distro:microsoft-windows and
% install it
ZMQ_DIR='C:\Program Files\ZeroMQ 3.2.4';

% Copy DLL locally
copyfile(sprintf('%s\\bin\\libzmq-v90-mt-3_2_4.dll', ZMQ_DIR), this_dir);
copyfile(sprintf('%s\\lib\\libzmq-v90-mt-3_2_4.lib', ZMQ_DIR), this_dir);

% Compile
mexargs = {...
    sprintf('-I%s\\include', ZMQ_DIR), ...
    sprintf('-L%s\\lib', ZMQ_DIR), ...
    '-lzmq-v90-mt-3_2_4'};

mex( mexargs{:}, 'client_communicate.cpp' );
mex( mexargs{:}, 'server_communicate.cpp' );

disp('Done.');