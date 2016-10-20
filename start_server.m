function start_server(request_callback, port)
% START_SERVER Start the network server.
%
% START_SERVER(request_callback) starts a server on TCP port 8148.
% Requests are handled by the supplied callback function.
%
% START_SERVER(request_callback, port) starts a server on the specified port.

    if nargin < 2
        port = 8148;
    end
    if nargin < 1
        error(sprintf(['Usage:\n'...
                       'start_server(request_callback)\n'...
                       'start_server(request_callback, port)']));
    end

    fprintf('Starting server on port %i ...\n', port);

    % Prepare the config structure
    config = struct();
    config.port = uint16(port);
    config.timeout = uint32(1000); % milliseconds to wait before
                                   % returning to MATLAB

    % Constants
    SERVER_INIT = uint32(0);
    SERVER_RECV = uint32(1);
    SERVER_SEND = uint32(2);

    % Call the mex function that wraps ZMQ
    try
        [success, ~] = server_communicate(SERVER_INIT, config);
    catch E
        if strcmp(E.identifier, 'MATLAB:UndefinedFunction')
            error('You need to compile the MEX files in +netsrv/private');
        else
            rethrow(E);
        end
    end
    if ~success
        error('netsrv:init_failed', 'Netsrv: Failed to initialise server.');
    end

    while true
        % This call periodically returns to MATLAB so that the
        % program can be interrupted with ctrl+c
        [have_msg, msg_serialised] = server_communicate(SERVER_RECV, config);

        % Did we receive a message?
        if have_msg
            % Deserialise
            msg = getArrayFromByteStream(msg_serialised);

            % Run the callback
            response = request_callback(msg);

            % Serialise
            response_serialised = getByteStreamFromArray(response);

            % Send the response
            [~, ~] = server_communicate(SERVER_SEND, response_serialised);
        end
    end
end
