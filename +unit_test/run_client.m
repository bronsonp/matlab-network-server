function run_client

try
    netsrv.start_client('localhost', 9967);

    request = struct();
    request.msg = 'echo';
    request.data = '=====>> netsrv test passed! <<=====';
    disp(netsrv.make_request(request));
    disp(netsrv.make_request(request));
    disp(netsrv.make_request(request));
    
    obj = netsrv.unit_test.HardToSerialise();
    request.data = obj;
    response = netsrv.make_request(request);
    if response.number == obj.number
        disp('=====>> class serialisation test passed! <<=====');
    else
        disp('=====>> class serialisation test FAILED! <<=====');
    end

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
