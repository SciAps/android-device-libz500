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

DATA [^\r|^\n|^\*]+
HEXDIGIT [0-9a-fA-F]
CHECKSUM \*{HEXDIGIT}{2}
NEWLINE \r?\n|\n
START \$


%%


{START}{DATA}{CHECKSUM}  { ALOGD("lex CHECKSUM"); }
{START}{DATA}{NEWLINE}   { ALOGD("lex CHECKSUM"); }
{NEWLINE}                { ALOGD("lex NEWLINE"); }



. { ALOGD("lex debug out: %s", yytext); }

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