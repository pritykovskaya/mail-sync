#ifndef UTIL_H
#define UTIL_H
//for file date
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "vector"

#include <string>
#include <fstream>
#include <cstring>

// for archive/unarchive
#include <stdlib.h>

//for working with directories
#include <dirent.h>

using namespace std;
struct date{
	int day;
        int month;
        int year;
	int hour;
	int min;
	int sec;
	bool operator<(const date &my_date);	
}; 

void convert_time(date &my_date, time_t time);

bool get_file_date(date &my_date, string file_path); 

bool directory_exists(const char* pzPath);

//void archive(string folder_name, string input_file, string archive_file);
void archive(string input_file, string archive_file);
void unarchive(string folder_name, string archive_file);
void delete_file(string file_name);

string get_file_name(string file_path);

vector<string> read_dir(const char* path_to_directory);
#endif
