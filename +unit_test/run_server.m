function run_server

    try
        netsrv.start_server(@callback, 9967);
    catch E
        disp(E.getReport());
        exit();
    end

    function response = callback(request)
        switch request.msg
          case 'echo'
            response = request.data;
          otherwise
            response = 'OK';
            t = timer();
            t.StartDelay = 0.1;
            t.TimerFcn = @(t, e)exit;
            start(t);
        end
    end
end
