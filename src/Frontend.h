#include <cstdio>

extern "C" {
	extern FILE *yyin;
	int yyparse();
	int yylex (void);
	void yyerror(const char *);
}
