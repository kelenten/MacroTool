// MacroTool.cpp: 定义应用程序的入口点。
// 
// Clang 前端和编译器基础
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"

// Clang 工具和解析器选项
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"

// Clang 词法分析器
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/Token.h"
#include "clang/Lex/MacroArgs.h"

// Clang 抽象语法树（AST）
#include <clang/AST/RecursiveASTVisitor.h>
#include "clang/AST/ASTConsumer.h"
#include <clang/AST/TypeLoc.h>

// Clang 源码管理和诊断
#include <clang/Basic/SourceManager.h>

// Clang 代码重写工具
#include <clang/Rewrite/Core/Rewriter.h>

// LLVM 支持库
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/raw_ostream.h"

// 标准库头文件
#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "../TestTool/FilePreprocessor.cpp"
#include "D:/LLVM/新建文件夹/json-develop/single_include/nlohmann/json.hpp"

using namespace clang;
using namespace clang::tooling;
using namespace llvm;

std::vector<std::pair<std::string, clang::SourceRange>> macroExpansions;

// 获取宏定义和宏展开的位置
class MyPPCallbacks : public PPCallbacks {
public:
    explicit MyPPCallbacks(Preprocessor& PP) : SM(PP.getSourceManager()), PP(PP) {}

    void If(SourceLocation Loc, SourceRange ConditionRange, ConditionValueKind ConditionValue) override {
        SourceLocation macroLoc = ConditionRange.getBegin();
        if (SM.getFileID(Loc) == SM.getMainFileID() && macroLoc.isMacroID()) {
            llvm::outs() << "#if defined\n";
            llvm::outs() << "Loc " << SM.getSpellingLineNumber(Loc) << ":" << SM.getSpellingColumnNumber(Loc) << "    file:" << SM.getFilename(Loc) << "\n";
            llvm::outs() << "Range:" << SM.getExpansionLineNumber(ConditionRange.getBegin()) << ":" << SM.getExpansionColumnNumber(ConditionRange.getBegin()) << "\n"
                << "to :" << SM.getExpansionLineNumber(ConditionRange.getEnd()) << ":" << SM.getExpansionColumnNumber(ConditionRange.getEnd()) << "\n";
            llvm::outs() << "ConditionValue: " << ConditionValue << "\n";

            // TODO:当前仅仅能够使用单一的宏调用，多个宏调用无法分别显示展开结果，只能显示最终结果 1/0；同样的也无法适用于嵌套宏
            Token TheTok{};
            if (!PP.getRawToken(macroLoc, TheTok, true)) {
                llvm::outs() << "Macro token at this location: " << PP.getSpelling(TheTok) << "\n";
            }
            else {
                llvm::outs() << "Failed to get token at macro expansion location.\n\n";
            }


        }
    }

    void MacroDefined(const Token& MacroNameTok, const MacroDirective* MD) override {
        const MacroInfo* MI = MD->getMacroInfo();
        SourceLocation Loc = MI->getDefinitionLoc();
        if (Loc.isValid() && SM.getFileID(Loc) == SM.getMainFileID()) {
            // 获取宏的名称
            StringRef MacroName = MacroNameTok.getIdentifierInfo()->getName();

            std::string macroNameStr = MacroName.str();

            // 获取文件名
            StringRef FileName = SM.getFilename(Loc);

            // 获取行号和列号
            unsigned Line = SM.getSpellingLineNumber(Loc);
            unsigned Column = SM.getSpellingColumnNumber(Loc);

            // 输出宏名称
            llvm::outs() << "\nMacro definition happened\nMacro name: " << MacroName << "\n";

            //输出宏定义的参数列表                       
            if (MI->isFunctionLike()) {
                std::string params = "(";
                unsigned NumParams = MI->getNumParams();
                llvm::outs() << "Macro parameters:";
                for (unsigned i = 0; i < NumParams; ++i) {
                    const IdentifierInfo* Param = MI->params()[i];
                    StringRef ParamName = Param->getName();
                    //llvm::outs()  << ParamName << " ";
                    params += ParamName.str();
                    if (i != NumParams - 1) {
                        //llvm::outs() << ",";
                        params += ",";
                    }

                }
                //llvm::outs() << " )\n";
                params += ")";
                llvm::outs() << params << "\n";
            }
            // 输出宏体
            llvm::outs() << "Macro body: ";
            std::string macroBody;
            for (const auto& Token : MI->tokens()) {
                llvm::outs() << PP.getSpelling(Token) << " ";
                macroBody += PP.getSpelling(Token);
            }

            // 位置信息
            llvm::outs() << "\n defined at line:" << Line
                << " , column:" << Column
                << " , at file: " << FileName << "\n";

            llvm::outs() << "\n";
        }
    }

