#include "LocalStorage.h"
#include <stdio.h>
#include <fstream>
LocalStorage::LocalStorage() {
	string tmp;
	string key;
	string value;
	ifstream myfile;
  	myfile.open("config.txt");
 
	if (myfile.is_open()) {
		while (!myfile.eof()) {
			getline(myfile, tmp);
			if (tmp.find("include") == 0) {
			} else {
				size_t found = tmp.find(": ");
				if (found != string::npos) {
					key = tmp.substr(0, found);
					value = tmp.substr((int)found + 2, tmp.length() - 2 - (int)found);
					
					if (key == "local directory") {
						path = value;
					}					
				}
			}
		}
		myfile.close();
	} else {
		throw string("Can't open a file");	
	}
}

string LocalStorage::get_path() {
	return path;
}

time_t LocalStorage::get_last_sync_time() {
	fstream input;

	input.open((path+"/.timestamp").c_str(), fstream::in);

	if (!input.is_open()) {
		return 0;
	}

	time_t time;
	
	input >> time;

	input.close();	

	return time;
}

void LocalStorage::set_last_sync_time(time_t time) {
	fstream output;

	output.open((path+"/.timestamp").c_str(), fstream::out);

	output << time;

	output.close();	
}

