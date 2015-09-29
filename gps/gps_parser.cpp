
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "gps_parser.h"

#include "log.h"

int nmea_lex(void*);


namespace gps {

#define IDLE 0
#define RUNNING 1
#define SHUTTING_DOWN 2

class ParserThread : public Thread {
	GpsParser* mParser;
	
	public:
	ParserThread(GpsParser* parser)
	: Thread(false), mParser(parser) { }
	
	virtual ~ParserThread() { }
	
	private:
	virtual bool threadLoop() {
		ALOGI("starting gps parser thread...");
		
		mParser->mState = RUNNING;
		mParser->init_scanner();
		
		int value;
		while(!exitPending()) {
			value = nmea_lex(mParser->mScanner);
			ALOGI("nmea_lex returned: %d", value);
			if(value == 0) {
				break;
			}
		}
		
		mParser->destroy_scanner();
		
		ALOGI("exiting gps parser thread");
		
		return false;
	}
	
};
	
GpsParser::GpsParser()
: mState(IDLE), mFD(-1) { }

GpsParser::~GpsParser() {
	if(mFD >= 0) {
		close(mFD);
	}
}

void GpsParser::setFD(int fd) {
	mFD = fd;
}

int GpsParser::readBytes(char* buf, const unsigned int maxBytes) {
	TRACE("maxBytes: %d", maxBytes);
	if(mState == SHUTTING_DOWN) {
		return 0;
	}
	int bytesRead = read(mFD, buf, maxBytes);
	ALOGI("%d bytes read", bytesRead);
	if(bytesRead < 0) {
		return 0;
	}
	return bytesRead;
}

void GpsParser::start() {
	mRecieveThread = new ParserThread(this);
	mRecieveThread->run("GPS Parser");
}

void GpsParser::stop() {
	mState = SHUTTING_DOWN;
	status_t result = mRecieveThread->requestExitAndWait();
	if (result) {
		ALOGW("could not stop parser thread. Error: %d", result);
	} 
	ALOGI("parser stopped");
}

void GpsParser::emitNMEASentence(const Slice& data) {
	
}
	
	
} // namespace gps