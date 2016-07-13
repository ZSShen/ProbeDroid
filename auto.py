import sys
import argparse
import os
import subprocess
import traceback


OPT_ARCH = "arch"
OPT_PATH = "path"

HELP_ARCH = "The target device architecture. Please specify arm or x86"
HELP_PATH = "The working directory to store our binary. /data/local/tmp for example."

WARN_ARCH = "The --arch argument is not designated."
WARN_PATH = "The --work argument is not designated."
WARN_INVALID_ARCH = "The --arch argument is invalid. Please specify arm or x86."

ARCH_ARM = "arm"
ARCH_X86 = "x86"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--install", dest="install", action="store_true")
    parser.add_argument("--deploy", dest="deploy", action="store_true")
    parser.add_argument("--%s" % (OPT_ARCH), help=HELP_ARCH)
    parser.add_argument("--%s" % (OPT_PATH), help=HELP_PATH)
    parser.set_defaults(install=False)
    parser.set_defaults(deploy=False)

    args = parser.parse_args()
    install = args.install
    deploy = args.deploy
    args = vars(args)

    if install:
        compile_engine()

    if deploy:
        arch = None
        if OPT_ARCH in args:
            arch = args[OPT_ARCH]
        if not arch:
            print WARN_ARCH
            return
        if arch != ARCH_ARM and arch != ARCH_X86:
            print WARN_INVALID_ARCH
            return

        path_device = None
        if OPT_PATH in args:
            path_device = args[OPT_PATH]
        if not path_device:
            print WARN_PATH
            return

        deploy_engine_and_tool(arch, path_device)


def compile_engine():
    cmd_ndk_build = "ndk-build"
    cmd_ant_compile = "ant compile"
    cmd_ant_jar = "ant build-jar"
    path_curr = os.getcwd()

    # Compile the ProbeDroid launcher.
    print("[Build Launcher ...]")
    path_launcher = os.path.join(path_curr, "launcher", "jni")
    os.chdir(path_launcher)
    rc = run_command(cmd_ndk_build)
    if rc == False:
        return
    os.chdir(path_curr)
    print("\n")

    # Compile the ProbeDroid core library.
    print("[Build Core Library ...]")
    path_core = os.path.join(path_curr, "engine", "jni")
    os.chdir(path_core)
    rc = run_command(cmd_ndk_build)
    if rc == False:
        return
    os.chdir(path_curr)
    print("\n")

    # Compile the ProbeDroid exported API jar.
    print("[Build Instrument API Jar ...]")
    path_engine = os.path.join(path_curr, "engine")
    os.chdir(path_engine)
    rc = run_command(cmd_ant_compile)
    if rc == False:
        return
    rc = run_command(cmd_ant_jar)
    if rc == False:
        return
    os.chdir(path_curr)
    print("\n")


def deploy_engine_and_tool(arch, path_device):
    prefix_adb_push = "adb push"
    path_curr = os.getcwd()

    # Migrate the ProbeDroid launcher.
    print("[Migrate Launcher ...]")
    path_launcher = os.path.join(path_curr, "launcher", "libs")
    if arch == ARCH_ARM:
        path_launcher = os.path.join(path_launcher, "armeabi-v7a", "launcher")
    else:
        path_launcher = os.path.join(path_launcher, "x86", "launcher")
    cmd_push = "%s %s %s" % (prefix_adb_push, path_launcher, path_device)
    rc = run_command(cmd_push)
    if rc == False:
        return
    print("")

    # Migrate the ProbeDroid core library.
    print("[Migrate Core Library ...]")
    path_core = os.path.join(path_curr, "engine", "libs")
    if arch == ARCH_ARM:
        path_core = os.path.join(path_core, "armeabi-v7a", "libProbeDroid.so")
    else:
        path_core = os.path.join(path_core, "x86", "libProbeDroid.so")
    cmd_push = "%s %s %s" % (prefix_adb_push, path_core, path_device)
    rc = run_command(cmd_push)
    if rc == False:
        return
    print("")

    # Migrate the instrumentation tools.
    print("[Migrate Instrumentation Tools ...]")
    path_tool = os.path.join(path_curr, "tools")
    list_file = os.listdir(path_tool)
    for name_file in list_file:
        if name_file.endswith(".apk") == False:
            continue
        path_file = os.path.join(path_tool, name_file)
        cmd_push = "%s %s %s" % (prefix_adb_push, path_file, path_device)
        print("%s" % (name_file))
        rc = run_command(cmd_push)
        if rc == False:
            return
    print("")


def run_command(cmd):
    rc = True
    try:
        env = os.environ
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, env=env, shell=True)
        while True:
            line = proc.stdout.readline()
            if line == '':
                break
            print line.rstrip()
    except:
        traceback.print_exc()
        rc = False
    return rc


if __name__ == "__main__":
    main()
