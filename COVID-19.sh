#!/bin/bash

source /home/jco/builds/COVID-19_filter/setexecenv_vars.dat

cd /home/jco/builds/COVID-19
git pull

cd /home/jco/builds/COVID-19_web-data
git pull

cd /home/jco/builds/COVID-19_filter
./COVID-19_filter 2>&1 | tee -a ./filter_run.log
