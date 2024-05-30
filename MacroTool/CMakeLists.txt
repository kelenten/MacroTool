# CMakeList.txt: MacroTool 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#

# 将源代码添加到此项目的可执行文件。
# add_executable (MacroTool "MacroTool.cpp" "MacroTool.h")


set(LLVM_DIR D:/LLVM/llvm-project/build/lib/cmake/llvm)

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "include_directories ${LLVM_INCLUDE_DIRS}")
message(STATUS "link_directories: ${LLVM_LIBRARY_DIRS}")

# 获取 LLVM 的包含目录和库目录
include_directories("D:/LLVM/llvm-project/clang/include" 
    "D:/LLVM/llvm-project/build/tools/clang/include"
    "D:/LLVM/llvm-project/llvm/include"
    "D:/LLVM/llvm-project/build/include"
)

link_directories(
    "D:/LLVM/llvm-project/build/Release/lib"
    "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.22621.0/um/x64"
)

# CMake库
list(APPEND CMAKE_MODULE_PATH ${LLVM_CMAKE_DIR})
include(AddLLVM)
set(LLVM_LINK_COMPONENTS 
 	Support
 	Analysis)

add_definitions(${LLVM_DEFINITIONS})
# Check if we are using clang-cl or another Clang variant
if(MSVC AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
  # Using clang-cl, use MSVC style flags
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
else()
  # Not using clang-cl, assume it's either GCC or regular Clang
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()

# 自动生成compile-command.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 生成可执行文件
add_llvm_executable(MacroTool "MacroTool.cpp" "MacroTool.h")

# 附加依赖项
target_link_libraries(MacroTool
  PRIVATE
  clang-repl clangAnalysis clangAnalysisFlowSensitive clangAnalysisFlowSensitiveModels clangAPINotes clangARCMigrate clangAST clangASTMatchers clangBasic clangCodeGen clangCrossTU clangDependencyScanning clangDirectoryWatcher clangDriver clangDynamicASTMatchers clangEdit clangExtractAPI clangFormat clangFrontend clangFrontendTool clangHandleCXX clangHandleLLVM clangIndex clangIndexSerialization clangInstallAPI clangInterpreter clangLex clangParse clangRewrite clangRewriteFrontend clangSema clangSerialization clangStaticAnalyzerCheckers clangStaticAnalyzerCore clangStaticAnalyzerFrontend clangSupport clangTooling clangToolingASTDiff clangToolingCore clangToolingInclusions clangToolingInclusionsStdlib clangToolingRefactoring clangToolingSyntax clangTransformer DynamicLibraryLib libclang LLVM-C LLVMAArch64AsmParser LLVMAArch64CodeGen LLVMAArch64Desc LLVMAArch64Disassembler LLVMAArch64Info LLVMAArch64Utils LLVMAggressiveInstCombine LLVMAMDGPUAsmParser LLVMAMDGPUCodeGen LLVMAMDGPUDesc LLVMAMDGPUDisassembler LLVMAMDGPUInfo LLVMAMDGPUTargetMCA LLVMAMDGPUUtils LLVMAnalysis LLVMARMAsmParser LLVMARMCodeGen LLVMARMDesc LLVMARMDisassembler LLVMARMInfo LLVMARMUtils LLVMAsmParser LLVMAsmPrinter LLVMAVRAsmParser LLVMAVRCodeGen LLVMAVRDesc LLVMAVRDisassembler LLVMAVRInfo LLVMBinaryFormat LLVMBitReader LLVMBitstreamReader LLVMBitWriter LLVMBPFAsmParser LLVMBPFCodeGen LLVMBPFDesc LLVMBPFDisassembler LLVMBPFInfo LLVMCFGuard LLVMCFIVerify LLVMCodeGen LLVMCodeGenTypes LLVMCore LLVMCoroutines LLVMCoverage LLVMDebugInfoBTF LLVMDebugInfoCodeView LLVMDebuginfod LLVMDebugInfoDWARF LLVMDebugInfoGSYM LLVMDebugInfoLogicalView LLVMDebugInfoMSF LLVMDebugInfoPDB LLVMDemangle LLVMDiff LLVMDlltoolDriver LLVMDWARFLinker LLVMDWARFLinkerClassic LLVMDWARFLinkerParallel LLVMDWP LLVMExecutionEngine LLVMExegesis LLVMExegesisAArch64 LLVMExegesisMips LLVMExegesisPowerPC LLVMExegesisX86 LLVMExtensions LLVMFileCheck LLVMFrontendDriver LLVMFrontendHLSL LLVMFrontendOffloading LLVMFrontendOpenACC LLVMFrontendOpenMP LLVMFuzzerCLI LLVMFuzzMutate LLVMGlobalISel LLVMHexagonAsmParser LLVMHexagonCodeGen LLVMHexagonDesc LLVMHexagonDisassembler LLVMHexagonInfo LLVMHipStdPar LLVMInstCombine LLVMInstrumentation LLVMInterfaceStub LLVMInterpreter LLVMipo LLVMIRPrinter LLVMIRReader LLVMJITLink LLVMLanaiAsmParser LLVMLanaiCodeGen LLVMLanaiDesc LLVMLanaiDisassembler LLVMLanaiInfo LLVMLibDriver LLVMLineEditor LLVMLinker LLVMLoongArchAsmParser LLVMLoongArchCodeGen LLVMLoongArchDesc LLVMLoongArchDisassembler LLVMLoongArchInfo LLVMLTO LLVMMC LLVMMCA LLVMMCDisassembler LLVMMCJIT LLVMMCParser LLVMMipsAsmParser LLVMMipsCodeGen LLVMMipsDesc LLVMMipsDisassembler LLVMMipsInfo LLVMMIRParser LLVMMSP430AsmParser LLVMMSP430CodeGen LLVMMSP430Desc LLVMMSP430Disassembler LLVMMSP430Info LLVMNVPTXCodeGen LLVMNVPTXDesc LLVMNVPTXInfo LLVMObjCARCOpts LLVMObjCopy LLVMObject LLVMObjectYAML LLVMOptDriver LLVMOption LLVMOrcDebugging LLVMOrcJIT LLVMOrcShared LLVMOrcTargetProcess LLVMPasses LLVMPowerPCAsmParser LLVMPowerPCCodeGen LLVMPowerPCDesc LLVMPowerPCDisassembler LLVMPowerPCInfo LLVMProfileData LLVMRemarks LLVMRISCVAsmParser LLVMRISCVCodeGen LLVMRISCVDesc LLVMRISCVDisassembler LLVMRISCVInfo LLVMRISCVTargetMCA LLVMRuntimeDyld LLVMScalarOpts LLVMSelectionDAG LLVMSparcAsmParser LLVMSparcCodeGen LLVMSparcDesc LLVMSparcDisassembler LLVMSparcInfo LLVMSupport LLVMSymbolize LLVMSystemZAsmParser LLVMSystemZCodeGen LLVMSystemZDesc LLVMSystemZDisassembler LLVMSystemZInfo LLVMTableGen LLVMTableGenBasic LLVMTableGenCommon LLVMTarget LLVMTargetParser LLVMTextAPI LLVMTextAPIBinaryReader LLVMTransformUtils LLVMVEAsmParser LLVMVECodeGen LLVMVectorize LLVMVEDesc LLVMVEDisassembler LLVMVEInfo LLVMWebAssemblyAsmParser LLVMWebAssemblyCodeGen LLVMWebAssemblyDesc LLVMWebAssemblyDisassembler LLVMWebAssemblyInfo LLVMWebAssemblyUtils LLVMWindowsDriver LLVMWindowsManifest LLVMX86AsmParser LLVMX86CodeGen LLVMX86Desc LLVMX86Disassembler LLVMX86Info LLVMX86TargetMCA LLVMXCoreCodeGen LLVMXCoreDesc LLVMXCoreDisassembler LLVMXCoreInfo LLVMXRay LTO Remarks "Version.Lib"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET MacroTool PROPERTY CXX_STANDARD 20)
endif()


# TODO: 如有需要，请添加测试并安装目标。
