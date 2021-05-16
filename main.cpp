// ========================================================================================== //
//   Program for METAR and TAF database receiving from Aviation Weather Center                //
//   https://aviationweather.gov/adds/dataserver_current/current/ for departure airport ICAO, //
//   arriving airport ICAO and radius zone near flight path with follow TAFS and METAR data   //
//   .csv files parsing                                                                       //
//                                                             Barracuda_marina, 16.05.2021   //
//                                                                            ver. 0.9 beta   //
// ========================================================================================== //


#include "METAF.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include "curl/curl.h"

#pragma comment(lib,"curllib-bcb.lib")

using namespace std;
using namespace metaf;

// --------------------------------------------------------------------------------------------

char ap_departure[5] = "UKOO";
char ap_arriving[5] = "UKBB";
char search_radius[3] = "50";
char hours_before_now[3] = "2";

//---------------------------------------------------------------------------
size_t write_data( char *ptr, size_t size, size_t nmemb, FILE* data)
{
    return fwrite(ptr, size, nmemb, data);
}
//---------------------------------------------------------------------------

int test_input(void);

//---------------------------------------------------------------------------

string errorMessage(ReportError error) {
    switch (error) {
        case ReportError::NONE:
        return "no error, parsed succesfully";

        case ReportError::EMPTY_REPORT:
        return "empty report";

        case ReportError::EXPECTED_REPORT_TYPE_OR_LOCATION:
        return "expected report type or ICAO location";

        case ReportError::EXPECTED_LOCATION:
        return "expected ICAO location";

        case ReportError::EXPECTED_REPORT_TIME:
        return "expected report time";

        case ReportError::EXPECTED_TIME_SPAN:
        return "expected time span";

        case ReportError::UNEXPECTED_REPORT_END:
        return "unexpected report end";

        case ReportError::UNEXPECTED_GROUP_AFTER_NIL:
        return "unexpected group after NIL";

        case ReportError::UNEXPECTED_GROUP_AFTER_CNL:
        return "unexpected group after CNL";

        case ReportError::UNEXPECTED_NIL_OR_CNL_IN_REPORT_BODY:
        return "unexpected NIL or CNL in report body";

        case ReportError::AMD_ALLOWED_IN_TAF_ONLY:
        return "AMD is allowed in TAF only";

        case ReportError::CNL_ALLOWED_IN_TAF_ONLY:
        return "CNL is allowed in TAF only";

        case ReportError::MAINTENANCE_INDICATOR_ALLOWED_IN_METAR_ONLY:
        return "Maintenance indicator is allowed only in METAR reports";

        case ReportError::REPORT_TOO_LARGE:
        return "Report has too many groups and may be malformed";
    }
}


string reportTypeMessage(ReportType reportType) {
    switch (reportType) {
        case ReportType::UNKNOWN:
        return "unable to detect";

        case ReportType::METAR:
        return "METAR";

        case ReportType::TAF:
        return "TAF";
    }
}

class MyVisitor : public Visitor<string> {
    virtual string visitKeywordGroup(
        const KeywordGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Keyword: " + rawString);
    }

    virtual string visitLocationGroup(
        const LocationGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("ICAO location: " + rawString);
    }

    virtual string visitReportTimeGroup(
        const ReportTimeGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Report Release Time: " + rawString);
    }

    virtual string visitTrendGroup(
        const TrendGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Trend Header: " + rawString);
    }

    virtual string visitWindGroup(
        const WindGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Wind: " + rawString);
    }

    virtual string visitVisibilityGroup(
        const VisibilityGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Visibility: " + rawString);
    }

    virtual string visitCloudGroup(
        const CloudGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Cloud Data: " + rawString);
    }

    virtual string visitWeatherGroup(
        const WeatherGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Weather Phenomena: " + rawString);
    }

    virtual string visitTemperatureGroup(
        const TemperatureGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Temperature and Dew Point: " + rawString);
    }

    virtual string visitPressureGroup(
        const PressureGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Pressure: " + rawString);
    }

    virtual string visitRunwayStateGroup(
        const RunwayStateGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("State of Runway:" + rawString);
    }

    virtual string visitSeaSurfaceGroup(
        const SeaSurfaceGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Sea Surface: " + rawString);
    }

    virtual string visitMinMaxTemperatureGroup(
        const MinMaxTemperatureGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Minimum/Maximum Temperature: " + rawString);
    }

    virtual string visitPrecipitationGroup(
        const PrecipitationGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Precipitation: " + rawString);
    }

    virtual string visitLayerForecastGroup(
        const LayerForecastGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Atmospheric Layer Forecast: " + rawString);
    }

