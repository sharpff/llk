#ifndef __JSMN_PRIV_H__
#define __JSMN_PRIV_H__
#include "jsonv2.h"
#define JSMN_STRICT		1
#define JSMN_PARENT_LINKS	1


typedef enum {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
	JSMN_ERROR_PART = -3,
} jsmnerr_t;

//typedef jsmn_parser jsmn_parser;
typedef jsontok_t jsmntok_t;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens,
 * each describing a single JSON object.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens);

#endif /* __JSMN_PRIV_H__ */
