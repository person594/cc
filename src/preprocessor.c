#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "preprocessor.h"
#include "hash_table.h"
#include "errors.h"

#if 0
hash_table macro_symbol_table;

void initialize() {
	static int initialized = 0;
	if (!initialized) {
		macro_symbol_table = hash_table_create(MACRO_HASH_TABLE_SIZE);
		tokenizer_initialized = 1;
	}
}

/* TODO - do I need to do any special work to make this reentrant?
 * static variables seem like a bad idea
 */
void expand_and_append(token tok) {
	static macro *current_function_macro = NULL;
	static token function_macro_token;
	static int args_read = 0;
	static int tokens_in_arg = 0;
	static int paren_depth = 0;
	static token **arguments = NULL;
	
	/*  if we are currently reading in the parameters to a function-like
	 *  macro, we do something entirely different
	 *  Remember that when we stop parsing our arguments, we need to add our
	 *  macro back to the symbol table. */
	if (current_function_macro) {
		if (paren_depth == 0) {
			/*  we'll only be here if we just started  */
			if (strcmp(tok.text, "(")) {
				/* Oops, they were just mentioning the name of the macro, not envoking it */
				free(arguments);
				stream_append(&stream, function_macro_token);
				hash_table_insert(macro_symbol_table, tok.text, current_function_macro);
				expand_and_append(tok);
				current_function_macro = NULL;
				return;
			} else {
				tokens_in_arg = 0;
				paren_depth++;
				return;
			}
		}
		
		if (strcmp(tok.text, "(") == 0) {
			paren_depth++;
		}
		
		if (strcmp(tok.text, ")") == 0) {
			if (--paren_depth == 0) {
				/* We need some level of reentrancy here, so we're making a copy
				 * of lots of static variables to keep on the stack. */
				macro *mac;
				token **args, macro_token;
				int n_args;
				int i;
				if (current_function_macro->num_params > 0) {
					arguments[args_read] = realloc(arguments[args_read], (tokens_in_arg + 1) * sizeof(token));
					tok.type = eof;
					arguments[args_read][tokens_in_arg++] = tok;
				}
				n_args = ++args_read;
				mac = current_function_macro;
				macro_token = function_macro_token;
				args = arguments;
				current_function_macro = NULL;
				if (n_args != mac->num_params) {
					int i;
					issue_error("Incorrect number of arguments for macro", function_macro_token);
				} else { /* This part can be recursive, we've gotta be careful here */
					/* This will be recursively calling this function.
					 * That's why we make sure our state is nice before calling it. */
					expand_function_like_macro(mac, args);
				}
				for (i = 0; i < mac->num_params && i < n_args; ++i) {
					free(args[i]);
				}
				if (mac->num_params > 0) {
					free(args);
				}
				hash_table_insert(macro_symbol_table, macro_token.text, mac);
				return;
			}
		}
		
		/* At this point we know they haven't closed their parens, they actually
		 * intend to give us an argument */
		
		if (args_read == -1) {
			args_read = 0;
		}
		
		if (args_read >= current_function_macro->num_params) {
			/* we'll be throwing an error sooner or later, but let's just keep eating
			 * tokens until they close their parens */
			return;
		}
		
		/* After this point, we know that arguments[args_read] is in bounds,
		 * i.e. they haven't given us too many arguments yet */
		
		if (tokens_in_arg == 0) {
			arguments[args_read] = (token *)malloc(sizeof(token));
		} else if ((tokens_in_arg & (tokens_in_arg - 1)) == 0) {
		 /* if tokens_in_arg is a power of 2, double the capacity of args[args_read] */
			arguments[args_read] = (token *)realloc(arguments[args_read], 2 * tokens_in_arg * sizeof(token));
		}
		if (paren_depth == 1 && strcmp(tok.text, ",") == 0) {
			tok.type = eof;
			arguments[args_read][tokens_in_arg++] = tok;
			args_read++;
			tokens_in_arg = 0;
		} else {
			arguments[args_read][tokens_in_arg++] = tok;
		}
		
		
		return;
	}
	
	
	if (tok.type == identifier) {
		macro *mac;
		/* We remove the macro from our table to prevent recursive expansion.
		 * Don't worry, we'll be adding it back later. */
		mac = hash_table_remove(macro_symbol_table, tok.text);
		if (mac) {
			if (mac->num_params < 0) { /* Object-like macro, this is easy */
				int i = 0;
				for (i = 0; i <  mac->body_len; ++i) {
					expand_and_append(mac->body[i]);
				}
				/* We're done with this macro, so we add it back to the symbol table */
				hash_table_insert(macro_symbol_table, tok.text, mac);
			} else { /* Oh no, a function-like macro, these are tricky. 
				We won't do anything just yet, as we still need to read in the arguments */
				current_function_macro = mac;
				function_macro_token = tok;
				/* The closing paren always increments args_read, we use -1 to flag
				 * we've seen no real tokens in the macro call yet.*/
				args_read = -1;
				paren_depth = 0;
				if (mac->num_params > 0) {
					arguments = malloc(mac->num_params * sizeof(token *));
				}
			}
		} else {
			stream_append(&stream, tok);
		}
	} else if (tok.type != comment) {
		stream_append(&stream, tok);
	}
}

