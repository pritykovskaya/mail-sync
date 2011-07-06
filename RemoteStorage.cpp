#include "RemoteStorage.h"
#include "certificate.h"
#include "util.h"
// for working with files
#include <stdio.h>
// for standart cout
#include <iostream>
#include <fstream>
// for converting strings
#include <cstring>
#include <stdlib.h>
#include <set>

using namespace std;

//to catch bags on the complilation
string RemoteStorage::LOGIN_KEY = "login";
string RemoteStorage::EMAIL_KEY = "email";
string RemoteStorage::PASSWORD_KEY = "password";
string RemoteStorage::SMTP_SERVER_KEY = "smtp server";
string RemoteStorage::IMAP_SERVER_KEY = "imap server";
string RemoteStorage::SMTP_PORT_KEY = "smtp port";
string RemoteStorage::TIMEFRAME_IN_DAYS_KEY = "timeframe in days";


vmime::ref<vmime::message> convertMessage(vmime::ref<vmime::net::message> message) {
	vmime::string data;
	vmime::utility::outputStreamStringAdapter out(data);
	message->extract(out);	
	// in string data - message data 
	
	vmime::ref<vmime::message> msg = vmime::create<vmime::message>();	
	msg->parse(data);
	
	return msg;
}

void extractContentHandlerToFile(vmime::ref<const vmime::contentHandler> content_handler, string file_name) {
	ofstream file;
	file.open(file_name.c_str(), ios::out | ios::binary);		
	vmime::utility::outputStreamAdapter out(file);
	content_handler->extract(out);
	file.close();		
}
string getName(vmime::ref<vmime::net::message> message) {
	vmime::ref<const vmime::headerFieldValue> headerValue = message->getHeader()->Subject()->getValue();
	vmime::ref<const vmime::text> headerText = headerValue.dynamicCast<const vmime::text>();
		
	return headerText->getConvertedText(vmime::charset());
}
vmime::datetime getDate(vmime::ref<vmime::net::message> message) {
	vmime::ref<const vmime::headerFieldValue> headerValue = message->getHeader()->Date()->getValue();
	vmime::ref<const vmime::datetime> headerDateTime = headerValue.dynamicCast<const vmime::datetime>();
		
	return *headerDateTime;
}
void RemoteStorage::read_config(string file_name) {
	string tmp;
	string key;
	string value;
	ifstream myfile;
  	myfile.open (file_name.c_str());
 
	if (myfile.is_open()) {
		while (!myfile.eof()) {
			getline(myfile, tmp);
			if (tmp.find("include") == 0) {
				read_config(tmp.substr(8, tmp.length() - 8));	
			} else {
				size_t found = tmp.find(": ");
				if (found != string::npos) {
					key = tmp.substr(0, found);
					value = tmp.substr((int)found + 2, tmp.length() - 2 - (int)found);
					properties[key] = value;
				} else {
				//throw string("Wrong format of config file");		
				}
			}
		}
		myfile.close();
	} else {
		//throw string("Can't open a file");	
	}
}

RemoteStorage::RemoteStorage() {
	read_config("config.txt");	
	
	is_transport_initialized = false;
	is_store_initialized = false;

	// tight with platform
	vmime::platform::setHandler<vmime::platforms::posix::posixHandler>();
	
	// creating session
	sess = vmime::create<vmime::net::session>();		
		
}

void RemoteStorage::init_store() {
	if (!is_store_initialized) { 
		//creating store	
		string STORE_EMAIL_URL = "imaps://" + properties[LOGIN_KEY] + ":" + properties[PASSWORD_KEY] + "@" + properties[IMAP_SERVER_KEY];
		vmime::utility::url store_url(STORE_EMAIL_URL);
	
		store = sess->getStore(store_url);	
		store->setCertificateVerifier(vmime::create<myCertVerifier>());
		store->connect();
	
		//creating folder
		folder = store->getDefaultFolder();
		is_store_initialized = true;
	}
}

void RemoteStorage::init_transport() {
	if (!is_transport_initialized) { 	
		vmime::utility::url transport_url("smtp://" + properties[SMTP_SERVER_KEY]);
		transport = sess->getTransport(transport_url);	

		transport->setProperty("auth.username", properties[LOGIN_KEY]);	
		transport->setProperty("auth.password", properties[PASSWORD_KEY]);
		transport->setProperty("server.port", properties[SMTP_PORT_KEY]);
	
		transport->setCertificateVerifier(vmime::create<myCertVerifier>());	
		transport->setProperty("connection.tls", true);	
		transport->setProperty("options.need-authentication", true);	
		
		transport->connect();
		is_transport_initialized = true;
	}
}


void parse_header(string header, date &my_date, string &path) {
	sscanf(header.c_str(), "nsync %d.%d.%d %d:%d:%d", &my_date.day, &my_date.month, &my_date.year, &my_date.hour, &my_date.min, &my_date.sec);
	size_t pos = header.find(" ", 17);
	path = header.substr(pos + 1, header.length() - (int)pos);
}


