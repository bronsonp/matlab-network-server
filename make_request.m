function response = make_request(request)
% ZMQ.MAKE_REQUEST(request) sends the supplied request data to the
% previously opened client socket.

    if nargin < 1
        error(sprintf(['Usage:\n'...
                       'make_request(request)']));
    end

    % Constants
    CLIENT_INIT = uint32(0);
    CLIENT_REQUEST = uint32(1);

    % Serialise the message
    request_s = hlp_serialize(request);

    % Send the message
    [success, response_s] = client_communicate(CLIENT_REQUEST, request_s);
    if ~success
        error('zmq:failed_to_communicate', 'ZMQ: Failed to send message to server.');
    end

    % Deserialise
    response = hlp_deserialize(response_s);

end
