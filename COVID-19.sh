#!/bin/bash

source /home/jco/builds/COVID-19_filter/setexecenv_vars.dat

cd /home/jco/builds/COVID-19
git pull 2>&1 | tee -a /home/jco/builds/COVID-19_filter/filter_run.log

cd /home/jco/builds/COVID-19_web-data
git pull 2>&1 | tee -a /home/jco/builds/COVID-19_filter/filter_run.log

cd /home/jco/builds/COVID-19_filter
./COVID-19_filter 2>&1 | tee -a /home/jco/builds/COVID-19_filter/filter_run.log
