#!/bin/bash

die () {
    echo >&2 "$@"
    exit 1
}

if [ ! -d "+netsrv" ]; then
    die "Run this script from the top level of the project where the +netsrv directory is"
fi

matlab -nosplash -nodisplay -r 'netsrv.unit_test.run_server;exit' &
matlab -nosplash -nodisplay -r 'netsrv.unit_test.run_client;exit'
