# Matlab Network Server

A dead-simple library for implementing a request-response server in Matlab.

## Usage example

Write a function to implement the server.

```matlab
function response = my_server(request)
    % create a Matlab object to be returned to the client
    response.message = ['Hello ' request];
end
```

Start the server, passing a callback function.

```
>> netsrv.start_server(@my_server, 8148);
```

On another machine, send requests to the server.

```
>> netsrv.start_client('localhost', 8148)
Opening ZMQ socket tcp://localhost:8148
```

```
>> netsrv.make_request('Dave)
ans =
    message: 'Hello Dave'
```

## Features

* Can send arbitrary Matlab objects.
* Uses the high performance [ZeroMQ](http://zeromq.org/) library for messaging. The server can handle many simultaneous client connections.

## Installing

Clone the repository into a directory beginning with a `+` symbol to create a Matlab package. The suggested name is `+netsrv`

    $ git clone https://github.com/bronsonp/matlab-network-server.git +netsrv

Install the ZeroMQ library.

    $ sudo apt-get install libzmq3-dev

Compile the C++ code.

    $ cd +netsrv/private
    $ make

If the build fails, you need to [set up the MEX compiler](http://www.mathworks.com.au/help/matlab/matlab_external/building-mex-files.html).

## Credits

Serialization is provided by the `hlp_serialize` code written by Christian Kothe and published on the [Mathworks File Exchange](http://www.mathworks.com.au/matlabcentral/fileexchange/34564-fast-serializedeserialize/content/hlp_serialize.m).

Messaging is provided by the [ZeroMQ](http://zeromq.org) library.
