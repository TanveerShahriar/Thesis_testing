import sys
import json
import re

def parse_call_graph(input_file):
    call_graph = {}

    with open(input_file, 'r') as file:
        lines = file.readlines()
        is_call_graph = False

        for line in lines:
            line = line.strip()
            if line == "--- Call graph Dump ---":
                is_call_graph = True
                continue

            if is_call_graph and line.startswith("Function:"):
                # Extract function and calls
                match = re.match(r"Function:\s+([^\s]+)\s+calls:\s+(.*)", line)
                if match:
                    function = match.group(1)
                    calls = match.group(2).split() if match.group(2) else []
                    call_graph[function] = calls

    return call_graph

def main():
    if len(sys.argv) != 3:
        print("Usage: python3 process_callgraph.py <input_file> <output_file>")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    call_graph = parse_call_graph(input_file)

    with open(output_file, 'w') as file:
        json.dump(call_graph, file, indent=4)

if __name__ == "__main__":
    main()
