#ifndef _REMOTESTORAGE_H
#define _REMOTESTORAGE_H
#include <string>
#include <vector>
#include <map>
#include <utility>

#include <vmime/vmime.hpp>
#include <vmime/platforms/posix/posixHandler.hpp>

#include "certificate.h"

#include "LocalStorage.h"

using namespace std;
	
class RemoteStorage {
	private:

		static string EMAIL_KEY;			
		static string LOGIN_KEY;	
		static string PASSWORD_KEY;	
		static string SMTP_SERVER_KEY;
		static string IMAP_SERVER_KEY;
		static string SMTP_PORT_KEY;
		static string TIMEFRAME_IN_DAYS_KEY;

		map<string, string> properties;		
		vmime::ref<vmime::net::session> sess;
		vmime::ref<vmime::net::store> store;
		vmime::ref<vmime::net::transport> transport;
		vmime::ref<vmime::net::folder> folder;

		bool is_store_initialized;
		bool is_transport_initialized;	
		void init_transport();
		void init_store();

		void read_config(string file_name);
	public:
		RemoteStorage();		

		vector<pair<string, int> > get_list();	
		vector<pair<string, int> > get_list(time_t stop_time);

		int find(string attachment_name);		
		
		bool download(int message_id, string path, string &why_message_was_not_downloaded);		
		void remove(int message_id);

		void send(string file_path);
		void send(string file_name, string file_path);

		void sync(LocalStorage ls);
};

#endif
