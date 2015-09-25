#ifndef GPS_PARSER_H_
#define GPS_PARSER_H_

#include <utils/Thread.h>

using namespace android;

namespace gps {
	
	class GpsParser {
		friend class ParserThread;
		
		private:
		int mState;
		int mFD;
		void* mScanner;
		sp<Thread> mRecieveThread;
		
		void init_scanner();
		void destroy_scanner();
		
		public:
		GpsParser();
		~GpsParser();
		
		void setFD(int fd);
		int readBytes(char* buf, const unsigned int maxBytes);
		void start();
		void stop();
		
	};
	
} // namespace gps


#endif // GPS_PARSER_H_