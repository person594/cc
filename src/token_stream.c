#include <stdlib.h>
#include "token_stream.h"

token_stream stream_create() {
	token_stream stream;
	stream.length = 0;
	stream.tokens = (token *)malloc(INIT_STREAM_LENGTH * sizeof(token));
	return stream;
}

void stream_append(token_stream *stream, token tok) {
	stream->tokens[stream->length++] = tok;
	if (stream->length < INIT_STREAM_LENGTH) return;
	/* if length is a power of 2 */
	if ((stream->length & (stream->length - 1)) == 0) {
		stream->tokens = realloc(stream->tokens, 2*sizeof(token) * stream->length);
	}
}
