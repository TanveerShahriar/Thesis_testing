#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/CommandLine.h"
#include "clang/AST/Decl.h"  // Include for FunctionDecl
#include "clang/AST/Expr.h"


using namespace clang::tooling;
using namespace clang::ast_matchers;
using namespace llvm;

// Apply a custom category to all command-line options
static cl::OptionCategory MyToolCategory("function-name-printer options");
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp("\nA tool to print all function names in a source file\n");

// Define the matcher to identify all functions
DeclarationMatcher FunctionMatcher = functionDecl(
    isDefinition(),
    unless(isExpansionInSystemHeader()), // Ignore functions defined in system headers
    hasParent(translationUnitDecl())     // Ensure the function is at the top level (not nested or within a library class/namespace)
).bind("function");

// Define a callback to handle matched nodes
class FunctionPrinter : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult &Result) override {
        if (const auto *Func = Result.Nodes.getNodeAs<clang::FunctionDecl>("function")) {  // Correct use of FunctionDecl
            llvm::outs() << "Function Name: " << Func->getNameAsString() << "\n";
        }
    }
};

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    CommonOptionsParser &OptionsParser = ExpectedParser.get();
    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());

    FunctionPrinter Printer;
    MatchFinder Finder;
    Finder.addMatcher(FunctionMatcher, &Printer);

    return Tool.run(newFrontendActionFactory(&Finder).get());
}