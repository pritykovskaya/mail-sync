#ifndef _CERTIF_H
#define _CERTIF_H
#include <vmime/vmime.hpp>

class myCertVerifier : public vmime::security::cert::certificateVerifier {
		public: 
			void verify(vmime::utility::ref<vmime::security::cert::certificateChain> certs);
};
#endif
