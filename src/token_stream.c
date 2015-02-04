#include <string.h>
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

void stream_cat(token_stream *stream, token_stream cat) {
	int i;
	/* TODO - make this more efficient */
	for (i = 0; i < cat.length; ++i) {
		stream_append(stream, cat.tokens[i]);
	}
}

token_stream stream_tail(token_stream stream, int start) {
	return substream(stream, start, stream.length);
}

token_stream substream(token_stream stream, int start, int end) {
	stream.tokens += start;
	stream.length = end - start;
	
	return stream;
}

int stream_identical(token_stream a, token_stream b) {
	int i;
	if (a.length != b.length) return 0;
	for (i = 0; i < a.length; ++i) {
		token tok_a, tok_b;
		tok_a = a.tokens[i];
		tok_b = b.tokens[i];
		if (tok_a.type != tok_b.type) return 0;
		if (tok_a.type == newline) continue;
		if ((!tok_a.preceding_whitespace) != (!tok_b.preceding_whitespace)) return 0;
		if (strcmp(tok_a.text, tok_b.text)) return 0;
		if (tok_a.type == replacement && tok_a.replacement_num != tok_b.replacement_num) return 0;
	}
	return 1;
}
