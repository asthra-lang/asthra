# Parser API

**Core Functions**

```c
// Parser lifecycle
ParserContext* parser_create(LexerContext* lexer);
void parser_destroy(ParserContext* ctx);

// Parsing
ASTNode* parser_parse_program(ParserContext* ctx);
ASTNode* parser_parse_expression(ParserContext* ctx);
ASTNode* parser_parse_statement(ParserContext* ctx);

// Error handling
bool parser_has_error(ParserContext* ctx);
const char* parser_get_error(ParserContext* ctx);
void parser_get_error_location(ParserContext* ctx, size_t* line, size_t* column);

// Utility
void parser_advance(ParserContext* ctx);
bool parser_match(ParserContext* ctx, TokenType type);
bool parser_consume(ParserContext* ctx, TokenType type);
``` 