    void MacroExpands(const Token& MacroNameTok, const MacroDefinition& MD, SourceRange Range, const MacroArgs* Args) override {
        
        
        SourceLocation Loc = Range.getBegin();

        if (Loc.isValid() && SM.getFileID(SM.getSpellingLoc(Loc)) == SM.getMainFileID()) {
            // 获取宏的名称
            StringRef MacroName = MacroNameTok.getIdentifierInfo()->getName();
            llvm::outs() << "\n\nMacro Expansion Happened\nMacro name: " << MacroName << "\n";
            std::string macroNameStr = MacroName.str();

            macroExpansions.push_back(std::make_pair(macroNameStr, Range));

            //输出宏展开的参数列表                       
            const MacroInfo* MI = MD.getMacroInfo();
            if (MI->isFunctionLike()) {
                std::string params = "(";
                unsigned NumParams = MI->getNumParams();
                llvm::outs() << "parameters: ";
                for (unsigned i = 0; i < NumParams; ++i) {
                    params += printMacroArgument(Args, i, PP);
                    if (i != NumParams - 1) {
                        //llvm::outs() << ",";
                        params += ",";
                    }
                }
                //llvm::outs() << ")\n";
                params += ")";
                llvm::outs() << params << "\n";
            }

            // 获取文件名
            StringRef FileName = SM.getFilename(SM.getSpellingLoc(Loc));
            // 获取行号和列号
            unsigned Line = SM.getExpansionLineNumber(Loc);
            unsigned Column = SM.getExpansionColumnNumber(Loc);

            llvm::outs() << "expended at line:" << Line
                << " , column:" << Column
                << " , at file: " << FileName << "\n\n";
        }
    }

private:
    SourceManager& SM;
    Preprocessor& PP;

    static std::string printMacroArgument(const MacroArgs* Args, unsigned ArgIndex, Preprocessor& PP) {
        if (!Args) return "";
        Token const* ArgTokens = Args->getUnexpArgument(ArgIndex);
        std::string ArgString;
        llvm::raw_string_ostream ArgStream(ArgString);

        for (const Token* T = ArgTokens; T && T->isNot(tok::eof); ++T) {
            ArgStream << PP.getSpelling(*T);
        }

        return ArgStream.str();
    }
};

class FindMacroExpansionsVisitor : public RecursiveASTVisitor<FindMacroExpansionsVisitor> {
public:
    explicit FindMacroExpansionsVisitor(SourceManager& SM, LangOptions& LangOpts)
        :SM(SM), LangOpts(LangOpts) {}

    template<typename T>

    bool VisitNode(T* Node) {
        Rewriter TheRewriter;
        TheRewriter.setSourceMgr(SM, LangOpts);

        SourceLocation beginLoc = Node->getBeginLoc();

        if (beginLoc.isValid() && SM.getFileID(SM.getExpansionLoc(beginLoc)) == SM.getMainFileID() && beginLoc.isMacroID()) {
            SourceRange range = Node->getSourceRange();
            CharSourceRange charRange = CharSourceRange::getTokenRange(range);

            std::string txt = TheRewriter.getRewrittenText(charRange);
            Node->dump();
            llvm::outs() << "\nTheRewriter : " << txt << "\n\n";

        }

        return true;
    }

    bool VisitStmt(Stmt* s) {
        return VisitNode(s);
    }

