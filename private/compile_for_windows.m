% Compile for Windows

% Change dir
this_dir = fileparts(mfilename('fullpath'));
cd(this_dir);

% Check for the required files
requirements = {'zmq.h', 'libzmq.lib', 'libzmq.exp', 'libzmq.dll'};
libpath = 'zeromq_windows_x64';
for r = requirements'
    if 0 == exist(fullfile(libpath, r{1}), 'file')
        error('Please download ZeroMQ 4.2.0 or later and place the following files in the %s folder: %s', libpath, strjoin(requirements, ', '));
    end
end

% Compile
mexargs = {'-largeArrayDims', ['-I' libpath], ['-L' libpath], '-lzmq'};

% Copy the DLL to the root directory so it can be found by the mex file
copyfile(fullfile(libpath, 'libzmq.dll'), this_dir);

mex( mexargs{:}, 'client_communicate.cpp' );
mex( mexargs{:}, 'server_communicate.cpp' );

disp('Done.');
