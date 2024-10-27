%{

#include"symbol_info.h"
#include "symbol_table.h"
#include "adjacency_list.h"

#define YYSTYPE symbol_info*

int yyparse(void);
int yylex(void);

extern FILE *yyin;

ofstream outlog, param_header_file, result;

int lines = 1;
symbol_table st(10, &outlog);
vector<symbol_info *> params;
int param_count = 0;

AdjacencyList graph(&outlog);
vector<string> scope;

void yyerror(char *s) {
    outlog<<"At line "<<lines<<" "<<s<<endl<<endl;
}
%}

%token IF ELSE FOR WHILE DO BREAK INT CHAR FLOAT DOUBLE VOID RETURN SWITCH CASE DEFAULT CONTINUE PRINTLN ADDOP MULOP INCOP DECOP RELOP ASSIGNOP LOGICOP NOT LPAREN RPAREN LCURL RCURL LTHIRD RTHIRD COMMA SEMICOLON CONST_INT CONST_FLOAT ID

%nonassoc IFX
%nonassoc ELSE
%%

start : program
	{
		outlog<<"At line no: "<<lines<<" start : program "<<endl<<endl;
		outlog << "Symbol Table" << endl << endl;
		st.print_all_scopes();

		graph.displayAdjacencyList();

		result << $1->get_name() << endl;
	}
	;

program : program unit
	{
		outlog<<"At line no: "<<lines<<" program : program unit "<<endl<<endl;
		outlog<<$1->get_name()<<"\n"<<$2->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+"\n"+$2->get_name(),"program");
	}
	| unit
	{
		outlog<<"At line no: "<<lines<<" program : unit "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"program");
	}
	;

unit : var_declaration
	{
		outlog<<"At line no: "<<lines<<" unit : var_declaration "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;

		$$ = new symbol_info($1->get_name(), "unit");
		
	}
	| func_definition
	{
		outlog<<"At line no: "<<lines<<" unit : func_definition "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;

		$$ = new symbol_info($1->get_name(), "unit");
	}
	;
	
func_definition : type_specifier ID LPAREN parameter_list RPAREN 
	{
		$2->set_symbol_type("Function Definition");
		$2->set_return_type($1->get_name());

		stringstream ss($4->get_name());
		string token;
		while (getline(ss, token, ',')) {
        	$2->add_param_type(token);
    	}
		st.insert($2);

		scope.push_back($2->get_name());

		param_header_file << "struct " << $2->get_name() << "Params" << endl;
		param_header_file << "{" << endl;

		for (auto& param : $2->get_params()) {
        	param_header_file << param << ";" << endl;
    	}

		param_header_file << $1->get_name() << " res;" << endl;

		param_header_file << "};" << endl << endl;
	}
	compound_statement
	{	
		outlog<<"At line no: "<<lines<<" func_definition : type_specifier ID LPAREN parameter_list RPAREN compound_statement "<<endl<<endl;
		outlog<<$1->get_name()<<" "<<$2->get_name()<<"("<<$4->get_name()<<")\n"<<$7->get_name()<<endl<<endl;

		$$ = new symbol_info($1->get_name()+" "+$2->get_name()+"("+$4->get_name()+")\n"+$7->get_name(),"func_def");	

		result << "void* " << "thread" << $2->get_name() << "(void* params)" << endl;
		result << "{" << endl;
		result << "struct " << $2->get_name() << "Params*" << " args = (" << "struct " << $2->get_name() << "Params*) params;" << endl;
		result << $1->get_name() << " result = " << $2->get_name() << "(";

		auto params = $2->get_params();
		for (auto it = params.begin(); it != params.end(); ++it) {
			if (it != params.begin()) {
				result << ", ";
			}

			istringstream iss(*it);
			string variable;

			iss.ignore(numeric_limits<streamsize>::max(), ' ');

			iss >> variable;
			result << "args->" << variable;
		}

		result << ");" << endl;

		result << "args->res = result;" << endl;
		result << "}" << endl << endl;

		scope.pop_back();
	}
	| type_specifier ID LPAREN RPAREN
	{
		$2->set_symbol_type("Function Definition");
		$2->set_return_type($1->get_name());

		st.insert($2);
		scope.push_back($2->get_name());
	}
	compound_statement
	{
		
		outlog<<"At line no: "<<lines<<" func_definition : type_specifier ID LPAREN RPAREN compound_statement "<<endl<<endl;
		outlog<<$1->get_name()<<" "+$2->get_name()<<"()\n"<<$6->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+" "+$2->get_name()+"()\n"+$6->get_name(),"func_def");	

		scope.pop_back();
	}
	;
	
