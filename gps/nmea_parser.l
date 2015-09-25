%{

#include "gps_parser.h"

#define YY_EXTRA_TYPE gps::GpsParser*
#define YY_INPUT(buf,result,max_size) \
result = yyextra->readBytes(buf, max_size);

#include "log.h"

%}

%option reentrant
%option noyywrap
%option prefix="nmea_"

HEX_DIGIT [0-9a-zA-Z]
CHECKSUM \*{HEX_DIGIT}{2}
NEWLINE \r\n?|\n
SENTENCE_START $

%%


. { ALOGD("debug out: %s", yytext); }

%%

namespace gps {

void GpsParser::init_scanner() {
  yylex_init(&mScanner);
  yyset_extra(this, mScanner);
}

void GpsParser::destroy_scanner() {
  yylex_destroy(mScanner);
}

} // namespace gsp