void tokenize(FILE *file) {
	int is_new_line = 1;
	token tok;
	if (!tokenizer_initialized) initialize();
	consume_whitespace(file);
	while ((tok = scan_token(file)).type != eof) {
		if (is_new_line && strcmp(tok.text, "#") == 0) {
			parse_directive(file);
			consume_whitespace(file);
			continue;
		}
		expand_and_append(tok);
		is_new_line = consume_whitespace(file);
	}
}

void parse_directive(FILE *file) {
	token tok;
	int found = 0;
	/* GCC seems perfectly content with empty preprocessor directives
	 * TODO : figure out if this is the correct treatment */
	if (consume_whitespace(file)) return;
	tok = scan_token(file);
	if (strcmp(tok.text, "include") == 0) {
		char *search_path, *path, ch;
		if (consume_whitespace(file)) {
			issue_error("Expected filename after include directive", tok);
			return;
		}
		ch = peek_ch(file);
		if (ch != '\"' && ch != '<') {
			issue_error("Expected filename after include directive", tok);
			skip_line(file);
			return;
		}
		tok = scan_preprocessor_string(file);
		/* Here is lots of filesystem specific stuff.  TODO - maybe make this work on bad (non-unix-like) filesystems */
		if (tok.text[0] == '\"') {
			search_path = (char *)malloc((strlen(QUOTE_PATH) + strlen(BRACKET_PATH) + 2) * sizeof(char));
			strcpy(search_path, QUOTE_PATH);
			strcat(search_path, ":");
			strcat(search_path, BRACKET_PATH);
		} else if (tok.text[0] == '<') {
			search_path = (char *)malloc((strlen(BRACKET_PATH) + 1) * sizeof(char));
			strcpy(search_path, BRACKET_PATH);
		} else {
			/* There was a tokenization error */
			skip_line(file);
			return; 
		}
		path = strtok(search_path, ":");
		while (path) {
			int path_len, tok_len;
			FILE *include_file;
			char *full_path;
			path_len = strlen(path);
			tok_len = strlen(tok.text);
			full_path = (char *)malloc(path_len + tok_len);
			/* I didn't forget the null character.  tok.text has 2 extra quoting
			 * characters, and we are adding a delimiter between the 2 parts */
			memcpy(full_path, path, path_len);
			full_path[path_len] = '/';
			memcpy(&full_path[path_len+1], &tok.text[1], tok_len-2);
			full_path[path_len+tok_len-1] = '\0';
			include_file = fopen(full_path, "r");
			free(full_path);
			if (include_file) {
				tokenize(include_file);
				fclose(include_file);
				found = 1;
				break;
			}
			path = strtok(NULL, ":");
		}
		if (!found) {
			issue_error("File not found", tok);
		}
		free(search_path);
		while (!consume_whitespace(file)) {
			tok = scan_token(file);
			issue_error("Unexpected token", tok);
		}
	} else if (strcmp(tok.text, "define") == 0) {
		if (consume_whitespace(file)) {
			issue_error("Expected filename after include directive", tok);
			return;
		}
		parse_macro_definition(file);
	/* Conditionals rely on our ability to parse and evaluate constant
	 * expressions, which we cannot do now.  TODO : implement #if, #else
	 * #endif, #ifdef, and #ifndef.*/	
	} else {
		issue_error("Unknown preprocessor directive", tok);
		skip_line(file);
	}
}