parameter_list	: parameter_list COMMA type_specifier ID
	{
		outlog<<"At line no: "<<lines<<" parameter_list	: parameter_list COMMA type_specifier ID "<<endl<<endl;
		outlog<<$1->get_name()<<","<<$3->get_name()<<" "<<$4->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+","+$3->get_name()+" "+$4->get_name(),"parameter_list");

		$4->set_symbol_type("Variable");
		$4->set_return_type($3->get_name());

		params.push_back($4);
		param_count++;
	}
	| parameter_list COMMA type_specifier
	{
		outlog<<"At line no: "<<lines<<" parameter_list	: parameter_list COMMA type_specifier "<<endl<<endl;
		outlog<<$1->get_name()<<","<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+","+$3->get_name(),"parameter_list");	
	}
	| type_specifier ID
	{
		outlog<<"At line no: "<<lines<<" parameter_list	: type_specifier ID "<<endl<<endl;
		outlog<<$1->get_name()<<" "<<$2->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+" "+$2->get_name(),"parameter_list");	

		$2->set_symbol_type("Variable");
		$2->set_return_type($1->get_name());

		params.push_back($2);
		param_count++;
	}
	| type_specifier
	{
		outlog<<"At line no: "<<lines<<" parameter_list	: type_specifier "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"parameter_list");	
	}
	;

compound_statement : LCURL 
	{
		st.enter_scope();
	}
	statements RCURL
	{
		outlog<<"At line no: "<<lines<<" compound_statement : LCURL statements RCURL "<<endl<<endl;
		outlog<<"{\n"+$3->get_name()+"\n}"<<endl<<endl;
		
		$$ = new symbol_info("{\n"+$3->get_name()+"\n}","compound_statement");

		if (param_count > 0) {
			for (auto param : params) {
				st.insert(param);
			}
			param_count = 0;
			params.clear();
		}

		st.print_all_scopes();
		st.exit_scope();
	}
	| LCURL 
	{
		st.enter_scope();
	}
	RCURL
	{
		outlog<<"At line no: "<<lines<<" compound_statement : LCURL RCURL "<<endl<<endl;
		outlog<<"{\n}"<<endl<<endl;
		
		$$ = new symbol_info("{\n}","compound_statement");

		st.print_all_scopes();
		st.exit_scope();
	}
	;

var_declaration : type_specifier declaration_list SEMICOLON
	{
		outlog<<"At line no: "<<lines<<" var_declaration : type_specifier declaration_list SEMICOLON "<<endl<<endl;
		outlog<<$1->get_name()<<" "<<$2->get_name()<<";"<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+" "+$2->get_name()+";","var_declaration");

		stringstream ss_var($2->get_name());
		string token_var;
		while (getline(ss_var, token_var, ',')) {
			symbol_info *func = new symbol_info(token_var, "ID");

			size_t index_lthird = token_var.find("[");
			size_t index_rthird = token_var.find("]");
			if (index_lthird != string::npos) {
				func->set_name(token_var.substr(0, index_lthird));
				func->set_symbol_type("Array");
				func->set_return_type($1->get_name());

				string s = token_var.substr(index_lthird + 1, index_rthird - index_lthird - 1);
				func->set_size(stoi(s));
			} else {
				func->set_symbol_type("Variable");
				func->set_return_type($1->get_name());
			}

			st.insert(func);
    	}
	}
	;

