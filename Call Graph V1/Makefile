# Variables
CPP_FILE=../Test\ Applications/test_input_1.cpp             # Source C++ file
RAW_OUTPUT=raw_callgraph.txt    # Temporary raw output file
FILTERED_OUTPUT=filtered_callgraph.txt # Filtered call graph file
OUTPUT_GRAPH=callgraph.json     # Final structured output file
CLANG=clang++                   # Use clang++ for proper C++ support
PROCESS_SCRIPT=process_callgraph.py # Script to format output
FILTER_SCRIPT=filter_callgraph.py # Script to filter call graph
CXXFLAGS=-std=c++17 -Wall       # Compiler flags
ANALYZE_FLAGS=-Xclang -analyze -Xclang -analyzer-checker=debug.DumpCallGraph # Analyzer flags

# Default target
all: generate_call_graph

# Generate the raw function call graph and process it
generate_call_graph: $(CPP_FILE)
	$(CLANG) $(CXXFLAGS) $(ANALYZE_FLAGS) -fsyntax-only $(CPP_FILE) > $(RAW_OUTPUT) 2>&1
	python $(FILTER_SCRIPT) $(RAW_OUTPUT) $(FILTERED_OUTPUT)
	python $(PROCESS_SCRIPT) $(FILTERED_OUTPUT) $(OUTPUT_GRAPH)
# @rm -f $(RAW_OUTPUT) $(FILTERED_OUTPUT)

# Clean up
clean:
	@rm -f $(RAW_OUTPUT) $(FILTERED_OUTPUT) $(OUTPUT_GRAPH)

# Rebuild
rebuild: clean all