#!/bin/bash

libOK=`export | grep LD_LIBRARY_PATH | grep "/usr/local/lib64"`
if [ "$libOK" == "" ]; then
    export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"/usr/local/lib64" 
fi
