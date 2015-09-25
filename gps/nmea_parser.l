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

HEXDIGIT [0-9a-fA-F]
digit [0-9]
CHECKSUM \*{HEXDIGIT}{2}
N_CHECKSUM [^\*]
START \$

%%

{START}{N_CHECKSUM}+{CHECKSUM} { ALOGD("sweet %s", yytext); }


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