#include <dxc/Support/WinIncludes.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MSFileSystem.h>
#include <fstream>
#include <dxil2spirv.h>

using namespace llvm;

int main(int argc, char** argv) {
	if (std::error_code ec = llvm::sys::fs::SetupPerThreadFileSystem())
		return 1;
	llvm::sys::fs::AutoCleanupPerThreadFileSystem auto_cleanup_fs;
	llvm::sys::fs::MSFileSystem* msfPtr;
	HRESULT hr;
	if (!SUCCEEDED(hr = CreateMSFileSystemForDisk(&msfPtr)))
		return 1;
	std::unique_ptr<llvm::sys::fs::MSFileSystem> msf(msfPtr);
	llvm::sys::fs::AutoPerThreadSystem pts(msf.get());

	cl::opt<bool>
		ShowHelp("help", cl::desc("Show this help"), cl::init(false));
	cl::opt<std::string>
		InputFilename("i", cl::desc("Input DXIL File"));
	cl::opt<std::string>
		OutputFilename("o", cl::desc("Output SPIRV file"));
	cl::opt<bool>
		Od("Od", cl::desc("Disable spirv-opt"), cl::init(false));
	cl::opt<bool>
		DisableWrapperFunc("no-wrapper", cl::desc("Disable wrapper function"), cl::init(false));
	cl::opt<bool>
		StripDebug("strip-debug", cl::desc("Strip debug info"), cl::init(false));
	cl::ParseCommandLineOptions(argc, argv, "dxil to spirv\n");
	if (ShowHelp) {
		cl::PrintHelpMessage();
		return 0;
	}
	if (InputFilename.getNumOccurrences() != 1 || OutputFilename.getNumOccurrences() != 1) {
		cl::PrintHelpMessage();
		return -1;
	}

	dxil2spirv::options options;
	options.enableOptimizer = !Od;
	options.stripName = StripDebug;
	options.enableWrapperFunction = !DisableWrapperFunc;

	std::ifstream fin(InputFilename, std::ios::binary);
	std::vector<uint8_t> dxil((std::istreambuf_iterator<char>(fin)),
		std::istreambuf_iterator<char>());
	std::vector<uint32_t> spirv;

	dxil2spirv::convert(spirv, dxil.data(), dxil.size(), &options);

	std::ofstream(OutputFilename, std::ios::binary)
		.write((char *)spirv.data(),
			spirv.size() * sizeof(uint32_t));
	return 0;
}