    virtual string visitPressureTendencyGroup(
        const PressureTendencyGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Pressure Tendency: " + rawString);
    }

    virtual string visitCloudTypesGroup(
        const CloudTypesGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Cloud Types: " + rawString);
    }

    virtual string visitLowMidHighCloudGroup(
        const LowMidHighCloudGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Low, middle, and high cloud layers: " + rawString);
    }

    virtual string visitLightningGroup(
        const LightningGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Lightning data: " + rawString);
    }

    virtual string visitVicinityGroup(
        const VicinityGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Events in vicinity: " + rawString);
    }

    virtual string visitMiscGroup(
        const MiscGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Miscellaneous Data: " + rawString);
    }

    virtual string visitUnknownGroup(
        const UnknownGroup & group,
        ReportPart reportPart,
        const string & rawString)
    {
        (void)group; (void)reportPart;
        return ("Not recognised by the parser: " + rawString);
    }
};

// ===================================================================================================
// |                               MAIN function start                                               |
// ===================================================================================================

int main(void)
{

    // Открываем файлы для заголовка и тела данных METAR и TAF и определяем другие переменные


    const string header_filename_metars = "files/metars.txt";
    const string body_filename_metars = "files/metars.csv";
    const string header_filename_tafs = "files/tafs.txt";
    const string body_filename_tafs = "files/tafs.csv";

    FILE *header_file_metars = fopen(header_filename_metars.c_str(),"w");
    if (header_file_metars == NULL)
    return -1;

    FILE *body_file_metars = fopen(body_filename_metars.c_str(),"w");
    if (body_file_metars == NULL)
    return -1;

    FILE *header_file_tafs = fopen(header_filename_tafs.c_str(),"w");
    if (header_file_tafs == NULL)
    return -1;

    FILE *body_file_tafs = fopen(body_filename_tafs.c_str(),"w");
    if (body_file_tafs == NULL)
    return -1;

    // Query data input

    cout << "\nPlease input flight path data or press <Ctrl+C> to default UKOO data: " << endl;
    cout << "\nDeparture AP ICAO: ";
    cin >> ap_departure;
    cout << "\nArriving AP ICAO: ";
    cin >> ap_arriving;
    cout << "\nRadius search (nm): ";
    cin >> search_radius;
    cout << "\nHours before now: ";
    cin >> hours_before_now;
    cout << "\nReceiving . . . " << endl;


    // Module to receive data files from AWC ======================================================================================

    CURL *curl_handle = curl_easy_init();
    if(curl_handle)
    {

        // METARS part -------------------------------------------------------------------------------------------------------------

        char url_metars_01[] = "https://aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=csv&flightPath=";
        char url_metars_02[] = "&hoursBeforeNow=";

        stringstream url_metars;
        url_metars << url_metars_01 << search_radius << ";" << ap_departure << ";" << ap_arriving << url_metars_02 << hours_before_now;
        string str_url_metars = url_metars.str();

        // debug block
        // test_input();
        //cout << str_url_metars << endl; //debug url

        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl_handle, CURLOPT_URL, str_url_metars.c_str());

        // save METARS file.csv
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, body_file_metars);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        // save header METARS file.txt
        curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, header_file_metars);

        CURLcode res_metars = curl_easy_perform(curl_handle);
        if(res_metars != CURLE_OK)
            cout << "curl_easy_perform() failed: %s\n" << curl_easy_strerror(res_metars) << endl;

        // TAFS part --------------------------------------------------------------------------------------------------------------

        char url_tafs_01[] = "https://aviationweather.gov/adds/dataserver_current/httpparam?dataSource=tafs&requestType=retrieve&format=csv&flightPath=";
        char url_tafs_02[] = "&hoursBeforeNow=";

        stringstream url_tafs;
        url_tafs << url_tafs_01 << search_radius << ";" << ap_departure << ";" << ap_arriving << url_tafs_02 << hours_before_now;
        string str_url_tafs = url_tafs.str();

        // debug block
        // test_input();
        // cout << str_url_tafs << endl; //debug url

        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl_handle, CURLOPT_URL, str_url_tafs.c_str());

        // save TAFS file.csv
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, body_file_tafs);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        // save header TAFS file.txt
        curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, header_file_tafs);

        CURLcode res_tafs = curl_easy_perform(curl_handle);
        if(res_tafs != CURLE_OK)
            cout << "curl_easy_perform() failed: %s\n" << curl_easy_strerror(res_tafs) << endl;

        curl_easy_cleanup(curl_handle);
    }

    fclose(header_file_metars);
    fclose(body_file_metars);
    fclose(header_file_tafs);
    fclose(body_file_tafs);

    cout << "\nDone!" << "\nFlightpath weather data were stored in subfolder </files> in that files: " << body_filename_metars << ", " << body_filename_tafs << endl;
	system("pause");