type_specifier : INT
	{
		outlog<<"At line no: "<<lines<<" type_specifier : INT "<<endl<<endl;
		outlog<<"int"<<endl<<endl;
		
		$$ = new symbol_info("int","type_specifier");
	
	}
	| FLOAT
	{
		outlog<<"At line no: "<<lines<<" type_specifier : FLOAT "<<endl<<endl;
		outlog<<"float"<<endl<<endl;
		
		$$ = new symbol_info("float","type_specifier");
	
	}
	| VOID
	{
		outlog<<"At line no: "<<lines<<" type_specifier : VOID "<<endl<<endl;
		outlog<<"void"<<endl<<endl;
		
		$$ = new symbol_info("void","type_specifier");
	}
	;

declaration_list : declaration_list COMMA ID
	{
		outlog<<"At line no: "<<lines<<" declaration_list : declaration_list COMMA ID "<<endl<<endl;
		outlog<<$1->get_name()<<","<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+","+$3->get_name(),"declaration_list");
	}
	| declaration_list COMMA ID LTHIRD CONST_INT RTHIRD
	{
		outlog<<"At line no: "<<lines<<" declaration_list : declaration_list COMMA ID LTHIRD CONST_INT RTHIRD "<<endl<<endl;
		outlog<<$1->get_name()+","<<$3->get_name()<<"["<<$5->get_name()<<"]"<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+","+$3->get_name()+"["+$5->get_name()+"]","declaration_list");
	}
	| ID
	{
		outlog<<"At line no: "<<lines<<" declaration_list : ID "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"declaration_list");
	}
	| ID LTHIRD CONST_INT RTHIRD
	{
		outlog<<"At line no: "<<lines<<" declaration_list : ID LTHIRD CONST_INT RTHIRD "<<endl<<endl;
		outlog<<$1->get_name()<<"["<<$3->get_name()<<"]"<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+"["+$3->get_name()+"]","declaration_list");
	}
	;

statements : statement
	{
		outlog<<"At line no: "<<lines<<" statements : statement "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"statements");
	}
	| statements statement
	{
		outlog<<"At line no: "<<lines<<" statements : statements statement "<<endl<<endl;
		outlog<<$1->get_name()<<"\n"<<$2->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+"\n"+$2->get_name(),"statements");
	}
	;

statement : var_declaration
	{
		outlog<<"At line no: "<<lines<<" statement : var_declaration "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"statement");
	}
	| expression_statement
	{
		outlog<<"At line no: "<<lines<<" statement : expression_statement "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"statement");
	}
	| compound_statement
	{
		outlog<<"At line no: "<<lines<<" statement : compound_statement "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"statement");
	}
	| FOR LPAREN expression_statement expression_statement expression RPAREN statement
	{
		outlog<<"At line no: "<<lines<<" statement : FOR LPAREN expression_statement expression_statement expression RPAREN statement "<<endl<<endl;
		outlog<<"for("<<$3->get_name()<<$4->get_name()<<$5->get_name()<<")\n"<<$7->get_name()<<endl<<endl;
		
		$$ = new symbol_info("for("+$3->get_name()+$4->get_name()+$5->get_name()+")\n"+$7->get_name(),"statement");
	}
	| IF LPAREN expression RPAREN statement %prec IFX
	{
		outlog<<"At line no: "<<lines<<" statement : IF LPAREN expression RPAREN statement "<<endl<<endl;
		outlog<<"if("<<$3->get_name()<<")\n"<<$5->get_name()<<endl<<endl;
	
		$$ = new symbol_info("if("+$3->get_name()+")\n"+$5->get_name(),"statement");
	}
	| IF LPAREN expression RPAREN statement ELSE statement
	{
		outlog<<"At line no: "<<lines<<" statement : IF LPAREN expression RPAREN statement ELSE statement "<<endl<<endl;
		outlog<<"if("<<$3->get_name()<<")\n"<<$5->get_name()<<"\nelse\n"<<$7->get_name()<<endl<<endl;
	
		$$ = new symbol_info("if("+$3->get_name()+")\n"+$5->get_name()+"\nelse\n"+$7->get_name(),"statement");
	}
	| WHILE LPAREN expression RPAREN statement
	{
		outlog<<"At line no: "<<lines<<" statement : WHILE LPAREN expression RPAREN statement "<<endl<<endl;
		outlog<<"while("<<$3->get_name()<<")\n"<<$5->get_name()<<endl<<endl;
		
		$$ = new symbol_info("while("+$3->get_name()+")\n"+$5->get_name(),"statement");
	}
	| PRINTLN LPAREN ID RPAREN SEMICOLON
	{
		outlog<<"At line no: "<<lines<<" statement : PRINTLN LPAREN ID RPAREN SEMICOLON "<<endl<<endl;
		outlog<<"printf("<<$3->get_name()<<");"<<endl<<endl; 
		
		$$ = new symbol_info("printf("+$3->get_name()+");","statement");
	}
	| RETURN expression SEMICOLON
	{
		outlog<<"At line no: "<<lines<<" statement : RETURN expression SEMICOLON "<<endl<<endl;
		outlog<<"return "<<$2->get_name()<<";"<<endl<<endl;
		
		$$ = new symbol_info("return "+$2->get_name()+";","statement");
	}
	;

