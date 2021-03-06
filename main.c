#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>

#include "debug.h"
#include "vm.h"
#include "object.h"
#include "list.h"
#include "bytecode_parser.h"
// #include "grammar.tab.h"
#include "lexer.h"
#include "ast_nodes.h"
#include "bytecodes.h"

extern List* programBlock;

#define EXIT_SUCCESS   0
#define EXIT_PROG_ERR  1
#define EXIT_BAD_ARGS  2
#define EXIT_EXCEPTION 3

// from 'debug.h'
extern int debug_flag;

void print_usage (FILE* stream, char* program_name) {
	fprintf (stream, "Usage:  %s INPUTFILES [options]\n", program_name);
	fprintf (stream,
		"  -h  --help             Display this usage information\n"
		"  -s  --stdin            Use stdin as the input stream\n"
	    "  -b  --bytecode         Print the bytecode representation and exit\n"
	    "  -t  --tree             Print the Abstract Syntax Tree and exit\n"
#ifndef NDEBUG
		"  -v  --verbose          Print verbose messages.\n"
#endif
	);
#ifndef NDEBUG
	fprintf(stream, "This is a debug version of the program\n");
#endif
}

const struct option long_options[] = {
	{"help",     0, NULL, 'h'},
	{"stdin",    0, NULL, 's'},
	{"bytecode", 0, NULL, 'b'},
	{"tree",     0, NULL, 't'},
#ifndef NDEBUG
	{"verbose",  0, NULL, 'v'},
#endif
	{NULL,       0, NULL, 0  }
};

/*
 * Program entry point. Parse arguments, then handle highest-level control flow.
*/
int main(int argc, char *argv[]) {
	bool bytecode_flag = false;
	bool tree_flag = false;
	bool use_stdin = false;
	debug_flag = 0;

	char ch;
	while ((ch = getopt_long(argc, argv, "hsbtv", long_options, NULL)) != -1) {
		switch (ch) {
			case 'h':
				print_usage(stderr, argv[0]);
				exit(EXIT_SUCCESS);
			case '?':
				print_usage(stderr, argv[0]);
				exit(EXIT_BAD_ARGS);
			case 's':
				use_stdin = true;
				break;
			case 'b':
				bytecode_flag = true;
				break;
			case 't':
				tree_flag = true;
				break;
#ifndef NDEBUG
			case 'v':
				debug_flag = 1;
				debug("DEBUG is on");
				break;
#endif
			default:
				if (ch == 'v') {
					fprintf(stderr, "This executable doesn't include debugging utilties\n");
					exit(EXIT_BAD_ARGS);
				} else {
					fprintf(stderr, "'getopt_long' returned unknown '%c'\n", ch);
					exit(EXIT_PROG_ERR);
				}
				break;
		}
	}

	FILE* input;

	if (use_stdin) {
		debug("using 'stdin' as input");
		input = stdin;
	} else {
		int filec = argc - optind;

		if (filec == 0) {
			fprintf(stderr, "No input file given\n");
			print_usage(stderr, argv[0]);
			exit(EXIT_BAD_ARGS);
		}

		if (filec > 1) {
			log_warn("more than one input file given--ignoring all but first");
		}

		debug("reading file '%s'", argv[optind]);
		input = fopen(argv[optind], "r");
	}

	Derp_vm* vm = derp_vm_create();

	yyin = input;
	do {
		yyparse();
	} while (!feof(yyin));

	if (tree_flag) {
		debug("Printing AST...");
		ast_list_print(programBlock, 0);
		return EXIT_SUCCESS;
	}

	file_blob_t blob;
	file_blob_init(&blob, argv[optind]);

	check(ast_compile(programBlock, &blob), "Failed to compile AST");

	vm->blob = &blob;

	if (bytecode_flag) {
		debug("Printing bytecodes...");
		file_blob_print(&blob);
		return EXIT_SUCCESS;
	}

	derp_vm_run(vm);

	derp_vm_destroy(vm);

	return EXIT_SUCCESS;
error:
	return EXIT_PROG_ERR;
}
