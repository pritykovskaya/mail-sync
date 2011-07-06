#include <dirent.h>
#include <iostream> 

using namespace std;

bool DirectoryExists( char* pzPath )
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

int main() {
	bool result = DirectoryExists("/tmp2");
	cout << result << endl;	
	
	return 0;
}
