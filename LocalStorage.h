#ifndef _LOCALSTORAGE_H
#define _LOCALSTORAGE_H

#include <string>
#include <time.h>

using namespace std;
class LocalStorage{
	private:
		string path;
	public:
		LocalStorage();
		string get_path();

		time_t get_last_sync_time();
		void set_last_sync_time(time_t time);
};

#endif
