#!/bin/bash

set -e

rm -rf `pwd`/build/*
list=`find ./ -name "*.proto"`
for file in ${list}
do
	path=${file%/*}
	`protoc -I=$path --cpp_out=$path $file`
done

cd `pwd`/build &&
	cmake .. &&
	make

cd ../
cp ./src/rpc_cfg.xml ./bin
