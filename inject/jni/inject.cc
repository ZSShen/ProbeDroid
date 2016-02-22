#include "globals.h"
#include "proc.h"


static const char* kOptLongZygote       = "zygote";
static const char* kOptLongAppName      = "app";
static const char* kOptLongLibPath      = "lib";
static const char* kOptLongModulePath   = "module";

static const char kOptZygote        = 'z';
static const char kOptAppName       = 'a';
static const char kOptLibPath       = 'l';
static const char kOptModulePath    = 'm';


void PrintUsage()
{
    const char *usage = \
    "Usage: ./inject --zygote PID --app APPNAME --lib LIBPATH --module MODULEPATH\n"
    "                -z       PID -a    APPNAME -l    LIBPATH -m       MODULEPATH\n\n"
    "Example: ./inject --zygote 934 --app org.zsshen.bmi --lib /data/local/tmp/libhook.so"
    " --module /data/local/tmp/analysis.apk\n"
    "         ./inject -z       934 -a    org.zsshen.bmi -l    /data/local/tmp/libhook.so"
    " -m       /data/local/tmp/analysis.apk\n\n";
    std::cerr << usage;
}

int main(int32_t argc, char** argv)
{
    // Acquire the command line arguments.
    struct option opts[] = {
        {kOptLongZygote, required_argument, 0, kOptZygote},
        {kOptLongAppName, required_argument, 0, kOptAppName},
        {kOptLongLibPath, required_argument, 0, kOptLibPath},
        {kOptLongModulePath, required_argument, 0, kOptModulePath},
    };

    char sz_order[kBlahSizeTiny];
    memset(sz_order, 0, sizeof(char) * kBlahSizeTiny);
    sprintf(sz_order, "%c:%c:%c:%c:", kOptZygote, kOptAppName, kOptLibPath,
            kOptModulePath);

    int opt, idx_opt;
    pid_t pid_zygote = 0;
    char *app_name = nullptr, *lib_path = nullptr, *module_path = nullptr;
    while ((opt = getopt_long(argc, argv, sz_order, opts, &idx_opt)) != -1) {
        switch (opt) {
            case kOptZygote:
                pid_zygote = atoi(optarg);
                break;
            case kOptAppName:
                app_name = optarg;
                break;
            case kOptLibPath:
                lib_path = optarg;
                break;
            case kOptModulePath:
                module_path = optarg;
                break;
            default:
                PrintUsage();
                return EXIT_FAILURE;
        }
    }

    if (pid_zygote == 0 || !app_name || !lib_path || !module_path) {
        PrintUsage();
        return EXIT_FAILURE;
    }

    // Start to inject the designated shared object.
    proc::EggHunter hunter;
    hunter.Hunt(pid_zygote, app_name, lib_path, module_path);

    return EXIT_SUCCESS;
}