expression_statement : SEMICOLON
	{
		outlog<<"At line no: "<<lines<<" expression_statement : SEMICOLON "<<endl<<endl;
		outlog<<";"<<endl<<endl;

		$$ = new symbol_info(";","expression_statement");
	}
	| expression SEMICOLON
	{
		outlog<<"At line no: "<<lines<<" expression_statement : expression SEMICOLON "<<endl<<endl;
		outlog<<$1->get_name()<<";"<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+";","expression_statement");
	}
	;

variable : ID
	{
		outlog<<"At line no: "<<lines<<" variable : ID "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"variable");
	}
	| ID LTHIRD expression RTHIRD
	{
		outlog<<"At line no: "<<lines<<" variable : ID LTHIRD expression RTHIRD "<<endl<<endl;
		outlog<<$1->get_name()<<"["<<$3->get_name()<<"]"<<endl<<endl;
	
		$$ = new symbol_info($1->get_name()+"["+$3->get_name()+"]","variable");
	}
	;

expression : logic_expression
	{
		outlog<<"At line no: "<<lines<<" expression : logic_expression "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"expression");
	}
	| variable ASSIGNOP logic_expression
	{
		outlog<<"At line no: "<<lines<<" expression : variable ASSIGNOP logic_expression "<<endl<<endl;
		outlog<<$1->get_name()<<"="<<$3->get_name()<<endl<<endl;

		$$ = new symbol_info($1->get_name()+"="+$3->get_name(),"expression");
		if ($3->get_symbol_type() == "function")
		{
			string expr = "";
			expr += "struct " + $3->get_struct_name() + "Params* " + $3->get_struct_name() + "Args = ";
			expr += "(struct " + $3->get_struct_name() + "Params*) malloc(sizeof(struct ";
			expr += $3->get_struct_name() + "Params));\n";
			
			vector<string> arg;
			stringstream temp($3->get_arguments());
			string part;

			while (getline(temp, part, ',')) {
				arg.push_back(part);
			}

			vector<string> temp_params = $3->get_params();
			for (size_t i = 0; i < arg.size(); ++i)
			{
				istringstream iss(temp_params[i]);
				string variable;

				iss.ignore(numeric_limits<streamsize>::max(), ' ');

				iss >> variable;
				expr += $3->get_struct_name() + "Args->" + variable + " = " + arg[i] + ";\n";
			}

			expr += "pthread_t " + $3->get_struct_name() + "Thread;\n";
			expr += "pthread_create(&" + $3->get_struct_name() + "Thread, NULL, ";
			expr += "&thread" + $3->get_struct_name() + ", " + $3->get_struct_name() + "Args);\n";

			expr += "pthread_join(" + $3->get_struct_name() + "Thread, NULL);\n";
			expr += $1->get_name() + " = " + $3->get_struct_name() + "Args->res";
			$$->set_name(expr);
		}
	}
	;