// !!!!!!!!!!!!!!!!!!!!!!!!!!  N E E D    T O    R E C O D E  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !           !            !                                 !                   !                      !

// === METAR section =====================================================================================

        int i = 1;
        int j = 1;
        char str_csv_metar1[8000];
        char str_txt_metar1[220];
        char str_csv_metar2[8000];
        char str_txt_metar2[220];

        FILE* fp_csv_m1 = fopen("files/metars.csv", "r");  // Path has to be changed for release ver.
        FILE* fp_txt_m1 = fopen("files/metaf.txt", "a");  // Path has to be changed for release ver.

        if (fp_csv_m1 != NULL) {
            while ((str_csv_metar1[i] = getc(fp_csv_m1)) != EOF) i++;
            str_csv_metar1[i] = '\0';
                for (i=663; i < 1500; i++) {
                    if (str_csv_metar1[i] != ',') {
                        str_txt_metar1[j] = str_csv_metar1[i];
                        j++;
                    }
                    if (str_csv_metar1[i] == ',') {
                        break;
                    }
                }

            str_txt_metar1[0] = 'M';
            str_txt_metar1[1] = 'E';
            str_txt_metar1[2] = 'T';
            str_txt_metar1[3] = 'A';
            str_txt_metar1[4] = 'R';
            str_txt_metar1[5] = ' ';
            str_txt_metar1[j] = '\n';
            cout << str_txt_metar1 << endl;

            fputs(str_txt_metar1, fp_txt_m1);
        }

     fclose(fp_csv_m1);


//== TAF section ========================================================================================

    i = 1;
    j = 1;
    char str_csv_taf1[8000];
    char str_txt_taf1[400];
    char str_csv_taf2[8000];
    char str_txt_taf2[400];

    FILE* fp_csv_t1 = fopen("files/tafs.csv", "r");  // Path has to be changed for release ver.

    if (fp_csv_t1 != NULL) {
        while ((str_csv_taf1[i] = getc(fp_csv_t1)) != EOF) i++;
        str_csv_taf1[i] = '\0';
            for (i=4784; i < 7000; i++) {
                if (str_csv_taf1[i] != ',') {
                    str_txt_taf1[j] = str_csv_taf1[i];
                    j++;
                }
                if (str_csv_taf1[i] == ',') {
                    break;
                }
            }

        str_txt_taf1[0] = 'T';
        str_txt_taf1[1] = 'A';
        str_txt_taf1[2] = 'F';
        str_txt_taf1[3] = ' ';
        str_txt_taf1[j] = '\n';
        cout << str_txt_taf1 << endl;

        fputs(str_txt_taf1, fp_txt_m1);
    }

    fclose(fp_csv_t1);
    fclose(fp_txt_m1);


// !           !            !                                 !                   !                      !
// !!!!!!!!!!!!!!!!!!!!!!!!!!  N E E D    T O    R E C O D E  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


//== Parsing section =====================================================================================

    cout << "Parsing report: " << str_txt_metar1 << "\n";
    const auto result1 = Parser::parse(str_txt_metar1);
    cout << "Parse error: ";
    cout << errorMessage(result1.reportMetadata.error) << "\n";
    cout << "Detected report type: ";
    cout << reportTypeMessage(result1.reportMetadata.type) << "\n";
    cout << result1.groups.size() << " groups parsed\n";
    MyVisitor visitor1;
    for (const auto groupInfo : result1.groups) {
        cout << visitor1.visit(groupInfo) << "\n";
    }
    system("pause");

    cout << "Parsing report: " << str_txt_taf1 << "\n";
    const auto result2 = Parser::parse(str_txt_taf1);
    cout << "Parse error: ";
    cout << errorMessage(result2.reportMetadata.error) << "\n";
    cout << "Detected report type: ";
    cout << reportTypeMessage(result2.reportMetadata.type) << "\n";
    cout << result2.groups.size() << " groups parsed\n";
    MyVisitor visitor3;
    for (const auto groupInfo : result2.groups) {
        cout << visitor3.visit(groupInfo) << "\n";
    }
    system("pause");

    return 0;
}

// ===================================================================================================
// |                               MAIN function end                                                 |
// ===================================================================================================


// Test function for flightpath data input -----------------------------------------------------------

int test_input(void)
{
    cout << ap_departure << endl;
    cout << ap_arriving << endl;
    cout << search_radius << endl;
    cout << hours_before_now << endl;
    return 0;
}


//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//                                                                                                       //
//                  E   N   D           O   F           P   R   O   G   R   A   M                        //
//                                                                                                       //
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
