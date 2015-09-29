%{

#include "gps_parser.h"

#define YY_EXTRA_TYPE gps::GpsParser*
#define YY_INPUT(buf,result,max_size) \
result = yyextra->readBytes(buf, max_size);

#include "log.h"

using namespace gps;

%}

%option reentrant
%option noyywrap
%option prefix="nmea_"

DATA [^\r|^\n|^\*]+
HEXDIGIT [0-9a-fA-F]
CHECKSUM \*{HEXDIGIT}{2}
NEWLINE \r\n
START \$


%%


{START}{DATA}{CHECKSUM} {
  size_t datalen = yyleng - 4;
  const Slice data(&yytext[1], datalen);
  if( gps::GpsParser::validChecksum(data, Slice(&yytext[datalen+1], 3)) ) {
    yyextra->emitNMEASentence(data);
  } else {
    ALOGW("invalid checksum: %s", yytext);
  }
}

{START}{DATA}{NEWLINE}   { 
  ALOGD("lex data no checksum: %s", yytext);
  size_t datalen = yyleng - 3;
  const Slice data(&yytext[1], datalen);
  yyextra->emitNMEASentence(data);
}
{NEWLINE}                {  }



. {  }

%%

#include <cstring>

using namespace std;

namespace gps {

void GpsParser::init_scanner() {
  yylex_init(&mScanner);
  yyset_extra(this, mScanner);
}

void GpsParser::destroy_scanner() {
  yylex_destroy(mScanner);
}

bool GpsParser::validChecksum(const Slice& data, const Slice& checksumStr) {
  char computedChecksum = 0;
  for(size_t i=0;i<data.size;i++){
    computedChecksum ^= data.ptr[i];
  }
  
  char receivedChecksum = 0;
  char checksumBuf[5];
  memset(checksumBuf, 0, 5);
  memcpy(checksumBuf, checksumStr.ptr, 5);
  if(sscanf(checksumBuf, "*%02hhx", &receivedChecksum) == 1){
    return computedChecksum == receivedChecksum;
  } else {
    ALOGE("fatial checksum error");
    return false;
  }
  
}

} // namespace gsp