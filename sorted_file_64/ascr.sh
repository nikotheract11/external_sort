#!/bin/bash
rm unsorted_data.db temp
make $1
./build/sr_main1
./build/sr_main2