/*
 * TODO - Varargs in function-like macros
 */
void parse_macro_definition(FILE *file) {
	token tok;
	macro *mac;
	char *macro_name;
	int body_size;
	mac = (macro *)malloc(sizeof(macro));
	mac->num_params = -1;
	mac->body_len = 0;
	mac->params = NULL;
	mac->body = NULL;
	tok = scan_token(file);
	if (tok.type != identifier) {
		issue_error("Macro name must be an identifier", tok);
		skip_line(file);
		return;
	}
	if (strcmp(tok.text, "defined") == 0) {
		issue_error("\"defined\" cannot be used as a macro name", tok);
		skip_line(file);
		return;
	}
	macro_name = tok.text;
	/* Function-like macro names are followed immediately by a left paren
	 * with no space between */
	if (peek_ch(file) == '(') {
		int params_size = INIT_MACRO_PARAMS_SIZE;
		mac->num_params = 0;
		mac->params = (char **)malloc(params_size * sizeof(char *));
		tok = scan_token(file);
		do {
			if (consume_whitespace(file)) {
				issue_error("Illegal whitespace in macro definition", tok);
				free(mac->params);
				return;
			}
			tok = scan_token(file);
			if (tok.type != identifier) {
				issue_error("Unexpected token; expected macro parameter", tok);
				free(mac->params);
				return;
			}
			
			if (mac->num_params == params_size) {
				params_size *= 2;
				mac->params = realloc(mac->params, params_size * sizeof(char *));
			}
			mac->params[mac->num_params++] = tok.text;
			
			if (consume_whitespace(file)) {
				issue_error("Illegal whitespace in macro definition", tok);
				free(mac->params);
				return;
			}
			
			tok = scan_token(file);
			
		} while(strcmp(tok.text, ",") == 0);
		if (strcmp(tok.text, ")")) {
			issue_error("Unexpected token; comma or closing parenthesis expected", tok);
			free(mac->params);
			return;
		}
	} /* The macro body; object-like macros start here */
	body_size = INIT_MACRO_BODY_SIZE;
	mac->body = malloc(body_size * sizeof(token));
	while (consume_whitespace(file) == 0) {
		tok = scan_token(file);
		mac->body[mac->body_len++] = tok;
		if (mac->body_len == body_size) {
			body_size *= 2;
			mac->body = realloc(mac->body, body_size * sizeof(token));
		} 
	}
	/* Insert mac, and retreive the old macro there for freeing */
	mac = hash_table_insert(macro_symbol_table, macro_name, mac);
	/* TODO
	 * Redefinition should be an error unless the two macros are identical
	 * Right now we simply replace the old macro with the new one.
	 * Fix this.
	 * */
	if (mac) {
		if (mac->params) {
			free(mac->params);
		}
		free(mac->body);
	}
}


/*
 * TODO - handle the ## operator
 * 
 */
void expand_function_like_macro(macro *mac, token **args) {
	int i;
	for (i = 0; i < mac->body_len; ++i) {
		int j;
		int found = 0;
		for (j = 0; j < mac->num_params; ++j) {
			if (strcmp(mac->body[i].text, mac->params[j]) == 0) {
				token tok;
				int k = 0;
				found = 1;
				while ((tok = args[j][k++]).type != eof) {
					expand_and_append(tok);
				}
				break;
			}
		}
		if (!found) {
			expand_and_append(mac->body[i]);
		}
	}
}

token_stream preprocess_stream(token_stream stream) {
	int i;
	token_stream out;
	out = stream_create();
	for (i = 0; i < stream.length; ++i) {
		token tok;
		tok = stream.tokens[i];
		/* comments should be stripped out by now,
		 * but if not, do nothing */
		if (tok.type == comment) continue;
		if (tok.type == identifier) {
			
		}
	}
}
#endif


