#!/bin/bash

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"/usr/local/lib64" 
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"../lib" 

jupyter notebook --allow-root --no-browser --ip="0.0.0.0"