logic_expression : rel_expression
	{
		outlog<<"At line no: "<<lines<<" logic_expression : rel_expression "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"logic_expression");
		if ($1->get_symbol_type() == "function")
		{
			$$->set_symbol_type("function");
			$$->set_struct_name($1->get_struct_name());
			$$->set_arguments($1->get_arguments());
			for(auto param : $1->get_params())
			{
				$$->add_param_type(param);
			}
		}
	}
	| rel_expression LOGICOP rel_expression
	{
		outlog<<"At line no: "<<lines<<" logic_expression : rel_expression LOGICOP rel_expression "<<endl<<endl;
		outlog<<$1->get_name()<<$2->get_name()<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+$2->get_name()+$3->get_name(),"logic_expression");
	}
	;

rel_expression : simple_expression
	{
		outlog<<"At line no: "<<lines<<" rel_expression : simple_expression "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"rel_expression");
		if ($1->get_symbol_type() == "function")
		{
			$$->set_symbol_type("function");
			$$->set_struct_name($1->get_struct_name());
			$$->set_arguments($1->get_arguments());
			for(auto param : $1->get_params())
			{
				$$->add_param_type(param);
			}
		}
	}
	| simple_expression RELOP simple_expression
	{
		outlog<<"At line no: "<<lines<<" rel_expression : simple_expression RELOP simple_expression "<<endl<<endl;
		outlog<<$1->get_name()<<$2->get_name()<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+$2->get_name()+$3->get_name(),"rel_expression");
	}
	;

simple_expression : term
	{
		outlog<<"At line no: "<<lines<<" simple_expression : term "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"simple_expression");
		if ($1->get_symbol_type() == "function")
		{
			$$->set_symbol_type("function");
			$$->set_struct_name($1->get_struct_name());
			$$->set_arguments($1->get_arguments());
			for(auto param : $1->get_params())
			{
				$$->add_param_type(param);
			}
		}
	}
	| simple_expression ADDOP term
	{
		outlog<<"At line no: "<<lines<<" simple_expression : simple_expression ADDOP term "<<endl<<endl;
		outlog<<$1->get_name()<<$2->get_name()<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+$2->get_name()+$3->get_name(),"simple_expression");
	}
	;

term : unary_expression
	{
		outlog<<"At line no: "<<lines<<" term : unary_expression "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"term");
		if ($1->get_symbol_type() == "function")
		{
			$$->set_symbol_type("function");
			$$->set_struct_name($1->get_struct_name());
			$$->set_arguments($1->get_arguments());
			for(auto param : $1->get_params())
			{
				$$->add_param_type(param);
			}
		}
	}
	| term MULOP unary_expression
	{
		outlog<<"At line no: "<<lines<<" term : term MULOP unary_expression "<<endl<<endl;
		outlog<<$1->get_name()<<$2->get_name()<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+$2->get_name()+$3->get_name(),"term");
	}
	;

unary_expression : ADDOP unary_expression
	{
		outlog<<"At line no: "<<lines<<" unary_expression : ADDOP unary_expression "<<endl<<endl;
		outlog<<$1->get_name()<<$2->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+$2->get_name(),"unary_expression");
	}
	| NOT unary_expression
	{
		outlog<<"At line no: "<<lines<<" unary_expression : NOT unary_expression "<<endl<<endl;
		outlog<<"!"<<$2->get_name()<<endl<<endl;
		
		$$ = new symbol_info("!"+$2->get_name(),"unary_expression");
	}
	| factor
	{
		outlog<<"At line no: "<<lines<<" unary_expression : factor "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"unary_expression");
		if ($1->get_symbol_type() == "function")
		{
			$$->set_symbol_type("function");
			$$->set_struct_name($1->get_struct_name());
			$$->set_arguments($1->get_arguments());
			for(auto param : $1->get_params())
			{
				$$->add_param_type(param);
			}
		}
	}
	;

