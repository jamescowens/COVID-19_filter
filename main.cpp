#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/newline.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/chrono.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"
#include "boost/date_time/posix_time/posix_time.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <map>
#include <algorithm>
#include<bits/stdc++.h>

using namespace std;

namespace fs = boost::filesystem;
namespace boostio = boost::iostreams;

bool fDebug = false;

/* Utility functions */

void PrintToConsole(std::string string)
{
    boost::posix_time::ptime datetime(boost::posix_time::microsec_clock::universal_time());

    cout << boost::posix_time::to_iso_extended_string(datetime) << " " << string << endl;
}

std::vector<std::string> split(const std::string& s, const std::string& delim, const std::string& delim_protect = "")
{
    size_t pos = 0;
    size_t end = 0;
    std::vector<std::string> elems;

    while(end != std::string::npos)
    {
        size_t next_delim = s.find(delim, pos);
        size_t next_delim_protect = s.find(delim_protect, pos);

        // If the position of the delimiter escape "protect" character is before
        // the next delimiter, then we are in an escaped sequence.
        if (next_delim_protect < next_delim)
        {
            // Advance the end to the next matching protect character.
            end = s.find(delim_protect, next_delim_protect + delim_protect.size());

            // Advance from the matching (closing) protect character to the next delimiter.
            // This should be the next character, but there could be white space.
            end = s.find(delim, end);
        }
        else
        {
            end = next_delim;
        }

        elems.push_back(s.substr(pos, end - pos));
        pos = end + delim.size();
    }

    // Append final value
    //elems.push_back(s.substr(pos, end - pos));
    return elems;
}

constexpr char ToLower(char c)
{
    return (c >= 'A' && c <= 'Z' ? (c - 'A') + 'a' : c);
}

std::string ToLower(const std::string& str)
{
    std::string r;
    for (auto ch : str) r += ToLower((unsigned char)ch);
    return r;
}


class daily_data_record
{

public:
    static std::vector<boost::gregorian::date> blacklist;

    static std::vector<std::string> header;

    unsigned int source_version;

    std::string FIPS;
    std::string Admin2;
    std::string Province_State;
    std::string Country_Region;
    boost::posix_time::ptime Last_Update;
    boost::gregorian::date Date;
    double Lat;
    double Long;
    int Confirmed;
    int Deaths;
    int Recovered;
    int Active;
    std::string Combined_Key;

    struct Key
    {
        std::string FIPS;
        std::string Admin2;
        std::string Province_State;
        std::string Country_Region;
        boost::gregorian::date Date;
    };

    struct KeyComp
    {
        bool operator()(const Key& key_left, const Key& key_right)
        {
            std::string left, right;

            left = key_left.FIPS
                   + key_left.Admin2
                   + key_left.Province_State
                   + key_left.Country_Region
                   + boost::gregorian::to_iso_string(key_left.Date);

            right = key_right.FIPS
                     + key_right.Admin2
                     + key_right.Province_State
                     + key_right.Country_Region
                     + boost::gregorian::to_iso_string(key_right.Date);

            return left < right;
        }
    };

    static std::map<Key, daily_data_record, KeyComp> map_daily_data_record;

    // Empty constructor for daily_data_record
    daily_data_record() {}

