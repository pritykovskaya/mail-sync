// for testing
#include "iostream"
using namespace std;

#include "util.h"

using namespace std;
void convert_time(date &my_date, time_t time) {
	struct tm* clock = new tm;			// create a time structure 
	clock = gmtime(&(time));	// Get the last time and put it into the time structure
	my_date.year = clock->tm_year + 1900; //returns the year (since 1900)
	my_date.month = clock->tm_mon + 1; //returns the month (January = 0)sd
	my_date.day = clock->tm_mday; //returns the day of the month
	my_date.hour =  clock->tm_hour;
	my_date.min =  clock->tm_min;
	my_date.sec =  clock->tm_sec;
}

bool get_file_date(date &my_date, string file_path) {
	FILE * pConfig;
	pConfig = fopen (file_path.c_str() ,"r"); 
	if (pConfig != NULL) {
		struct stat attrib;			// create a file attribute structure
        	stat(file_path.c_str(), &attrib);	// get the attributes of afile.txt		
		convert_time(my_date, attrib.st_mtime);		
		//delete clock;
		fclose (pConfig);
		return 1;
	} else { 
		return 0;
	}
}

bool date::operator<(const date &my_date) {
	if (this->year < my_date.year) {
		return  true;
	} 
	if (this->year > my_date.year) {
		return false;
	}	
	if (this->month < my_date.month) {
		return  true;
	} 
	if (this->month > my_date.month) {
		return false;
	}
	if (this->day < my_date.day) {
		return  true;
	} 
	if (this->day > my_date.day) {
		return false;
	}
	if (this->hour < my_date.hour) {
		return  true;
	} 
	if (this->hour > my_date.hour) {
		return false;
	}
	
	if (this->min < my_date.min) {
		return  true;
	} 
	if (this->min > my_date.min) {
		return false;
	}
	
	if (this->sec < my_date.sec) {
		return  true;
	} 
	if (this->sec >= my_date.sec) {
		return false;
	}
}	

//void archive(string folder_name, string input_file, string archive_file) {
//	system(("cd " + folder_name + " && tar czf " + archive_file + " " + input_file).c_str());
//}

string get_file_name(string file_path) {
	string tmp = file_path;
	size_t cur_found = tmp.find("/");

	while (cur_found != string::npos) {
		tmp = tmp.substr(cur_found + 1, tmp.length() - 1 - (int)cur_found);
		cur_found = tmp.find("/");
	}

	return tmp;
}

void archive(string input_file, string archive_file) {
	string folder_name;

	string tmp =  get_file_name(input_file);
	
	folder_name = input_file.substr(0, input_file.length() - tmp.length() - 1);
	system(("cd " + folder_name + " && tar czf " + archive_file + " " + tmp).c_str());
}

void unarchive(string folder_name, string archive_file) {
	system(("cd " + folder_name + " && tar xzf " + archive_file).c_str());
}

void delete_file(string file_name) {
	system(("rm " + file_name).c_str());
}

bool directory_exists(const char* pzPath)
{
    if ( pzPath == NULL) return false;
 
    DIR *pDir;
    bool bExists = false;
 
    pDir = opendir (pzPath);
 
    if (pDir != NULL)
    {
        bExists = true;    
        (void) closedir (pDir);
    }
 
    return bExists;
}

vector<string> read_dir(const char* path_to_directory)        {
	vector<string> files;
	DIR *dir = opendir(path_to_directory);
       char tmp[1024];
       struct stat st;

       if( dir )       {
               struct dirent *ent;

               while( (ent = readdir(dir)) != NULL)    {
                       if( (strcmp(ent->d_name, ".") == 0)     ||
                           (strcmp(ent->d_name, "..") == 0)  || 
			   (strcmp(ent->d_name, ".timestamp") == 0))  {
                               continue;
                       }

                       snprintf(tmp, 1024, "%s/%s", path_to_directory, ent->d_name);
                       stat(tmp, &st);
                       if((st.st_mode & S_IFMT) == S_IFDIR) // is Directory
                               continue;

			files.push_back(tmp);
               }
               closedir(dir);
       }
	return files;
 }