    bool VisitDecl(Decl* D) {
        return VisitNode(D);
    }

    bool VisitExpr(Expr* E) {
        return VisitNode(E);
    }




private:
    SourceManager& SM;
    LangOptions& LangOpts;

};

class FindMacroExpansionsConsumer : public ASTConsumer {
public:
    explicit FindMacroExpansionsConsumer(SourceManager& SM, LangOptions& LangOpts) : Visitor(SM, LangOpts) {
    }

    void HandleTranslationUnit(ASTContext& Context) override {
        llvm::outs() << "开始AST查询\n";
        Visitor.TraverseAST(Context);
    }

private:
    FindMacroExpansionsVisitor Visitor;
};

class PreprocessFrontendAction : public ASTFrontendAction {
public:
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(CompilerInstance& CI, StringRef file) override {
        return std::make_unique<FindMacroExpansionsConsumer>(CI.getSourceManager(), CI.getLangOpts());
    }

    void ExecuteAction() override {
        auto& CI = getCompilerInstance();
        auto& PP = CI.getPreprocessor();
        SourceManager& SM = PP.getSourceManager();

        CI.getPreprocessor().addPPCallbacks(std::make_unique<MyPPCallbacks>(CI.getPreprocessor()));
        PP.EnterMainSourceFile();

        std::vector<Token> tokens;

        Token Tok{};
        PP.Lex(Tok);

        while (true) {
            
            
            if (Tok.is(tok::eof))
                break;

            SourceLocation Loc = Tok.getLocation();
            SourceLocation SpellingLoc = SM.getSpellingLoc(Loc);
            SourceLocation ExpansionLoc = SM.getExpansionLoc(Loc);

            if (SM.getFileID(SpellingLoc) == SM.getMainFileID() && Loc.isMacroID()) {
                tokens.push_back(Tok);
                // 输出位置信息
                /*llvm::outs() << "Location: " << SM.getPresumedLoc(Loc).getLine() << ":" << SM.getPresumedLoc(Loc).getColumn() << "\n";
                llvm::outs() << "Spelling Location: " << SM.getPresumedLoc(SpellingLoc).getLine() << ":" << SM.getPresumedLoc(SpellingLoc).getColumn() << "\n";
                llvm::outs() << "Expansion Location: " << SM.getPresumedLoc(ExpansionLoc).getLine() << ":" << SM.getPresumedLoc(ExpansionLoc).getColumn() << "\n";*/

                // 获取宏展开的原始标识符 Token

                

                /*SourceLocation macroSL = SM.getImmediateMacroCallerLoc(ExpansionLoc);
                llvm::outs() << "Location: " << SM.getPresumedLoc(macroSL).getLine() << ":" << SM.getPresumedLoc(macroSL).getColumn() << "\n";
                
                std::string macroName;
                
                llvm::outs() << "macroName: " << macroName << "\n";*/


                /*nlohmann::json* macroJson = nullptr;  // 使用指针来存储可能的找到的对象的地址

                auto it = jsonMap.find(macroName);
                if (it != jsonMap.end()) {
                    macroJson = &(it->second);  // 存储找到的对象的地址
                }
                else {
                    jsonMap[macroName] = nlohmann::json{};
                    macroJson = &(jsonMap[macroName]);
                }

                if ((*macroJson).contains("expansion body")) {
                    std::string str = (*macroJson)["expansion body"];
                    (*macroJson)["expansion body"] = str + PP.getSpelling(Tok);
                }
                else {
                    (*macroJson)["expansion body"] = PP.getSpelling(Tok);
                }
                */
                /*
                auto [it, inserted] = jsonMap.insert_or_assign(macroName, nlohmann::json{});

                // 使用引用直接操作 json 对象
                nlohmann::json& macroJson = it->second;
                // 检查 "expansion body" 键是否存在
                if (macroJson.contains("expansion body")) {
                    macroJson["expansion body"] += PP.getSpelling(Tok);  // 如果存在，则追加内容
                }
                else {
                    macroJson["expansion body"] = PP.getSpelling(Tok);   // 如果不存在，初始化它
                }
                */

               // llvm::outs() << PP.getSpelling(Tok) << "\n";
            }

            
            PP.Lex(Tok);
        }
        llvm::outs() << "\n";


        for (std::pair<std::string, clang::SourceRange> expansion : macroExpansions) {
            std::string macroName = expansion.first;
            SourceRange range = expansion.second;

            llvm::outs() << "macroName : " << macroName << "\n";
            llvm::outs() << "Spelling range : " << SM.getSpellingLineNumber(range.getBegin())
                << " : " << SM.getSpellingColumnNumber(range.getBegin()) << "\n"
                << "to : " << SM.getSpellingLineNumber(range.getEnd())
                << " : " << SM.getSpellingColumnNumber(range.getEnd()) << "\n\n";
            for (Token token : tokens) {
                SourceLocation Loc = token.getLocation();
                SourceLocation SpellingLoc = SM.getSpellingLoc(Loc);
                SourceLocation ExpansionLoc = SM.getExpansionLoc(Loc);

                /*llvm::outs() << "Loc : " << SM.getSpellingLineNumber(Loc)
                    << " : " << SM.getSpellingColumnNumber(Loc) << "\n";
                llvm::outs() << "SpellingLoc : " << SM.getSpellingLineNumber(SpellingLoc)
                    << " : " << SM.getSpellingColumnNumber(SpellingLoc) << "\n";
                llvm::outs() << "ExpansionLoc : " << SM.getSpellingLineNumber(ExpansionLoc)
                    << " : " << SM.getSpellingColumnNumber(ExpansionLoc) << "\n";
                
                llvm::outs() << token.getLiteralData() << "\n\n";*/


                bool isInRange = !SM.isBeforeInTranslationUnit(ExpansionLoc, range.getBegin()) &&
                    !SM.isBeforeInTranslationUnit(range.getEnd(), ExpansionLoc);

                if (isInRange) {
                    llvm::outs() << PP.getSpelling(Tok);
                }
            }
            llvm::outs() << "\n";
        }
        

        /*for (const auto& [key, value] : jsonMap) {
            std::cout << "Key: " << key << "\n";
            std::cout << "Value: " << value.dump(4) << "\n\n";
        }*/
    }