    // Normal constructor for daily_data_record which loads the content of the read text line into the variables.
    daily_data_record(boost::gregorian::date file_date, std::string input_line)
    {
        source_version = GetVersion(file_date);

        if (fDebug) PrintToConsole("INFO: Version = " + std::to_string(source_version));

        std::vector<std::string> v_line = split(input_line, ",", "\"");

        if (fDebug) PrintToConsole("INFO: v_line.size() = " + std::to_string(v_line.size()));

        std::vector<std::string> v_datetime;
        std::vector<std::string> v_date;
        std::string time;
        std::string datetime;
        std::string century;

        switch (source_version)
        {
        case 0:
            Province_State = v_line[0];
            Country_Region = v_line[1];

            v_datetime = split(v_line[2], " ");

            if (fDebug) PrintToConsole("INFO: v_datetime[0] = " + v_datetime[0]);
            if (fDebug) PrintToConsole("INFO: v_datetime[1] = " + v_datetime[1]);

            // The date for version 0 has slashes.
            v_date = split(v_datetime[0], "/");

            // Change the date order to yyyy-m-d and add time.

            if (v_date[2].size() < 4) century = "20";
            datetime = century + v_date[2] +"-" + v_date[0] + "-" + v_date[1] + " " + v_datetime[1];

            if (fDebug) PrintToConsole("INFO: datetime = " + datetime);

            Last_Update = boost::posix_time::time_from_string(datetime);

            Date = Last_Update.date();

            Confirmed = StringToInt(v_line[3]);
            Deaths = StringToInt(v_line[4]);
            Recovered = StringToInt(v_line[5]);

            break;

        case 2:
            Lat = StringToDouble(v_line[6]);
            Long = StringToDouble(v_line[7]);

            // No break here on purpose, because version 2 is a superset of version 1 and the date has the same version.
            [[fallthrough]];

        case 1:
            Province_State = v_line[0];
            Country_Region = v_line[1];

            v_datetime = split(v_line[2], "T");

            if (fDebug) PrintToConsole("INFO: v_datetime[0] = " + v_datetime[0]);

            // The date is already in the right order.
            datetime = v_datetime[0] + " " + v_datetime[1];

            if (fDebug) PrintToConsole("INFO: datetime = " + datetime);

            Last_Update = boost::posix_time::time_from_string(datetime);

            Date = Last_Update.date();

            Confirmed = StringToInt(v_line[3]);
            Deaths = StringToInt(v_line[4]);
            Recovered = StringToInt(v_line[5]);

            break;

        case 3:
            FIPS = v_line[0];
            Admin2 = v_line[1];
            Province_State = v_line[2];
            Country_Region = v_line[3];

            v_datetime = split(v_line[4], " ");

            if (fDebug) PrintToConsole("INFO: v_datetime[0] = " + v_datetime[0]);
            if (fDebug) PrintToConsole("INFO: v_datetime[1] = " + v_datetime[1]);

            // The date for version 0 has slashes.
            v_date = split(v_datetime[0], "/");

            // Change the date order to yyyy-m-d and add time.

            if (v_date[2].size() < 4) century = "20";
            datetime = century + v_date[2] +"-" + v_date[0] + "-" + v_date[1] + " " + v_datetime[1];

            if (fDebug) PrintToConsole("INFO: datetime = " + datetime);

            Last_Update = boost::posix_time::time_from_string(datetime);
            Date = Last_Update.date();

            Lat = StringToDouble(v_line[5]);
            Long = StringToDouble(v_line[6]);
            Confirmed = StringToInt(v_line[7]);
            Deaths = StringToInt(v_line[8]);
            Recovered = StringToInt(v_line[9]);
            Active = StringToInt(v_line[10]);

            Combined_Key = v_line[11];

            break;

        case 4:
            FIPS = v_line[0];
            Admin2 = v_line[1];
            Province_State = v_line[2];
            Country_Region = v_line[3];

            v_datetime = split(v_line[4], " ");

            // The date is already in the right order.
            datetime = v_datetime[0] + " " + v_datetime[1];

            if (fDebug) PrintToConsole("INFO: datetime = " + datetime);

            Last_Update = boost::posix_time::time_from_string(datetime);
            Date = Last_Update.date();

            Lat = StringToDouble(v_line[5]);
            Long = StringToDouble(v_line[6]);
            Confirmed = StringToInt(v_line[7]);
            Deaths = StringToInt(v_line[8]);
            Recovered = StringToInt(v_line[9]);
            Active = StringToInt(v_line[10]);

            Combined_Key = v_line[11];
        }

    }

    int StringToInt(std::string& input)
    {
        int result = 0;

        if (!input.empty())
        {
            try
            {
                result = std::stoi(input);
            }
            catch (...)
            {
                PrintToConsole("ERROR: Bad value in converting string to integer.");
            }
        }

        return result;
    }

    double StringToDouble(std::string& input)
    {
        double result = 0.0;

        if (!input.empty())
        {
            try
            {
                result = std::stod(input);
            }
            catch (...)
            {
                PrintToConsole("ERROR: Bad value in converting string to integer.");
            }
        }

        return result;
    }


