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
#include "D:/LLVM/jsoncpp/json-develop/single_include/nlohmann/json.hpp"
#include "clang/Analysis/MacroExpansionContext.h"

#include "clang/Format/Format.h"
#include "llvm/Support/Allocator.h"
#include "clang/Basic/IdentifierTable.h"
#include "clang/Lex/PreprocessorOptions.h"


using namespace clang;
using namespace clang::tooling;
using namespace llvm;

struct macroDef {
    std::string MacroName;
    std::string Macrobody;
    std::string File;
    unsigned Line;
    unsigned Column;
    bool IsFunctionLike;
    bool IsDefined;
    std::string Params;

    // 构造函数
    macroDef(const std::string& name, std::string body, const std::string& file, unsigned line, unsigned column, bool functionLike, bool defined, std::string params)
        : MacroName(name), Macrobody(body), File(file), Line(line), Column(column), IsFunctionLike(functionLike), IsDefined(defined), Params(params) {}
};

struct macroExp {
    std::string MacroName;
    std::string ExpandedContent;
    std::string File;
    unsigned Line;
    unsigned Column;
    bool IsFunctionLike;
    std::string Params;

    // 构造函数
    macroExp(const std::string& macroName, std::string expandedContent, const std::string& file,
        unsigned line, unsigned column, bool isFunctionLike, std::string params)
        : MacroName(macroName), ExpandedContent(expandedContent), File(file),
        Line(line), Column(column), IsFunctionLike(isFunctionLike), Params(params) {}
};

std::vector<std::shared_ptr<macroDef>> definitions;
std::vector<std::shared_ptr<macroExp>> expansions;

// 函数将 macroDef 转换为 JSON 对象
nlohmann::json macroDefToJson(const std::shared_ptr<macroDef>& def) {
    nlohmann::json root;
    root["MacroName"] = def->MacroName;
    root["Macrobody"] = def->Macrobody;
    root["File"] = def->File;
    root["Line"] = def->Line;
    root["Column"] = def->Column;
    root["IsFunctionLike"] = def->IsFunctionLike;
    root["IsDefined"] = def->IsDefined;
    root["Params"] = def->Params;
    return root;
}

// 函数将 macroExp 转换为 JSON 对象
nlohmann::json macroExpToJson(const std::shared_ptr<macroExp>& exp) {
    nlohmann::json root;
    root["MacroName"] = exp->MacroName;
    root["ExpandedContent"] = exp->ExpandedContent;
    root["File"] = exp->File;
    root["Line"] = exp->Line;
    root["Column"] = exp->Column;
    root["IsFunctionLike"] = exp->IsFunctionLike;
    root["Params"] = exp->Params;
    return root;
}

std::vector <std::shared_ptr<macroExp>> expansionText;
std::vector<std::shared_ptr<macroDef>> definitonTest;
bool compile;
std::vector<std::pair<std::shared_ptr<macroDef>, clang::FileID>> defparams;
std::vector<std::pair<std::shared_ptr<macroDef>, clang::FileID>> defbodys;
std::vector<std::pair<std::shared_ptr<macroExp>, clang::FileID>> expparams;
std::vector<std::pair<std::shared_ptr<macroExp>, clang::FileID>> expbodys;


// 获取宏定义和宏展开的位置
class MyPPCallbacks : public PPCallbacks {
public:
    explicit MyPPCallbacks(Preprocessor& PP) : SM(PP.getSourceManager()), PP(PP) {}

