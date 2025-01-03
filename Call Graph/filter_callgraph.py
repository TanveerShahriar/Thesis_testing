import sys
import re

def filter_callgraph(input_file, output_file):
    with open(input_file, 'r') as infile, open(output_file, 'w') as outfile:
        lines = infile.readlines()
        in_function = False
        current_function = None

        for line in lines:
            # Detect function definition line
            if line.startswith("  Function:"):
                function_name = line.split("Function:")[1].split("calls:")[0].strip()
                if not function_name.startswith("std::") and not function_name.startswith("__"):
                    in_function = True
                    current_function = function_name
                    outfile.write(f"Function: {current_function} calls:\n")
                else:
                    in_function = False

            # Write call graph for functions defined in the file
            elif in_function and "calls:" in line:
                outfile.write(line)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python filter_callgraph.py <input_file> <output_file>")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    filter_callgraph(input_file, output_file)