    unsigned int GetVersion(boost::gregorian::date file_date)
    {
        // Initial version is 0.
        unsigned int version = 0;

        if (file_date >= boost::gregorian::date(2020,2,2))
        {
            version = 1;
        }

        if (file_date >= boost::gregorian::date(2020,3,1))
        {
            version = 2;
        }

        if (file_date >= boost::gregorian::date(2020,3,22))
        {
            version = 3;
        }

        if (file_date >= boost::gregorian::date(2020,3,23))
        {
            version = 4;
        }

        return version;
    }

    bool ToString(std::string separator, std::string& output_string)
    {
        try
        {
            output_string = FIPS + separator
                    + Admin2 + separator
                    + Province_State + separator
                    + Country_Region + separator
                    + boost::posix_time::to_iso_extended_string(Last_Update) + separator
                    + boost::gregorian::to_iso_extended_string(Date) + separator
                    + std::to_string(Lat) + separator
                    + std::to_string(Long) + separator
                    + std::to_string(Confirmed) + separator
                    + std::to_string(Deaths) + separator
                    + std::to_string(Recovered) + separator
                    + std::to_string(Active) + separator
                    + Combined_Key;
        }
        catch (...)
        {
            PrintToConsole("ERROR streaming daily_date_record to string.");
            return false;
        }

        return true;
    }

    Key GetKey()
    {
        Key key;

        key.FIPS = FIPS;
        key.Admin2 = Admin2;
        key.Country_Region = Country_Region;
        key.Province_State = Province_State;
        key.Date = Date;

        return key;
    }

};

// Static initialization
std::vector<std::string> daily_data_record::header = {"FIPS",
                                                      "Admin2",
                                                      "Province_State",
                                                      "Country_Region",
                                                      "Last_Update",
                                                      "Date",
                                                      "Lat",
                                                      "Long",
                                                      "Confirmed",
                                                      "Deaths",
                                                      "Recovered",
                                                      "Active",
                                                      "Combined_Key"};

std::vector<boost::gregorian::date> daily_data_record::blacklist = {boost::gregorian::date(2020,3,11),
                                                                   boost::gregorian::date(2020,3,13)};

std::map<daily_data_record::Key, daily_data_record, daily_data_record::KeyComp> daily_data_record::map_daily_data_record {};