factor : variable
	{
		outlog<<"At line no: "<<lines<<" factor : variable "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"factor");
	}
	| ID LPAREN argument_list RPAREN
	{
		outlog<<"At line no: "<<lines<<" factor : ID LPAREN argument_list RPAREN "<<endl<<endl;
		outlog<<$1->get_name()<<"("<<$3->get_name()<<")"<<endl<<endl;

		$$ = new symbol_info($1->get_name()+"("+$3->get_name()+")","factor");
		$$->set_symbol_type("function");
		$$->set_struct_name($1->get_name());
		$$->set_arguments($3->get_name());
		symbol_info* foundSymbol = st.lookup($1);
		for(auto param : foundSymbol->get_params())
		{
			$$->add_param_type(param);
		}

		graph.addEdge(scope.back(), $1->get_name());
	}
	| LPAREN expression RPAREN
	{
		outlog<<"At line no: "<<lines<<" factor : LPAREN expression RPAREN "<<endl<<endl;
		outlog<<"("<<$2->get_name()<<")"<<endl<<endl;
	
		$$ = new symbol_info("("+$2->get_name()+")","factor");
	}
	| CONST_INT
	{
		outlog<<"At line no: "<<lines<<" factor : CONST_INT "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"factor");
	}
	| CONST_FLOAT
	{
		outlog<<"At line no: "<<lines<<" factor : CONST_FLOAT "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"factor");
	}
	| variable INCOP
	{
		outlog<<"At line no: "<<lines<<" factor : variable INCOP "<<endl<<endl;
		outlog<<$1->get_name()<<"++"<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+"++","factor");
	}
	| variable DECOP
	{
		outlog<<"At line no: "<<lines<<" factor : variable DECOP "<<endl<<endl;
		outlog<<$1->get_name()<<"--"<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+"--","factor");
	}
	;

argument_list : arguments
	{
		outlog<<"At line no: "<<lines<<" argument_list : arguments "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"argument_list");
	}
	|
	{
		outlog<<"At line no: "<<lines<<" argument_list :  "<<endl<<endl;
		outlog<<""<<endl<<endl;
					
		$$ = new symbol_info("","argument_list");
	}
	;

arguments : arguments COMMA logic_expression
	{
		outlog<<"At line no: "<<lines<<" arguments : arguments COMMA logic_expression "<<endl<<endl;
		outlog<<$1->get_name()<<","<<$3->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name()+","+$3->get_name(),"arguments");
	}
	| logic_expression
	{
		outlog<<"At line no: "<<lines<<" arguments : logic_expression "<<endl<<endl;
		outlog<<$1->get_name()<<endl<<endl;
		
		$$ = new symbol_info($1->get_name(),"arguments");
	}
	;

%%

int main(int argc, char *argv[])
{
	if(argc != 2) 
	{
		cout<<"Please input file name"<<endl;
		return 0;
	}

	yyin = fopen(argv[1], "r");
	outlog.open("my_log.txt", ios::trunc);
	param_header_file.open("param_struct.h", ios::trunc);
	result.open("result.c", ios::trunc);

	param_header_file << "#ifndef PARAM_HEADER_H" << endl;
	param_header_file << "#define PARAM_HEADER_H" << endl << endl;

	result << "#include <stdio.h>" << endl;
	result << "#include <pthread.h>" << endl;
	result << "#include \"param_struct.h\"" << endl;
	result << endl;

	st.enter_scope();
	scope.push_back("Global");
	
	if(yyin == NULL)
	{
		cout<<"Couldn't open file"<<endl;
		return 0;
	}
    
	yyparse();
	
	//print number of lines
	outlog<<"Total lines: "<<lines;
	
	param_header_file << "#endif" << endl;

	outlog.close();
	param_header_file.close();
	result.close();
	
	fclose(yyin);
	
	return 0;
}