time_t converse_date(vmime::datetime date) {
	struct tm* c_date = new tm;		
	c_date->tm_year = date.getYear() - 1900; //returns the year (since 1900)
	c_date->tm_mon = date.getMonth() - 1; //returns the month (January = 0)sd
	c_date->tm_mday = date.getDay(); //returns the day of the month
	c_date->tm_hour =  date.getHour();
	c_date->tm_min =  date.getMinute();
	c_date->tm_sec =  date.getSecond();
	time_t letter_date = mktime(c_date);
	delete c_date;
	return letter_date;

}
bool is_letter_fresh(time_t stop, vmime::datetime letter_date){
	time_t t_letter_date = converse_date(letter_date);
	//cout << t_letter_date << " " << stop << endl;  
	return (t_letter_date > stop);
}

vector<pair<string, int> > RemoteStorage::get_list() {
	init_store();

	vector<pair<string, int> > result;	

	folder->open(vmime::net::folder::MODE_READ_WRITE);
	if (folder->getMessageCount() == 0) {
		return result;
	}

	std::vector <vmime::ref <vmime::net::message> > messages = folder->getMessages();
	std::vector <vmime::ref <vmime::net::message> > message_block;

	//извлекаем время первого письма
	folder->fetchMessage(messages[messages.size() - 1], vmime::net::folder::FETCH_ENVELOPE);
	vmime::datetime first_letter_date = getDate(messages[messages.size() - 1]);
	time_t start = converse_date(first_letter_date);
		
	return RemoteStorage::get_list(start - atoi(properties[TIMEFRAME_IN_DAYS_KEY].c_str())*24*60*60);
}

vector<pair<string, int> > RemoteStorage::get_list(time_t stop_time) {
	init_store();
	vector<pair<string, int> > result;
	folder->open(vmime::net::folder::MODE_READ_WRITE);
	if (folder->getMessageCount() == 0) {
		return result;
	}
	std::vector <vmime::ref <vmime::net::message> > messages = folder->getMessages();
	std::vector <vmime::ref <vmime::net::message> > message_block;

	time_t stop = stop_time;
	
	//убрать!!!
	int last_fetched = messages.size();
	for (int i = (messages.size() - 1); i >= 0; i--) {		
		if (i < last_fetched) {
			// fetch last_fetched-1 .. last_fetched-10
			vector<vmime::ref <vmime::net::message> > to_fetch;
			for (int j = last_fetched-1; j >= last_fetched - 10, j >= 0; j--) {
				to_fetch.push_back(messages[j]);
			} 

			folder->fetchMessages(to_fetch, vmime::net::folder::FETCH_ENVELOPE);
			
			last_fetched = last_fetched - 10;
			if (last_fetched < 0) {
				last_fetched = 0;
			}
		}
		
		folder->fetchMessage(messages[i], vmime::net::folder::FETCH_ENVELOPE);
		string tmp = getName(messages[i]);
		vmime::datetime letter_date = getDate(messages[i]);
  		if (is_letter_fresh(stop, letter_date)) {
			if (tmp.find("nsync") == 0) {
				// cout << "add to result" << endl;
				result.push_back(make_pair(tmp, messages[i]->getNumber()));
			} 
		} else {
			break;
		}
	}
	return result;
}


int RemoteStorage::find(string attachment_name) {
	vector<pair<string, int> > list_of_messages = get_list();
	
	for (int i = 0; i < list_of_messages.size(); i++) {
		string header = list_of_messages[i].first;

		date my_date;
		string path;

		parse_header(header, my_date, path);

		if (path == attachment_name) {
			return list_of_messages[i].second;
		}		
	}

	return -1;
}

bool RemoteStorage::download(int message_id, string path, string &why_message_was_not_downloaded) {
	if (message_id == -1) {
		why_message_was_not_downloaded = "There is no such message in the remote storage";
		return false;
	}	

	init_store();
	folder->open(vmime::net::folder::MODE_READ_WRITE);
	vmime::ref <vmime::net::message> message = folder->getMessage(message_id);
	folder->fetchMessage(message, vmime::net::folder::FETCH_ENVELOPE);
	string name;
	date s_date;
	parse_header(getName(message), s_date, name);
	date f_date;	
	if (!get_file_date(f_date, path + "/" + name) || f_date < s_date) {
		const std::vector<vmime::ref<const vmime::attachment> > atts = vmime::attachmentHelper::findAttachmentsInMessage(convertMessage(message));
		// тут надо проверить, что такая папка есть
		extractContentHandlerToFile(atts[0]->getData(), path + "/" + name + ".tgz");
		unarchive(path, name + ".tgz");
		delete_file(path + "/" + name + ".tgz");
		//cout << f_date.min << ":" << f_date.sec << " " << s_date.min << ":" << s_date.sec << " " << f_date < s_date << endl;
 		return true;
	}	
	why_message_was_not_downloaded = "Your version is the newest one";
	return false;
}