    void If(SourceLocation Loc, SourceRange ConditionRange, ConditionValueKind ConditionValue) override {
        if (compile) {
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
    }
    
    void MacroUndefined(const Token& MacroNameTok, const MacroDefinition& MD, const MacroDirective* Undef) override {
        if (compile) {
            SourceLocation Loc = MacroNameTok.getLocation();
            if (Loc.isValid() && SM.getFileID(Loc) == SM.getMainFileID()) {
                bool isDefined = false;

                // 获取宏的名称
                StringRef MacroName = MacroNameTok.getIdentifierInfo()->getName();
                std::string macroNameStr = MacroName.str();

                // 获取文件名
                StringRef FileName = SM.getFilename(Loc);

                // 获取行号和列号
                unsigned Line = SM.getSpellingLineNumber(Loc);
                unsigned Column = SM.getSpellingColumnNumber(Loc);

                // 输出宏名称
                llvm::outs() << "\nMacro Undefinition happened\nMacro name: " << MacroName << "\n";
                // 位置信息
                llvm::outs() << "defined at line:" << Line
                    << " , column:" << Column
                    << " , at file: " << FileName << "\n";

                llvm::outs() << "\n";

                std::string str = "";
                auto def = std::make_shared<macroDef>(
                    macroNameStr,
                    str,
                    FileName.str(),
                    Line,
                    Column,
                    false,
                    isDefined,
                    str
                );

                definitions.emplace_back(def);
            }
        }
        
    }
    
    void MacroDefined(const Token& MacroNameTok, const MacroDirective* MD) override {
        if (compile) {
            const MacroInfo* MI = MD->getMacroInfo();
            SourceLocation Loc = MI->getDefinitionLoc();
            if (Loc.isValid() && SM.getFileID(Loc) == SM.getMainFileID()) {
                // 获取宏的名称
                StringRef MacroName = MacroNameTok.getIdentifierInfo()->getName();
                std::string macroNameStr = MacroName.str();
                bool isDefined = true;

                // 获取文件名
                StringRef FileName = SM.getFilename(Loc);

                // 获取行号和列号
                unsigned Line = SM.getSpellingLineNumber(Loc);
                unsigned Column = SM.getSpellingColumnNumber(Loc);

                // 输出宏名称
                llvm::outs() << "\nMacro definition happened\nMacro name: " << MacroName << "\n";

                std::string params = "";
                //输出宏定义的参数列表                       
                if (MI->isFunctionLike()) {
                    params += "(";
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

                ArrayRef<Token> bodyTokens = MI->tokens();

                // 输出宏体
                llvm::outs() << "Macro body: ";
                std::string macroBody;
                for (const auto& Token : bodyTokens) {
                    llvm::outs() << PP.getSpelling(Token) << " ";
                    macroBody += PP.getSpelling(Token);
                }

                // 位置信息
                llvm::outs() << "\n defined at line:" << Line
                    << " , column:" << Column
                    << " , at file: " << FileName << "\n";
               
                std::unique_ptr<llvm::MemoryBuffer> Buffer1 = llvm::MemoryBuffer::getMemBufferCopy(params);
                clang::FileID FID1 = SM.createFileID(std::move(Buffer1), clang::SrcMgr::C_User);

                PP.EnterSourceFile(FID1, nullptr, clang::SourceLocation());

                std::unique_ptr<llvm::MemoryBuffer> Buffer2 = llvm::MemoryBuffer::getMemBufferCopy(macroBody);
                clang::FileID FID2 = SM.createFileID(std::move(Buffer2), clang::SrcMgr::C_User);

                PP.EnterSourceFile(FID2, nullptr, clang::SourceLocation());

                std::string str = "";
                auto def = std::make_shared<macroDef>(
                    macroNameStr,
                    str,
                    FileName.str(),
                    Line,
                    Column,
                    (MI->isFunctionLike()),
                    isDefined,
                    str
                );
                definitions.emplace_back(def);
                defparams.emplace_back(def, FID1);
                defbodys.emplace_back(def, FID2);
            }
        }
    }

    void MacroExpands(const Token& MacroNameTok, const MacroDefinition& MD, SourceRange Range, const MacroArgs* Args) override {
        if (compile) {
            SourceLocation Loc = Range.getBegin();
            if (Loc.isValid() && SM.getFileID(SM.getExpansionLoc(SM.getTopMacroCallerLoc(Loc))) == SM.getMainFileID()) {
                // 获取宏的名称
                StringRef MacroName = MacroNameTok.getIdentifierInfo()->getName();
                llvm::outs() << "\n\nMacro Expansion Happened\nMacro name: " << MacroName << "\n";
                std::string macroNameStr = MacroName.str();

                std::string original = macroNameStr;

                std::string params = "";
                //输出宏展开的参数列表                       
                const MacroInfo* MI = MD.getMacroInfo();
                if (MI->isFunctionLike()) {
                    params += "(";
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
                    original += params;
                }

                // 获取文件名
                StringRef FileName = SM.getFilename(SM.getSpellingLoc(Loc));
                // 获取行号和列号
                unsigned Line = SM.getExpansionLineNumber(Loc);
                unsigned Column = SM.getExpansionColumnNumber(Loc);

                llvm::outs() << "expended at line:" << Line
                    << " , column:" << Column
                    << " , at file: " << FileName << "\n";


                std::unique_ptr<llvm::MemoryBuffer> Buffer1 = llvm::MemoryBuffer::getMemBufferCopy("\n" + params + "\n");
                clang::FileID FID1 = SM.createFileID(std::move(Buffer1), clang::SrcMgr::C_User);

                PP.EnterSourceFile(FID1, nullptr, clang::SourceLocation());

                std::unique_ptr<llvm::MemoryBuffer> Buffer2 = llvm::MemoryBuffer::getMemBufferCopy("\n" + original   +   "\n");
                clang::FileID FID2 = SM.createFileID(std::move(Buffer2), clang::SrcMgr::C_User);

                PP.EnterSourceFile(FID2, nullptr, clang::SourceLocation());
               

                std::string str = "";
                auto exp = std::make_shared<macroExp>(
                    macroNameStr,
                    str,
                    FileName.str(),
                    Line,
                    Column,
                    (MI->isFunctionLike()),
                    str
                );
                expansions.emplace_back(exp);
                expparams.emplace_back(exp, FID1);
                expbodys.emplace_back(exp, FID2);
            }
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

    void getmacroInfo(const Token& MacroNameTok, const MacroDirective* MD, bool isDefined) {
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

            ArrayRef<Token> bodyTokens = MI->tokens();

            // 输出宏体
            llvm::outs() << "Macro body: ";
            std::string macroBody;
            for (const auto& Token : bodyTokens) {
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
        auto& SM = PP.getSourceManager();
        auto& LO = CI.getLangOpts();

        /*MacroExpansionContext recorder(LO);
        recorder.registerForPreprocessor(PP);*/


        std::unique_ptr<PPCallbacks> callbacks = std::make_unique<MyPPCallbacks>(CI.getPreprocessor());
        PP.addPPCallbacks(std::move(callbacks));
        PP.EnterMainSourceFile();

        // 创建一个文件输出流
        std::ofstream outFile("test1_pre.txt");
        if (!outFile) {
            std::cerr << "Error opening output file." << std::endl;
            return;
        }

        compile = true;
        clang::Token Tok{};
        PP.Lex(Tok);

        while (true) {
            SourceLocation Loc = Tok.getLocation();
            outFile << PP.getSpelling(Tok) << ' ';
            if (SM.isInMainFile(SM.getSpellingLoc(Loc))) {
                
            }
            for (const auto& element : defparams) {
                if (element.second == SM.getFileID(SM.getExpansionLoc(Loc))) {
                    
                    element.first->Params += PP.getSpelling(Tok);
                    //llvm::outs() << PP.getSpelling(Tok);
                }
            }
            for (const auto& element : defbodys) {
                if (element.second == SM.getFileID(SM.getExpansionLoc(Loc))) {
                    
                    element.first->Macrobody += PP.getSpelling(Tok);
                    //llvm::outs() << PP.getSpelling(Tok);
                }
            }
            for (const auto& element : expparams) {
                if (element.second == SM.getFileID(SM.getExpansionLoc(SM.getTopMacroCallerLoc(Loc)))) {
                    
                    element.first->Params += PP.getSpelling(Tok);
                    //llvm::outs() << PP.getSpelling(Tok);
                }
            }
            for (const auto& element : expbodys) {
                if (element.second == SM.getFileID(SM.getExpansionLoc(SM.getTopMacroCallerLoc(Loc)))) {
                    
                    element.first->ExpandedContent += PP.getSpelling(Tok);
                    //llvm::outs() << PP.getSpelling(Tok);
                }
            }

            
            if (Tok.is(tok::eof))
                break;           
            PP.Lex(Tok);    
        }
        outFile.close();  // 关闭文件流
        //for (std::pair<std::string, clang::SourceRange> expansion : macroExpansions) {
        //    std::string macroName = expansion.first;
        //    SourceRange range = expansion.second;

        //    SourceLocation begin = range.getBegin();
        //    SourceLocation end = range.getEnd();

        //    llvm::outs() << "macroName : " << macroName << "\n";
        //    
        //    std::optional< StringRef > expansionText = recorder.getExpandedText(begin);
        //    std::optional< StringRef > originalText = recorder.getOriginalText(begin);

        //    if (expansionText) {
        //        llvm::outs() << "Expanded macro: " << *expansionText << "\n";
        //    }
        //    if (originalText){
        //        llvm::outs() << "originalText macro: " << *originalText << "\n\n";
        //    }   
        //}


        //for (const auto& exp : expansionText) {
        //    std::string MacroName = exp->MacroName;
        //    llvm::outs() << "!!macroName: " << MacroName << "\n";
        //    std::string Code = exp->ExpandedContent;
        //    std::unique_ptr<llvm::MemoryBuffer> Buffer = llvm::MemoryBuffer::getMemBufferCopy(Code);
        //    clang::FileID FID = SM.createFileID(std::move(Buffer), clang::SrcMgr::C_User);
        //    // 设置 Preprocessor 读取这个 FileID
        //    PP.EnterSourceFile(FID, nullptr, clang::SourceLocation());
        //    std::string expandedContent = "";
        //    do {
        //        PP.Lex(Tok);
        //        if (PP.getSourceManager().isInMainFile(Tok.getLocation())) {
        //            expandedContent += PP.getSpelling(Tok);
        //            llvm::outs() << PP.getSpelling(Tok);
        //        }
        //    } while (Tok.isNot(clang::tok::eof));
        //    llvm::outs() << "\n\n";
        //    exp->ExpandedContent = expandedContent;
        //} 

        printDefinitions(definitions);
        printExpansions(expansions);
    }

private:
    void printDefinitions(const std::vector<std::shared_ptr<macroDef>>& definitions) {
        std::cout << "Macro Definitions:" << std::endl;
        for (const auto& defPtr : definitions) {
            std::cout << "MacroName: " << defPtr->MacroName << std::endl;
            std::cout << "Macrobody: " << defPtr->Macrobody << std::endl;
            std::cout << "File: " << defPtr->File << std::endl;
            std::cout << "Line: " << defPtr->Line << std::endl;
            std::cout << "Column: " << defPtr->Column << std::endl;
            std::cout << "IsFunctionLike: " << defPtr->IsFunctionLike << std::endl;
            std::cout << "IsDefined: " << defPtr->IsDefined << std::endl;
            std::cout << "Params: " << defPtr->Params << std::endl;
            std::cout << "------------------------" << std::endl;
        }
    }

    void printExpansions(const std::vector<std::shared_ptr<macroExp>>& expansions) {
        std::cout << "Macro Expansions:" << std::endl;
        for (const auto& expPtr : expansions) {
            if (expPtr) { // 确保指针有效
                std::cout << "MacroName: " << expPtr->MacroName << std::endl;
                std::cout << "ExpandedContent: " << expPtr->ExpandedContent << std::endl;
                std::cout << "File: " << expPtr->File << std::endl;
                std::cout << "Line: " << expPtr->Line << std::endl;
                std::cout << "Column: " << expPtr->Column << std::endl;
                std::cout << "IsFunctionLike: " << expPtr->IsFunctionLike << std::endl;
                std::cout << "Params: " << expPtr->Params << std::endl;
                std::cout << "------------------------" << std::endl;
            }
        }
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
    Tool.run(newFrontendActionFactory<PreprocessFrontendAction>().get());

    nlohmann::json defs;
    nlohmann::json exps;
    for (const auto& def : definitions) {
        defs.push_back(macroDefToJson(def));
    }
    for (const auto& exp : expansions) {
        exps.push_back(macroExpToJson(exp));
    }
    std::ofstream file1("definitions.json");
    if (file1.is_open()) {
        file1 << defs.dump(4);  
        file1.close();
    }
    std::ofstream file2("expansions.json");
    if (file2.is_open()) {
        file2 << exps.dump(4);
        file2.close();
    }
    return 0;
}