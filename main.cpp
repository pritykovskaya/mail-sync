#include <string>
#include "RemoteStorage.h"
#include "LocalStorage.h"
#include "certificate.h"
#include "util.h"
#include <iostream>

#include <fstream>
using namespace std;
int mapping[1000];

void short_help() {
	cout << "This tool has next commands:" << endl;
	cout << "1) view" << endl;
	cout << "2) remove" << endl;
	cout << "3) download" << endl;
	cout << "4) send" << endl;
	cout << "use --help for detailed help" << endl;
}

void detailed_help() {
	cout << "There are next commands:" << endl;
	cout << "1) view - to see full list of messages" << endl;
	cout << "2) remove - to delete letter from mailbox" << endl;
	cout << "3) download - to download an attachment from mailbox" << endl;
	cout << "4) send -  to write a letter" << endl;
}

void view(RemoteStorage RS) {
	cout << "fetching messages " << endl;
	vector< pair < string, int> > list_of_messages = RS.get_list();
	if (list_of_messages.size() == 0) {
		cout << "There are no letters send by our util" << endl;
	} else {
		cout << "Inbox:" << endl;
		for (int i = 0; i < list_of_messages.size(); ++i) {
			pair<string, int> e = list_of_messages[i];
			cout << i + 1 << ") " << e.first << endl; 
			mapping[i + 1] = e.second;
		}
	}
}
int main(int argc, char** argv)
{	
	if (argv[1] == NULL) {
		short_help();
	} else if ((string)argv[1] == "--help") {		
		detailed_help();
	} else if ((string)argv[1] == "view") {
		RemoteStorage RS;
		view(RS);
	} else if ((string)argv[1] == "download") {
		RemoteStorage RS;
		LocalStorage LS;	
		bool res;	
		string why_message_was_not_downloaded;
		if (argv[2] == NULL) {
			view(RS);
			cout << "Enter number: ";
			int num;
			cin >> num;			
			res = RS.download(mapping[num], LS.get_path(), why_message_was_not_downloaded);
		} else {
			if (argv[3] == NULL) {
				res = RS.download(RS.find((string)argv[2]), LS.get_path(), why_message_was_not_downloaded);
			} else {
				if(!directory_exists(argv[3])) {
					cout << "There is no such directory";
				} else {
					res = RS.download(RS.find((string)argv[2]), argv[3], why_message_was_not_downloaded);
				}
			}
		}
		if (res) {
			cout << "new version was downloaded" << endl; 
		} else {
			cout << why_message_was_not_downloaded << endl;
		}
	} else if ((string)argv[1] == "remove") {
		RemoteStorage RS;
		if (argv[2] == NULL) {
			view(RS);
			cout << "Enter number: ";
			int num;
			cin >> num;
			RS.remove(mapping[num]);
		} else {
			RS.remove(RS.find((string)argv[2]));
		}
	} else if ((string)argv[1] == "send") {
		RemoteStorage RS;
		LocalStorage LS;
		if (argv[2] == NULL) {
			//cout << "Enter file name in local storage: ";
			cout << "Enter file name:";
			string file_name;
			cin >> file_name;
			// in header we have a path assosiated with LS
			//RS.send(file_name, LS.get_path());
			RS.send(file_name);
		} else {
			//RS.send(argv[2], LS.get_path());
			RS.send(argv[2]);
		}
	} else if ((string)argv[1] == "sync") {
		RemoteStorage RS;
		LocalStorage LS;

		RS.sync(LS);
	} else {
		cout << "Illegal command" << endl;
		short_help();
	}
		
	return 0;	
}

