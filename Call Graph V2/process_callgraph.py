import sys
import json

def parse_call_graph(text):
    lines = text.strip().splitlines()
    graph = {}
    current_func = None
    
    for line in lines:
        line = line.strip()
        if not line:
            # Skip empty lines
            continue
        
        if line.endswith("calls:"):
            # This line indicates the start of a new function block, e.g. "f34 calls:"
            current_func = line.split()[0]
            graph[current_func] = []
        else:
            # Otherwise, it should be one of the called functions
            graph[current_func].append(line)
    
    return graph

def main():
    if len(sys.argv) < 3:
        print("Usage: python parse_call_graph.py <input_file> <output_file>")
        sys.exit(1)
        
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    
    # Read the call-graph text from the input file
    with open(input_file, 'r') as f:
        call_graph_text = f.read()
    
    # Parse the text to build the call graph dictionary
    graph_dict = parse_call_graph(call_graph_text)
    
    # Convert the dictionary to a JSON string
    graph_json = json.dumps(graph_dict, indent=2)
    
    # Write the JSON string to the output file
    with open(output_file, 'w') as f:
        f.write(graph_json)

if __name__ == "__main__":
    main()