// This function is a bit weird, because for the "global" file type entry, it writes out immediately to the destination file,
// where for the "daily" file type entry it puts its output into a map unless output map to file is specified. This is
// inelegant but it works.
bool ProcessSourceFile(fs::path source_file, std::pair<std::string, std::string> source_file_type_entry, fs::path destination_file, bool output_map_to_file, bool append = true)
{
    if (!append && fs::exists(destination_file)) fs::remove(destination_file);

    fs::ifstream infile(source_file, std::ios_base::in |std::ios_base::binary);

    // Convert file line endings to POSIX standard line endings to avoid problems.
    boost::iostreams::filtering_istream filtered_infile;
    filtered_infile.push(boost::iostreams::newline_filter(boost::iostreams::newline::posix));
    filtered_infile.push(infile);

    fs::ofstream outfile(destination_file, std::ios_base::out | std::ios_base::app);

    if ((!output_map_to_file && !infile) || (output_map_to_file && !outfile))
    {
        PrintToConsole("ERROR: Input or output file error.");
        return false;
    }

    // We can stream directly to the outfile
    auto& stream = outfile;

    std::string line;

    if (infile)
    {
        // Get header
        std::getline(filtered_infile, line);

        if (fDebug) PrintToConsole("INFO: line = " + line);

        // From here on out we have to differentiate by file type entry
        // The global file type can be processed directly from input to output
        // with only minimal transformation because it has one version and
        // is unitary, so we don't need a fancy object.
        if (source_file_type_entry.first == "global")
        {
            std::vector<std::string> header = split(line, ",", "\"");

            unsigned int base_dimension_num_columns = 4;

            // Put header base dimension column names in the output.
            for (unsigned int i = 0; i < base_dimension_num_columns; ++i)
            {
                stream << header[i] << ",";
            }

            // We add status, date, and number
            stream << "status,date,number,cumulative" << endl;

            // Now the data
            while (std::getline(filtered_infile, line))
            {
                if (fDebug) PrintToConsole("INFO: line = " + line);

                std::vector<std::string> vline = split(line, ",", "\"");

                if (fDebug) PrintToConsole("INFO: Number of fields = " + std::to_string(vline.size()));

                // The nature of these files is that the base dimension fields need to be
                // repeated for EACH successive field value (for the dates), because the
                // source data has these series as columns rather than rows, which is not
                // optimal for ingestion. This converts the columns into rows for
                // base_dimension_num_columns onward, while repeating the base_dimension
                // columns each time. It is easier here to use old style element references.

                // The outer loop is for EACH date field in the row, now to be written out as
                // as separate row for each date.

                int prev_total = 0;

                for (unsigned int i = base_dimension_num_columns; i < vline.size(); ++i)
                {
                    // Write out the base dimension fields (repeated each time).
                    for (unsigned int j = 0; j < base_dimension_num_columns; ++j)
                    {
                        stream << vline[j] << ",";
                    }

                    // The source_file_type is now a field too.
                    stream << source_file_type_entry.second << ",";

                    // Pick up the date from the header and write it out.
                    // If the index is beyond the header size, then abort processing.
                    if (i >= header.size()) break;
                    stream << header[i] << ",";

                    // Now write out the value (number) associated with the date.
                    // We want the difference for the daily values.

                    int new_total;
                    try
                    {
                        new_total = std::stoi(vline[i]);
                    }
                    catch (...)
                    {
                        continue;
                    }

                    int value = new_total - prev_total;
                    stream << value << "," << vline[i] << endl;

                    // Update the prev total for the next difference.
                    prev_total = new_total;
                }
            }

            outfile.flush();
            outfile.close();
            infile.close();

            return true;
        }
        // This part is going to make heavy use of the daily_data_record object.
        else if (source_file_type_entry.first == "daily")
        {
            // The daily files are in the form of mm-dd-yyyy
            int year;
            int month;
            int day;

            try
            {
                std::vector<std::string> v_date = split(source_file.filename().stem().string(), "-");
                year = std::stoi(v_date[2]);
                month = std::stoi(v_date[0]);
                day = std::stoi(v_date[1]);
            }
            catch (...)
            {
                PrintToConsole("ERROR: Date parse from filename invalid.");
                return false;
            }

            boost::gregorian::date date(year, month, day);

            // We throw the original header in the source file away, because several versions
            // are being merged into a single structure.
            // Now the data...
            while (std::getline(filtered_infile, line))
            {
                if (fDebug) PrintToConsole("INFO: line = " + line);

                // Form the record object from the data file input line. Note
                // that the versioning is implicitely determined by the fed in date from
                // the filename.
                daily_data_record record(date, line);

                // Insert into the global map. Using the [] operator, and having the key have
                //  the Date member, ensures that when there are multiple records on the same day,
                // the latest record will replace the earlier for the same geographic combination,
                // which is what we want.
                daily_data_record::map_daily_data_record[record.GetKey()] = record;
            }

            PrintToConsole("INFO: map_daily_data_record.size() = " + std::to_string(daily_data_record::map_daily_data_record.size()));

            return true;
        }

    }

    // If output_map_to_file is true, this indicates the map is completely loaded and
    // should be written to the output file.
    if (output_map_to_file)
    {
        // Write out header.
        {
            std::string output_string;

            for (const auto& element : daily_data_record::header)
            {
                std::string separator;

                if(!output_string.empty()) separator = ",";

                output_string += separator + element;
            }

            outfile << output_string;
        }

        outfile << endl;

        // Write out data.
        for (auto& row : daily_data_record::map_daily_data_record)
        {
            std::string output_string;
            if (row.second.ToString(",", output_string))
            {
                if (fDebug) PrintToConsole("INFO: output string = " + output_string);
                outfile << output_string << endl;
            }
            else
            {
                PrintToConsole("ERROR: Problem dumping row from map to output file.");
            }
        }

        outfile.flush();
        outfile.close();
        infile.close();
    }

    return true;

}