void  RemoteStorage::remove(int message_id) {
	if (message_id != -1) {
		init_store();
		folder->open(vmime::net::folder::MODE_READ_WRITE);		
		folder->deleteMessage(message_id);
	}	
} 

void RemoteStorage::sync(LocalStorage ls) {
	cout << "Sync" << endl;
	
	date last_sync;
	convert_time(last_sync, ls.get_last_sync_time());

	vector<pair<string, int> > files = get_list();
	time_t start_sync_time;
	time(&start_sync_time);

	set<string> remote_storage_files;
	
	for (int i = 0; i < files.size(); i++) {
		pair<string, int> file = files[i];
		
		if (file.first.find("/") == string::npos) {
			date mail_date;
			string path;			

			parse_header(file.first, mail_date, path);
	
			remote_storage_files.insert(path);

			date file_date;
			
			if (!get_file_date(file_date, ls.get_path()+"/"+path)) {
				// file not exist
				if (last_sync < mail_date) {
					string tmp;
					cout << "File " << path << " doesn't exist in local storage; Downloading..." << endl;
					if (!download(file.second, ls.get_path(), tmp)) {
						cout << tmp;
					}
				} else {
					cout << "File " << path << " removed in local storage; Removing file from remote storage" << endl;
					remove(file.second);
				}
			} else {
				// file exist
				if (mail_date < file_date) {
					cout << "File " << path << " sending to remote storage" << endl;
					send(path, ls.get_path()+"/"+path);
				} 

				if (file_date < mail_date) {
					cout << "File " << path << " downloading from remote storage" << endl;
					string tmp;
					if (!download(file.second, ls.get_path(), tmp)) {
						cout << tmp;
					}
				}		
			}
		}
	}
 
	vector<string> local_files = read_dir(ls.get_path().c_str());

	for (int i = 0; i < local_files.size(); i++) {
		string file = get_file_name(local_files[i]);		

		if (remote_storage_files.count(file) == 0) {
			cout << "sending new file " << file << " to remote storage" << endl;
			send(file, ls.get_path()+"/"+file);
		}
	} 	
	
	ls.set_last_sync_time(start_sync_time);	

	cout << "Sync is finished" << endl;
}

string make_header(date my_date, string attachment_name) {
	char date[255];
	sprintf(date, "%d.%d.%d %d:%d:%d", my_date.day, my_date.month, my_date.year, my_date.hour, my_date.min, my_date.sec);

	return "nsync " + (string)date + " " + attachment_name;
}
		// in header we have a path assosiated with LS
		

//void  RemoteStorage::send(string file_name, string folder_path) {
void  RemoteStorage::send(string file_name) {
	RemoteStorage::send(file_name, file_name);
}

void  RemoteStorage::send(string file_name, string file_path) {
	init_transport();

	date my_date;
	if (get_file_date(my_date, file_path)) {	
		string header = make_header(my_date, file_name);	

	//	cout << "Removing old version" << endl;
		int id = find(file_name);
	//	cout << id << endl;
		if (id != -1) {
			remove(id);
		}
	//	cout << "Sending \"" << folder_path + "/" +file_name << "\" with name " << header << endl;		
		cout << "Sending \"" << file_name << "\" with name " << header << endl;
		vmime::messageBuilder mb;
		mb.setSubject(vmime::text(header));
		mb.setExpeditor(vmime::mailbox(properties[EMAIL_KEY]));
		mb.getRecipients().appendAddress(vmime::create<vmime::mailbox>(properties[EMAIL_KEY]));
		mb.getTextPart()->setText(vmime::create<vmime::stringContentHandler>("Text..."));
	
		//archive(folder_path, file_name, file_name + ".tgz");
	
		archive(file_path, file_path + ".tgz");
		//vmime::ref<vmime::fileAttachment> att = vmime::create<vmime::fileAttachment>(folder_path + "/" + file_name + ".tgz", vmime::mediaType(), vmime::text("Attachment text"));
		 
		vmime::ref<vmime::fileAttachment> att = vmime::create<vmime::fileAttachment>(file_path + ".tgz", vmime::mediaType(), vmime::text("Attachment text"));
		mb.appendAttachment(att);

		vmime::ref<vmime::message> msg = mb.construct();
		//fprintf(stderr, "before \n");
		transport->send(msg);
		//fprintf(stderr, "after \n");
		//delete_file(folder_path + "/" + file_name + ".tgz");
		delete_file(file_path + ".tgz");
		cout << "Message is sent" << endl;
	} else {
		cout << "There is no such file" << endl;
	}
}

