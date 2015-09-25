
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gps_parser.h"

extern "C" {
	extern int nmea_lex(void*);
}

namespace gps {

class ParserThread : public Thread {
	private:
	GpsParser* mParser;
	
	public:
	
	ParserThread(GpsParser* parser)
	: mParser(parser) { }
	
	~ParserThread() { }
	
	protected:
	bool threadLoop() {
		
		mParser->init_scanner();
		
		while(!exitPending()) {
			nmea_lex(mParser->mScanner);
		}
		
		mParser->destroy_scanner();
		
		return false;
	}
	
};
	
GpsParser::GpsParser()
: mFD(-1), mRecieveThread(NULL) {
	
}

GpsParser::~GpsParser() {
	if(mFD >= 0) {
		close(mFD);
	}
}

void GpsParser::setFD(int fd) {
	mFD = fd;
}

int GpsParser::readBytes(char* buf, const unsigned int maxBytes) {
	return read(mFD, buf, maxBytes);
}

void GpsParser::start() {
	mRecieveThread = new ParserThread(this);
	mRecieveThread->run("GPS Parser");
}

void GpsParser::stop() {
	mRecieveThread->requestExitAndWait();
}
	
	
} // namespace gps