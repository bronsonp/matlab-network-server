function run_client

try
    netsrv.start_client('localhost', 9967);

    request = struct();
    request.msg = 'echo';
    request.data = '=====>> netsrv test passed! <<=====';
    disp(netsrv.make_request(request));
    disp(netsrv.make_request(request));
    disp(netsrv.make_request(request));

    request = struct();
    request.msg = 'quit';
    netsrv.make_request(request);

    pause(1.5); % wait for server to quit

    exit();

catch E
    disp(E.getReport());
    exit();
end



end
