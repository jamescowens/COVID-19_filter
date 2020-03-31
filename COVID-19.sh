#!/bin/bash

cd /home/jco/builds/COVID-19
git pull
cd /home/jco/builds/COVID-19_web-data
git pull
cd /home/jco/builds/COVID-19_filter
./COVID-19_filter 1>> filter_run.log 2>&1
