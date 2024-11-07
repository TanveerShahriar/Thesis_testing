# Makefile for Yacc and Flex project

# Compiler and flags
CC = g++
CFLAGS = -fpermissive -w
YACC = yacc
LEX = flex

# Files
YACC_FILE = y.y
LEX_FILE = lex_analyzer.l
YACC_OUTPUT = y.tab.c
LEX_OUTPUT = lex.yy.c
YACC_OBJECT = y.o
LEX_OBJECT = l.o
EXECUTABLE = parser
OUTPUT_FILE = my_log.txt
OUTPUT_C_FILE = result.c
PARAM_STRUCT_FILE = param_struct.h

# Default rule
all: $(EXECUTABLE)

# Compile Yacc
$(YACC_OUTPUT): $(YACC_FILE)
	$(YACC) -d -y --debug --verbose $(YACC_FILE)

# Compile Lex
$(LEX_OUTPUT): $(LEX_FILE)
	$(LEX) $(LEX_FILE)

# Compile Yacc object
$(YACC_OBJECT): $(YACC_OUTPUT)
	$(CC) -w -c -o $(YACC_OBJECT) $(YACC_OUTPUT)

# Compile Lex object
$(LEX_OBJECT): $(LEX_OUTPUT)
	$(CC) $(CFLAGS) -c -o $(LEX_OBJECT) $(LEX_OUTPUT)

# Link objects and create executable
$(EXECUTABLE): $(YACC_OBJECT) $(LEX_OBJECT)
	$(CC) $(YACC_OBJECT) $(LEX_OBJECT) -o $(EXECUTABLE)

# Clean up generated files
clean:
	rm -f $(YACC_OUTPUT) $(LEX_OUTPUT) $(YACC_OBJECT) $(LEX_OBJECT) $(OUTPUT_FILE) $(EXECUTABLE).exe ${OUTPUT_C_FILE} ${PARAM_STRUCT_FILE} y.output y.tab.h

# Rebuild the project
rebuild: clean all
