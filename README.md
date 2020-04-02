# COVID-19_filter

License
-------

COVID-19_filter is released under the terms of the MIT license. See https://opensource.org/licenses/MIT for more
information.

Commentary
----------

1. The paths in the source code and the script are hard-coded for my environment at this point. I plan on changing
that soon, but until then you will have to adapt both the source code and the script for your use.

2. You will need to clone the https://github.com/CSSEGISandData/COVID-19 twice in two separate directories,
one for the master (which updates the global and daily), and the other for the web-site tag, which does the
intraday updates, or alternately check out each branch and pull before running. That would require you to change
the update script.

3. There are two output files produced. Both are flat fact tables with csv, using " " to protect fields that have
commas within the field value itself.

The two output files are filtered_global_output.csv and filtered_daily_output.csv.

Compilation
-----------

You will need an up to date GCC (probably 5+) and BOOST you will need to modify the make file to point to the correct
boost locations. The most convenient way is to use QtCreator, which is what I used to make the project, and modify
the .pro file that contains the project config.

You may be able to get this to work to create a windows exe if you have the appropriate MinGW environment, but I have
not tested that. I have been building/running it in Linux.
