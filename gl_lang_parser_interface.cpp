#include "gl_lang_parser_interface.h"

void gl_lang_error(Demo::GL::LocationType*, Demo::GL::Parser* parser, yyscan_t, const char* msg) {
    parser->createError(msg, Demo::GL::Parser::numerrors);
}
