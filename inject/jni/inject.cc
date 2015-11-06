#include <util.h>
#include <proc.h>


using namespace util;

#define OPT_LONG_ZYGOTE             "zygote"
#define OPT_LONG_APPNAME            "app"
#define OPT_LONG_LIBPATH            "lib"
#define OPT_ZYGOTE                  'z'
#define OPT_APPNAME                 'a'
#define OPT_LIBPATH                 'l'


void PrintUsage()
{
    const char *usage = "Usage: ./inject --zygote PID --app APPNAME --lib LIBPATH\n"
    "                  -z       PID -a    APPNAME -l    LIBPATH\n\n"
    "Example: ./inject --zygote 933 --app org.zsshen.bmi --lib /data/local/tmp/libhook.so\n"
    "         ./inject -z       933 -a    org.zsshen.bmi -l    /data/local/tmp/libhook.so\n\n";
    std::cout << usage;
}


int32_t main(int32_t argc, char** argv)
{
    // Acquire the command line arguments.
    static struct option opts[] = {
        {OPT_LONG_APPNAME, required_argument, 0, OPT_APPNAME},
        {OPT_LONG_LIBPATH, required_argument, 0, OPT_LIBPATH},
    };

    char sz_order[SIZE_TINY_BLAH];
    memset(sz_order, 0, sizeof(char) * SIZE_TINY_BLAH);
    sprintf(sz_order, "%c:%c:%c:", OPT_ZYGOTE, OPT_APPNAME, OPT_LIBPATH);

    int32_t opt, idx_opt;
    pid_t pid_zygote = 0;
    char *sz_app = NULL, *path_lib = NULL;
    while ((opt = getopt_long(argc, argv, sz_order, opts, &idx_opt)) != -1) {
        switch (opt) {
            case OPT_ZYGOTE:
                pid_zygote = atoi(optarg);
                break;
            case OPT_APPNAME:
                sz_app = optarg;
                break;
            case OPT_LIBPATH:
                path_lib = optarg;
                break;
            default:
                PrintUsage();
                return FAIL;
        }
    }

    if (pid_zygote == 0 || !sz_app || !path_lib) {
        PrintUsage();
        return FAIL;
    }

    // Start to inject the designated shared object.
    proc::EggHunter hunter;
    hunter.Hunt(pid_zygote, sz_app);

    return SUCC;
}