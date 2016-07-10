import sys
import argparse
import os
import subprocess
import traceback


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--install", dest="install", action="store_true")
    parser.set_defaults(install=False)

    args = parser.parse_args()
    install = args.install
    if install:
        compile_engine()


def compile_engine():
    cmd_ndk_build = "ndk-build"
    cmd_ant_compile = "ant compile"
    cmd_ant_jar = "ant build-jar"

    # Compile the ProbeDroid launcher.
    print "[Build Launcher ...]"
    path_curr = os.getcwd()
    path_launcher = os.path.join(path_curr, "launcher", "jni")
    os.chdir(path_launcher)
    run_command(cmd_ndk_build)
    os.chdir(path_curr)
    print "\n"

    # Compile the ProbeDroid core library.
    print "[Build Core Library ...]"
    path_core = os.path.join(path_curr, "engine", "jni")
    os.chdir(path_core)
    run_command(cmd_ndk_build)
    os.chdir(path_curr)
    print "\n"

    # Compile the ProbeDroid exported API jar.
    print "[Build Instrument API Jar ...]"
    path_engine = os.path.join(path_curr, "engine")
    os.chdir(path_engine)
    run_command(cmd_ant_compile)
    run_command(cmd_ant_jar)
    os.chdir(path_curr)
    print "\n"


def run_command(cmd):
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


if __name__ == "__main__":
    main()