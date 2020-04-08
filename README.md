# COVID-19_filter

License
-------

COVID-19_filter is released under the terms of the MIT license. See https://opensource.org/licenses/MIT for more
information.

Compilation
-----------

You will need an up to date GCC (probably 5+) and a reasonably modern BOOST installation with the development packages
also installed. The most convenient way to compile is to use QtCreator, which is what I used to make the project, and modify
the .pro file that contains the project config to your liking.

You may be able to get this to work to create a windows exe if you have the appropriate MinGW environment, but I have
not tested that. I have been building/running it in Linux.

The executable name is COVID-19_filter.

Commentary
----------

1. You will need to clone the https://github.com/CSSEGISandData/COVID-19 twice in two separate directories,
one for the master (which updates the global and daily), and the other for the web-site tag, which does the
intraday updates, or alternately check out each branch and pull before running. That would require you to change
the update script.

2. There are two output files produced. Both are flat fact tables with csv, using " " to protect fields that have
commas within the field value itself. The two output files are filtered_global_output.csv and filtered_daily_output.csv.

3. You can specify the source and destination paths on the command line. You can get help on the command line
parameters by running COVID-19_filter -help. Note that the daily and intraday (HEAD) output is to the same file, since
the intraday file is really a up-to-the-minute version of "today's" data in daily format.

Output
------

Both output files are "" escaped (when necessary) CSV files. The filtered_global_output.csv file has the following columns:

Province/State,Country/Region,Lat,Long,status,date,number,cumulative

The filtered_daily_output file has the following columns:

FIPS,Admin2,Province_State,Country_Region,Last_Update,Date,Lat,Long,Confirmed,Deaths,Recovered,Active,Combined_Key

These files are flat fact tables that are excellent for direct importation into spreadsheets for pivot tables
or other data analysis tools.

