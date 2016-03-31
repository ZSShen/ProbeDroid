#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>

#include "globals.h"
#include "proc.h"


static const char* kOptLongZygote = "zygote";
static const char* kOptLongAppName = "app";
static const char* kOptLongLibPath = "lib";
static const char* kOptLongModulePath = "module";
static const char* kOptLongClassName = "class";

static const char kOptZygote = 'z';
static const char kOptAppName = 'a';
static const char kOptLibPath = 'l';
static const char kOptModulePath = 'm';
static const char kOptClassName = 'c';


void PrintUsage()
{
    const char *usage = \
    "Usage: ./launcher\n"
    "               --zygote [-z] PID (The zygote process id)\n"
    "               --app    [-a] APPNAME (The package name of target app)\n"
    "               --lib    [-l] LIBPATH (The path name of ProbeDroid core library)\n"
    "               --module [-m] MODULEPATH (The path name of analysis package)\n"
    "               --class  [-c] CLASSNAME (The full qualified main class name of analysis package)\n"
    "\n"
    "Example:\n"
    "./launcher --zygote 934"
            " --app com.google.android.app.maps"
            " --lib /data/local/tmp/libprobedroid.so"
            " --module /data/local/tmp/Instument.apk"
            " --class org.zsshen.instument.Main\n"
    "\n"
    "./launcher -z 934"
            " -a com.google.android.app.maps"
            " -l /data/local/tmp/libprobedroid.so"
            " -m /data/local/tmp/Instument.apk"
            " -c org.zsshen.instument.Main\n";

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
        {kOptLongClassName, required_argument, 0, kOptClassName},
    };

    char sz_order[kBlahSizeTiny];
    memset(sz_order, 0, sizeof(char) * kBlahSizeTiny);
    sprintf(sz_order, "%c:%c:%c:%c:%c:", kOptZygote, kOptAppName, kOptLibPath,
            kOptModulePath, kOptClassName);

    int opt, idx_opt;
    pid_t pid_zygote = 0;
    char *app_name = nullptr, *lib_path = nullptr, *module_path = nullptr,
         *class_name = nullptr;
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
            case kOptClassName:
                class_name = optarg;
                break;
            default:
                PrintUsage();
                return EXIT_FAILURE;
        }
    }

    if (pid_zygote == 0 || !app_name || !lib_path || !module_path || !class_name) {
        PrintUsage();
        return EXIT_FAILURE;
    }

    // Start to launcher the designated shared object.
    proc::EggHunter hunter;
    hunter.Hunt(pid_zygote, app_name, lib_path, module_path, class_name);

    return EXIT_SUCCESS;
}