int main()
{
    int error = false;

    fs::path source_path;
    fs::path destination_path;
    fs::path data_dir = "/home/jco/builds/COVID-19/csse_covid_19_data/";

    std::map<std::string, std::pair<std::string, std::string>> source_file_spec;
    std::map<std::string, std::pair<std::string, std::string>> destination_file_spec;
    std::vector<std::pair<std::string, std::string>> source_file_type;

    source_file_spec["global"] = std::make_pair("csse_covid_19_time_series/time_series_covid19_", "_global.csv");
    source_file_spec["daily"] = std::make_pair("csse_covid_19_daily_reports", ".csv");

    destination_file_spec["global"] = std::make_pair("csse_covid_19_time_series/filtered_", "_output.csv");
    destination_file_spec["daily"] = std::make_pair("csse_covid_19_daily_reports/filtered_", "_output.csv");

    // A little kludgy. The global files are three different categories,
    // while the daily files are one per day and taken care of with an
    // iterator.
    source_file_type.push_back(std::make_pair("global", "confirmed"));
    source_file_type.push_back(std::make_pair("global", "deaths"));
    source_file_type.push_back(std::make_pair("global", "recovered"));

    source_file_type.push_back(std::make_pair("daily", ""));

    if (fDebug) PrintToConsole("INFO: source file type count = " + std::to_string(source_file_type.size()));

    // Start with a fresh file at first.
    bool append = false;


    for (const auto& entry : source_file_type)
    {

        auto source_file_spec_entry = source_file_spec.find(entry.first);

        if (source_file_spec_entry == source_file_spec.end())
        {
            PrintToConsole("ERROR: Invalid source file spec, aborting.");
            return 1;
        }

        auto destination_file_spec_entry = destination_file_spec.find(entry.first);

        if (destination_file_spec_entry == destination_file_spec.end())
        {
            PrintToConsole("ERROR: Invalid destination file spec, aborting.");
            return 1;
        }

        destination_path = data_dir / (fs::path) (destination_file_spec_entry->second.first + entry.first + destination_file_spec_entry->second.second);

        if (entry.first == "global")
        {
            source_path = data_dir / (fs::path) (source_file_spec_entry->second.first + entry.second + source_file_spec_entry->second.second);

            PrintToConsole("INFO: Processing source " + source_path.filename().string()
                           + " to destination " + destination_path.filename().string());

            error = !ProcessSourceFile(source_path, entry, destination_path, false, append);

            // Append for each additional pass.
            append = true;
        }
        else if (entry.first == "daily")
        {
            // Set source path to the directory to be iterated through for the daily files
            source_path = data_dir / source_file_spec_entry->second.first;

            // We need to order the files properly
            std::map<boost::gregorian::date, fs::directory_entry> sorted_directory_map;

            // Iterate and populate map for proper sort.
            for (fs::directory_entry& dir_entry : fs::directory_iterator(source_path))
            {
                // Only process the csv files and ignore the output file itself.
                std::string extension = ToLower(dir_entry.path().extension().string());

                if (extension != ".csv" || dir_entry == destination_path)
                {
                    PrintToConsole("INFO: skipping " + dir_entry.path().filename().string());
                    continue;
                }

                // The daily files are in the form of mm-dd-yyyy
                int year;
                int month;
                int day;

                try
                {
                    std::vector<std::string> v_date = split(dir_entry.path().filename().stem().string(), "-");
                    year = std::stoi(v_date[2]);
                    month = std::stoi(v_date[0]);
                    day = std::stoi(v_date[1]);
                }
                catch (...)
                {
                    PrintToConsole("ERROR: Date parse from filename invalid.");
                    error = true;
                    continue;
                }

                boost::gregorian::date date(year, month, day);

                // Do not process daily files on the blacklist.
                if (std::find(daily_data_record::blacklist.begin(),daily_data_record::blacklist.end(), date)
                        != daily_data_record::blacklist.end())
                {
                    continue;
                }

                sorted_directory_map[date] = dir_entry;
            }

            // Iterate
            for (const auto& dir_entry : sorted_directory_map)
            {
                PrintToConsole("INFO: Processing source " + dir_entry.second.path().filename().string()
                               + " to map");

                // If a file fails to process, flag the error and continue to iterate.
                error = error || !ProcessSourceFile(dir_entry.second.path(), entry, destination_path, false);
            }

            PrintToConsole("INFO: Processing map to destination " + destination_path.filename().string());

            // Write the map to file
            error = error || !ProcessSourceFile(fs::path {}, entry, destination_path, true, false);
        }
    }

    if (!error)
    {
        PrintToConsole("INFO: Execution successful.\n" + std::to_string(error));
    }
    else
    {
        PrintToConsole("ERROR: Execution failed.\n" + std::to_string(error));
    }

    return error;
}
