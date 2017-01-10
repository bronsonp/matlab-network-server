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
>> netsrv.make_request('Dave')
ans =
    message: 'Hello Dave'
```

## Features

* Can send arbitrary Matlab objects.
* Uses the high performance [ZeroMQ](http://zeromq.org/) library for messaging. The server can handle many simultaneous client connections. Note however that Matlab's event loop is single threaded. The callback function will not be executed multiple times in parallel, so it should return quickly.

## Installing

**It is recommended to clone this repository into a directory called `+netsrv`.** This path is assumed by the unit testing shell script.

The recommended way to install is to add this as a git subtree to your existing project.

    $ git remote add -f matlab-network-server https://github.com/bronsonp/matlab-network-server.git
    $ git subtree add --prefix +netsrv matlab-network-server master

At a later time, if there are updates released that you wish to add to your project:

    $ git fetch matlab-network-server
    $ git subtree pull --prefix +netsrv matlab-network-server master

If you do not intend to use git subtree, you can simply clone the repository:

    $ git clone https://github.com/bronsonp/matlab-network-server.git +netsrv

### Compiling (Linux)

Install the ZeroMQ library (version 4.2.0 or later)

    $ sudo apt-get install libzmq3-dev

Compile the C++ code.

    $ cd +netsrv/private
    $ make

If the build fails, you need to [set up the MEX compiler](http://www.mathworks.com.au/help/matlab/matlab_external/building-mex-files.html).

### Compiling (Windows)

1. Run the `compile_for_windows.m` script in the `private` subdirectory.

A precompiled copy of ZeroMQ 4.2.0 is included in this repository. If you have issues, you might need to compile ZeroMQ from source yourself.

## A note on serialisation

At the time of writing, Matlab does not provide an official method to serialise
objects other than the `save` and `load`  functions (which require a round-trip
to the filesystem). Therefore, this code uses the [undocumented Matlab
functions](http://undocumentedmatlab.com/blog/serializing-deserializing-matlab-data)
`getByteStreamFromArray` and `getArrayFromByteStream`. The advantage of these
built-in functions over a third-party serialisation library is that they can
handle Matlab classes with private properties that would be invisible to third
party code.

The problem arises when serialising a class with a private property. A third
party serialisation library cannot access the private property and so cannot
fully inspect the state of the class. The workaround is that all user-defined
classes must implement the `saveobj` and `loadobj` methods to save and restore
all properties. However, this results in fragile code since any new properties
added to the class must also be added to `saveobj` and `loadobj`. It's also
terribly boring boilerplate code. There's no automated way to test whether
`saveobj` and `loadobj` are working correctly, so we resort to undocumented
Matlab functions for improved reliability.

See the provided `unit_test` package for an example of a Matlab class that
requires language support for proper serialisation.

## Credits

Messaging is provided by the [ZeroMQ](http://zeromq.org) library.
