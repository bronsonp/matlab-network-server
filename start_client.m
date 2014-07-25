function start_client(server, port, timeout)
% START_CLIENT open a connection to a ZMQ server.
%
% START_CLIENT(server) connects to the specified server on port 8148.
%
% START_CLIENT(server, port) connects to the specified server on the requested port.
%
% START_CLIENT(..., timeout) additionally specifies the timeout in milliseconds
% after which an error will be raised in the case of communication failure.
%

    if nargin < 3
        timeout = 10000; % milliseconds to wait before returning to MATLAB
                         % in the case of communication failure.
    end
    if nargin < 2
        port = 8148;
    end
    if nargin < 1
        error(sprintf(['Usage:\n'...
                       'start_client(server)\n'...
                       'start_client(server, port)']));
    end

    % Prepare the config structure
    config = struct();
    config.endpoint = sprintf('tcp://%s:%i', server, port);
    config.timeout = uint32(timeout); 

    % Constants
    CLIENT_INIT = uint32(0);
    CLIENT_REQUEST = uint32(1);

    % Call the mex function that wraps ZMQ
    try
        [success, ~] = client_communicate(CLIENT_INIT, config);
    catch E
        if strcmp(E.identifier, 'MATLAB:UndefinedFunction')
            error('You need to compile the MEX files in +netsrv/private');
        else
            rethrow(E);
        end
    end
    if ~success
        error('netsrv:init_failed', 'Netsrv: Failed to initialise client.');
    end

end