    void EndSourceFileAction() override {
        /* // 将 unordered_map 转换为 nlohmann::json 对象
         nlohmann::json jsonObj;
         for (const auto& [key, value] : jsonMap) {
             jsonObj[key] = value;
         }

         // 将 json 对象写入到文件
         std::ofstream outFile("output.json");
         outFile << jsonObj.dump(4);  // 参数 4 是为了美观的缩进
         outFile.close();*/
    }

private:
    std::vector<std::string> traceMacroExpansion(SourceLocation Loc, SourceManager& SM, Preprocessor& PP) {
        std::vector<std::string> names;
        while (Loc.isMacroID()) {
            SourceLocation MacroLoc = SM.getImmediateMacroCallerLoc(Loc);
            std::string MacroName;
            Token TheTok{};
            if (!PP.getRawToken(MacroLoc, TheTok, true)) {
                llvm::outs() << "Macro token at this location: " << PP.getSpelling(TheTok) << "\n";
                MacroName = PP.getSpelling(TheTok);
            }
            else {
                llvm::outs() << "Failed to get token at macro expansion location.\n\n";
            }
            names.push_back(MacroName);
            Loc = SM.getImmediateSpellingLoc(MacroLoc);
        }
        std::reverse(names.begin(), names.end());
        return names;
    }

};

static cl::OptionCategory MyToolCategory("My Tool Options");

int main(int argc, const char** argv) {
    llvm::outs() << "Output the locations of macro definitions and macro expansions\n\n";
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << "Failed to parse options: " << ExpectedParser.takeError() << "\n";
        return 1;
    }
    CommonOptionsParser& OptionsParser = ExpectedParser.get();

    ClangTool Tool(OptionsParser.getCompilations(), OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<PreprocessFrontendAction>().get());
}