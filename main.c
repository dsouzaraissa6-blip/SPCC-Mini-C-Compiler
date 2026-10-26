#include "codegen.h"
#include "compiler.h"
#include "ir.h"
#include "optimizer.h"
#include "symbol_table.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define make_directory(path) _mkdir(path)
#else
#define make_directory(path) mkdir(path, 0777)
#endif

int yyparse(void);

int compiler_errors;
FILE *compiler_report;
static int diagnostics_heading_written;

static int ensure_output_directory(void)
{
    struct stat info;
    if (stat("output", &info) == 0) {
        if (S_ISDIR(info.st_mode)) {
            return 1;
        }
        fprintf(stderr, "Cannot create output folder: 'output' exists and is not a directory.\n");
        return 0;
    }
    if (errno != ENOENT) {
        perror("output");
        return 0;
    }
    if (make_directory("output") != 0) {
        perror("output");
        return 0;
    }
    return 1;
}

void compiler_error(int line, const char *format, ...)
{
    va_list arguments;
    fprintf(stderr, "line %d: error: ", line);
    va_start(arguments, format);
    vfprintf(stderr, format, arguments);
    va_end(arguments);
    fputc('\n', stderr);

    if (compiler_report != NULL) {
        if (!diagnostics_heading_written) {
            fputs("\nDIAGNOSTICS\n", compiler_report);
            diagnostics_heading_written = 1;
        }
        fprintf(compiler_report, "line %d: error: ", line);
        va_start(arguments, format);
        vfprintf(compiler_report, format, arguments);
        va_end(arguments);
        fputc('\n', compiler_report);
    }
    compiler_errors++;
}

void compiler_record_token(const char *category, const char *lexeme, int line)
{
    if (compiler_report != NULL) {
        fprintf(compiler_report, "%-6d %-24s %s\n", line, lexeme, category);
    }
}

static void report_assembly(FILE *report, const char *path)
{
    FILE *assembly = fopen(path, "r");
    if (assembly == NULL) {
        fprintf(report, "Could not reopen generated assembly file '%s'.\n", path);
        return;
    }
    fputs("\nPHASE 6: TARGET CODE GENERATION - X86-32 ASSEMBLY\n", report);
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), assembly) != NULL) {
        fputs(buffer, report);
    }
    fclose(assembly);
}

int main(int argc, char **argv)
{
    if (argc > 4) {
        fprintf(stderr, "usage: %s [input.c [output.asm [report.txt]]]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *source_path = argc >= 2 ? argv[1] : "input.c";
    const char *assembly_path = argc >= 3 ? argv[2] : "output/output.asm";
    const char *report_path = argc >= 4 ? argv[3] : "output/output.txt";

    if (!ensure_output_directory()) {
        return EXIT_FAILURE;
    }

    FILE *source = fopen(source_path, "r");
    if (source == NULL) {
        perror(source_path);
        return EXIT_FAILURE;
    }

    compiler_report = fopen(report_path, "w");
    if (compiler_report == NULL) {
        perror(report_path);
        fclose(source);
        return EXIT_FAILURE;
    }
    fprintf(compiler_report, "MINI C COMPILER - COMPILATION REPORT\nSource file: %s\n",
            source_path);
    fputs("\nPHASE 1: LEXICAL ANALYSIS - TOKEN TABLE\n", compiler_report);
    fputs("Line   Lexeme                   Token category\n", compiler_report);
    fputs("------ ------------------------ ----------------------\n", compiler_report);

    yyin = source;
    compiler_errors = 0;
    diagnostics_heading_written = 0;
    symbol_table_init();
    ir_init();

    int parse_status = yyparse();
    fclose(source);
    fputs("\nPHASE 2: SYNTAX ANALYSIS\n", compiler_report);
    if (parse_status != 0) {
        fputs("Parsing failed.\n", compiler_report);
        symbol_print_tables(compiler_report);
        fprintf(compiler_report, "\nCOMPILATION FAILED (%d error(s)).\n",
                compiler_errors);
        fprintf(stderr, "Compilation failed with %d error(s). Report: %s\n",
                compiler_errors, report_path);
        fclose(compiler_report);
        compiler_report = NULL;
        return EXIT_FAILURE;
    }
    fputs("Parsing successful: source matches the supported grammar.\n",
          compiler_report);

    fputs("\nPHASE 3: SEMANTIC ANALYSIS - SYMBOL TABLES\n", compiler_report);
    ir_validate_calls();
    symbol_print_tables(compiler_report);
    if (compiler_errors != 0) {
        fprintf(compiler_report, "\nCOMPILATION FAILED (%d semantic error(s)).\n",
                compiler_errors);
        fprintf(stderr, "Compilation failed with %d semantic error(s). Report: %s\n",
                compiler_errors, report_path);
        fclose(compiler_report);
        compiler_report = NULL;
        return EXIT_FAILURE;
    }

    fputs("\nSemantic checks passed.\n", compiler_report);
    ir_print(compiler_report, "\nPHASE 4: INTERMEDIATE CODE - THREE-ADDRESS CODE");
    optimize_ir();
    ir_print(compiler_report, "\nPHASE 5: OPTIMIZATION - CONSTANT FOLDING");

    FILE *assembly = fopen(assembly_path, "w");
    if (assembly == NULL) {
        perror(assembly_path);
        fprintf(compiler_report, "\nCOMPILATION FAILED: cannot create %s.\n",
                assembly_path);
        fclose(compiler_report);
        compiler_report = NULL;
        return EXIT_FAILURE;
    }
    int generated = generate_assembly(assembly);
    int close_status = fclose(assembly);
    if (close_status != 0) {
        perror(assembly_path);
        fclose(compiler_report);
        compiler_report = NULL;
        return EXIT_FAILURE;
    }
    if (!generated || compiler_errors != 0) {
        fprintf(compiler_report, "\nCOMPILATION FAILED during target code generation.\n");
        fprintf(stderr, "Compilation failed during target code generation. Report: %s\n",
                report_path);
        fclose(compiler_report);
        compiler_report = NULL;
        return EXIT_FAILURE;
    }
    report_assembly(compiler_report, assembly_path);
    fprintf(compiler_report,
            "\nCOMPILATION SUCCEEDED\nAssembly file: %s\n", assembly_path);
    fclose(compiler_report);
    compiler_report = NULL;
    printf("Compilation succeeded.\nFull report: %s\nAssembly: %s\n",
           report_path, assembly_path);
    return EXIT_SUCCESS;
}