token_stream parse_directive(token_stream line) {
	token directive;
	token_stream stream;
	directive = line.tokens[1];
	stream = stream_create();
	/* do nothing on empty directives */
	if (directive.type == newline) {
		return stream;
	}
	if (strcmp(directive.text, "include") == 0) {
		char *search_path, *path;
		token file_tok;
		int i, found = 0;
		file_tok = line.tokens[2];
		if (file_tok.type != string_literal) {
			issue_error("Expected filename after include directive", file_tok);
			return stream;
		}
		/* Here is lots of filesystem specific stuff.  TODO - maybe make this work on bad (non-unix-like) filesystems */
		if (file_tok.text[0] == '\"') {
			search_path = (char *)malloc((strlen(QUOTE_PATH) + strlen(BRACKET_PATH) + 2) * sizeof(char));
			strcpy(search_path, QUOTE_PATH ":" BRACKET_PATH);
		} else if (file_tok.text[0] == '<') {
			search_path = (char *)malloc((strlen(BRACKET_PATH) + 1) * sizeof(char));
			strcpy(search_path, BRACKET_PATH);
		} else {
			/* There was a tokenization error */
			return stream; 
		}
		path = strtok(search_path, ":");
		while (path) {
			int path_len, tok_len;
			FILE *include_file;
			char *full_path;
			path_len = strlen(path);
			tok_len = strlen(file_tok.text);
			full_path = (char *)malloc(path_len + tok_len);
			/* I didn't forget the null character.  tok.text has 2 extra quoting
			 * characters, and we are adding a delimiter between the 2 parts */
			memcpy(full_path, path, path_len);
			full_path[path_len] = '/';
			memcpy(&full_path[path_len+1], &file_tok.text[1], tok_len-2);
			full_path[path_len+tok_len-1] = '\0';
			include_file = fopen(full_path, "r");
			free(full_path);
			if (include_file) {
				free(stream.tokens);
				stream = preprocess(include_file);
				fclose(include_file);
				found = 1;
				break;
			}
			path = strtok(NULL, ":");
		}
		if (!found) {
			issue_error("File not found", file_tok);
		}
		free(search_path);
		
		for (i = 3; i < line.length - 1; ++i) {
			issue_error("Unexpected token", line.tokens[i]);
		}
	} else if (strcmp(directive.text, "define") == 0) {
		/* TODO - define directive   
		parse_macro_definition(file);
		*/
	/* Conditionals rely on our ability to parse and evaluate constant
	 * expressions, which we cannot do now.  TODO : implement #if, #else
	 * #endif, #ifdef, and #ifndef.*/	
	} else {
		issue_error("Unknown preprocessor directive", directive);
	}
	
	return stream;
	
}


token_stream preprocess(FILE *file) {
	token_stream line, out_stream;
	token *position;
	out_stream = stream_create();
	do {
		line = tokenize_line(file);
		if (strcmp(line.tokens[0].text, "#") == 0) {
			stream_cat(&out_stream, parse_directive(line));
		} else {
			stream_cat(&out_stream, line);
			/*
			int i;
			for (i = 0;  i < line.length; ++i) {
				
			}
			*/
		}
	} while (line.tokens[0].type != eof);
	return out_stream;
}



int main(int argc, char *argv[]) {
	token_stream stream;
	int i;
	stream = preprocess(stdin);
	for (i = 0; i < stream.length; ++i) {
		token tok;
		tok = stream.tokens[i];
		if (tok.preceding_whitespace) {
			printf(" ");
		}
		printf("%s", tok.text);
	}
}

/*
int main(int argc, char *argv[]) {
	
	token tok;
	while ((tok = next_token(stdin)).type != eof) {
		char *typeString;
		switch (tok.type) {
			case identifier:
				typeString = "Identifier";
				break;
			case number:
				typeString = "Number";
				break;
			case char_constant:
				typeString = "Character Constant";
				break;
			case string_literal:
				typeString = "String Literal";
				break;
			case symbol:
				typeString = "Symbol";
				break;
			case other:
				typeString = "Other";
				break;
			case eof:
				typeString = "End of File";
				break;
			default:
				typeString = "Unknown Token";
				break;
		}
		printf("%s : %s\n@%d:%d\n", tok.text, typeString, tok.line, tok.col);
	}
}


token *tokenize(FILE *file, int *count) {
	token tok;
	int buffer_size = 1024;
	int i = 0;
	token *token_buffer = (token *)malloc(buffer_size *  sizeof(token));
	do {
		tok = tok = next_token(file);
		if (i == buffer_size) {
			buffer_size *= 2;
			token_buffer = (token *)realloc(token_buffer, buffer_size * sizeof(token));
		}
		token_buffer[i++] = tok;
	} while (tok.type != eof);
	*count = i;
	return token_buffer;
}
*/



