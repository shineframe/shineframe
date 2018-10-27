#!/bin/bash  
  
for((i=1;i<=3;i++));  
do   
    rm -rf $i/*.log
    cp -rf ../../build/raft_nosql_example $i/raft_nosql_example
done
