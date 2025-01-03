#include <clang/AST/AST.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/Frontend/FrontendActions.h>
#include <clang/Tooling/CommonOptionsParser.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>
#include <unordered_map>
#include <vector>
#include <string>

using namespace clang;
using namespace clang::ast_matchers;

static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// Just a global adjacency list for demonstration
static std::unordered_map<std::string, std::vector<std::string>> CallGraph;

class MyCallHandler : public MatchFinder::MatchCallback {
public:
  void run(const MatchFinder::MatchResult &Result) override {
    const auto *Call = Result.Nodes.getNodeAs<CallExpr>("callSite");
    const auto *Caller = Result.Nodes.getNodeAs<FunctionDecl>("caller");
    const auto *Callee = Result.Nodes.getNodeAs<FunctionDecl>("callee");

    if (!Call || !Caller || !Callee)
      return;

    // For older Clang: no isInSystemHeader(). Use SourceManager check:
    const SourceManager &SM = *Result.SourceManager;
    if (Callee->isImplicit() ||
        SM.isInSystemHeader(Callee->getLocation()) ||
        !Callee->isUserProvided()) {
      return;
    }

    std::string callerName = Caller->getQualifiedNameAsString();
    std::string calleeName = Callee->getQualifiedNameAsString();
    CallGraph[callerName].push_back(calleeName);
  }
};

int main(int argc, const char **argv) {
  // Use the new factory method for CommonOptionsParser in modern Clang:
  auto ExpectedParser =
      clang::tooling::CommonOptionsParser::create(argc, argv, MyToolCategory);
  if (!ExpectedParser) {
    llvm::errs() << "Error creating CommonOptionsParser: "
                 << llvm::toString(ExpectedParser.takeError()) << "\n";
    return 1;
  }

  clang::tooling::CommonOptionsParser &OptionsParser = ExpectedParser.get();

  clang::tooling::ClangTool Tool(OptionsParser.getCompilations(),
                                 OptionsParser.getSourcePathList());

  MyCallHandler Handler;
  MatchFinder Finder;

  Finder.addMatcher(
    callExpr(
      callee(functionDecl().bind("callee")),
      hasAncestor(functionDecl(
        isDefinition(),
        unless(isImplicit()),
        // Skip calls inside system headers:
        unless(isExpansionInSystemHeader())
      ).bind("caller"))
    ).bind("callSite"),
    &Handler
  );

  // Run the tool
  int Result = Tool.run(clang::tooling::newFrontendActionFactory(&Finder).get());

  // Dump the adjacency list
  for (auto &kv : CallGraph) {
    llvm::outs() << kv.first << " calls:\n";
    for (auto &callee : kv.second) {
      llvm::outs() << "  " << callee << "\n";
    }
  }
  return Result;
}