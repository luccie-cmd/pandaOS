#!/bin/python3
#
# Copyright (c) - All Rights Reserved.
# 
# See the LICENCE file for more information.
#


import os
import glob
import sys
import subprocess
import getpass
from script.util import parseSize, compareFiles
basename = os.path.basename(os.path.dirname(os.path.realpath(__file__)))

def readConfig(path: str) -> dict[str, list[str]]:
    """Reads a configuration file and returns a dictionary of lists of strings."""
    config = {}
    try:
        with open(path, "r") as f:
            lines = f.readlines()
            for line in lines:
                line = line.strip()
                if ':' in line:
                    key, value = line.split(":", 1)  # Split at the first ':'
                    key = key.strip()
                    value = value.strip().strip("'")  # Strip leading/trailing whitespace and single quotes
                    config[key] = [v.strip().strip("'") for v in value.split(",")]  # Split by ',' and strip quotes from each part
    except FileNotFoundError:
        print(f"Error: Configuration file '{path}' not found.")
        return config
    return config

    
def writeConfig(config: dict[str, list[str]], path: str) -> None:
    """Writes a configuration dictionary to a file."""
    with open(path, "w") as f:
        for key, values in config.items():
            f.write(f"{key}:")
            for val in enumerate(values):
                f.write(f"'{val[1]}'")
                if val[0]+1 < len(values):
                    # pass
                    f.write(", ")
            f.write('\n')

def checkConfig(config: dict[str, list[str]], allowed_config: list[list[str, list[str], bool]]) -> bool:
    """Checks if a configuration dictionary is valid."""
    for key, values in config.items():
        print(key, values)
        for allowed_key, allowed_values, required in allowed_config:
            if key == allowed_key:
                if len(allowed_values) == 0:
                    continue
                if required and len(values) == 0:
                    return False
                if not required and len(values) > 0:
                    return False
                for val in values:
                    if val not in allowed_values:
                        return False
    return True

CONFIG = readConfig("./script/config.py")
OLD_CONFIG = readConfig("./script/config.py.old")
ALLOWED_CONFIG = [
    ["config", ["release", "debug"], True],
    ["arch", ["x64"], True],
    ["compiler", ["gcc", "clang"], True],
    ["imageFS", ["fat32"], True],
    ["bootloader", ["limine-uefi", "bios", "multiboot2"], True],
    ["outDir", [], True],
    ["imageSize", [], False],
    ["mt19937", ["yes", "no"], True]
]
if not checkConfig(CONFIG, ALLOWED_CONFIG):
    print("Invalid config file.")
    print("Allowed config items")
    for option in ALLOWED_CONFIG:
        name = option[0]
        values = option[1]
        required = option[2]
        print(f"{name} (required = {required})")
        if len(values) == 0:
            print("    This can be anything as long as it's provided")
        else:
            for val in values:
                print(f"  - {val}")
    exit(1)
writeConfig(CONFIG, "./script/config.py.old")
force_rebuild = False
if OLD_CONFIG != CONFIG:
    force_rebuild = True
    print("Configuration changed, rebuilding...")
# Add some default values to the config
CONFIG["CFLAGS"] = ['-Werror', '-nostdlib', '-c', '-mno-avx512f', '-D_GLIBCXX_HOSTED', '-finline-functions', '-fno-pic', '-mno-red-zone', '-fno-stack-protector', '-fno-lto', '-fno-stack-check', '-mno-avx', '-Wall', '-Wextra']
CONFIG["INCPATHS"] = ['-Ixed', '-Iklibc']
CONFIG["CXXFLAGS"] = ['-fno-rtti', '-fno-exceptions']
CONFIG["ASFLAGS"] = ['-felf64']
CONFIG["LDFLAGS"] = ['-nostdlib', '-no-pie']
if "imageSize" not in CONFIG:
    CONFIG["imageSize"] = '128m'

if "debug" in CONFIG.get("config"):
    CONFIG["CFLAGS"] += ["-O0"]
    CONFIG["CFLAGS"] += ["-g"]
    CONFIG["CFLAGS"] += ["-DDEBUG"]
else:
    CONFIG["CFLAGS"] += ["-O2"]
    CONFIG["CFLAGS"] += ["-DNDEBUG"]
if "yes" in CONFIG.get("mt19937"):
    CONFIG["CFLAGS"] += ["-DMT19937"]
if "x64" in CONFIG.get("arch"):
    CONFIG["CFLAGS"] += ["-m64"]

if "debug" in CONFIG.get("config"):
    CONFIG["LDFLAGS"] += ["-O0"]
else:
    CONFIG["LDFLAGS"] += ["-O2"]

def callCmd(command, print_out=False):
    with open("commands.txt", "a") as f:
        f.write(command+'\n')
    # Run the command and capture the output
    # result = subprocess.run(command, capture_output=False, text=True, shell=True)
    result = subprocess.run(command, capture_output=not print_out, text=True, shell=True)
    if result.returncode != 0:
        print(result.stderr)
    return [result.returncode, result.stdout]

callCmd("rm -rf commands.txt")

def checkExtension(file: str, valid_extensions: list[str]):
    for ext in valid_extensions:
        if file.endswith(ext):
            return True
    return False

def getExtension(file):
    return file.split(".")[-1]

def buildC(file):
    compiler = CONFIG["compiler"][0]
    options = CONFIG["CFLAGS"].copy()
    options.append("-std=c11")
    command = compiler + " " + file
    for option in options:
        command += " " + option
    print(f"C     {file}")
    command += f" -o {CONFIG['outDir'][0]}/{file}.o"
    return callCmd(command, True)[0]

def buildCXX(file):
    compiler = CONFIG["compiler"][0]
    if compiler == "gcc":
        compiler = "g"
    compiler += "++"
    options = CONFIG["CFLAGS"].copy()
    options += CONFIG["CXXFLAGS"].copy()
    options.append("-std=c++23")
    command = compiler + " " + file
    for option in options:
        command += " " + option
    print(f"CXX   {file}")
    command += f" -o {CONFIG['outDir'][0]}/{file}.o"
    return callCmd(command, True)[0]

def buildASM(file):
    compiler = "nasm"
    options = CONFIG["ASFLAGS"].copy()
    command = compiler + " " + file
    for option in options:
        command += " " + option
    print(f"AS    {file}")
    command += f" -o {CONFIG['outDir'][0]}/{file}.o"
    return callCmd(command, True)[0]

def buildKernel(kernel_dir: str):
    files = glob.glob(kernel_dir+'/**', recursive=True)
    CONFIG["INCPATHS"] += [f"-I{kernel_dir}"]
    for file in files:
        if not os.path.isfile(file):
            continue
        if not checkExtension(file, ["c", "cc", "asm"]):
            continue
        if getExtension(file) == "inc" or getExtension(file) == "h":
            continue
        basename = os.path.basename(os.path.dirname(os.path.realpath(__file__)))
        str_paths = ""
        for incPath in CONFIG["INCPATHS"]:
            str_paths += f" {incPath}"
        callCmd(f"cpp {str_paths} {file} -o ./tmp.txt")
        if not force_rebuild and compareFiles("./tmp.txt", os.path.abspath(f"/tmp/{basename}/cache/{file}")):
            continue
        callCmd(f"mkdir -p {CONFIG['outDir'][0]}/{os.path.dirname(file)}")
        callCmd(f"mkdir -p /tmp/{basename}/cache/{os.path.dirname(file)}")
        callCmd(f"cp ./tmp.txt /tmp/{basename}/cache/{file}")
        code = 0
        CONFIG["CFLAGS"] += CONFIG["INCPATHS"]
        if getExtension(file) == "c":
            code = buildC(file)
        elif getExtension(file) == "asm":
            code = buildASM(file)
        elif getExtension(file) == "cc":
            code = buildCXX(file)
        else:
            print(f"Invalid or unhandled extension `{getExtension(file)}` on file {file}")
            exit(1)

        for incPath in CONFIG["INCPATHS"]:
            CONFIG["CFLAGS"].remove(incPath)

        if code != 0:
            callCmd(f"rm -f /tmp/{basename}/cache/{file}")
            exit(code)

def linkKernel(kernel_dir, linker_file, libc_file, lib_dir="./lib"):
    files = glob.glob(kernel_dir+'/**', recursive=True)
    command = "ld"
    options = CONFIG["LDFLAGS"]
    for option in options:
        command += " " + option
    for file in files:
        if not os.path.isfile(file):
            continue
        if not checkExtension(file, ["o"]):
            continue
        command += " " + file
    command += f" -T {linker_file}"
    command += " --no-whole-archive"
    command += " --whole-archive"
    command += f" {libc_file}"
    if CONFIG["config"][0] == "debug":
        command += f" -Map={CONFIG['outDir'][0]}/kernel.map"
    lib_files = glob.glob(lib_dir+'/**')
    for file in lib_files:
        if getExtension(file) == 'a':
            command += f" {file}"
    command += f" -o {CONFIG['outDir'][0]}/kernel.elf"
    callCmd(command, True)
    callCmd(f"strip -g -s -x -X {CONFIG['outDir'][0]}/kernel.elf -o {CONFIG['outDir'][0]}/kernel.elf")

def makeImageFile(out_file):
    size = parseSize(CONFIG["imageSize"][0])
    command = f"dd if=/dev/zero of={out_file} bs=1M count={size//parseSize("1M")}"
    callCmd(command)

def makePartitionTable(out_file):
    print("> Making GPT partition")
    command = f"parted {out_file} --script mklabel gpt"
    callCmd(command)
    print("> Making EFI partition")
    command = f"parted {out_file} --script mkpart EFI {CONFIG['imageFS'][0].upper()} 1MB 100MB"
    callCmd(command)
    print("> Setting EFI partition to be bootable")
    command = f"parted {out_file} --script set 1 boot on"
    callCmd(command)
    command = f"parted {out_file} --script set 1 esp on"
    callCmd(command)

def setupLoopDevice(out_file):
    print("> Setting up loop device")
    command = f"sudo losetup --show -f -P {out_file} > /tmp/tmp.txt"
    callCmd(command)
    loop_device = ""
    with open("/tmp/tmp.txt") as f:
        loop_device = f.readline()
    loop_device = loop_device.strip()
    print(f"> Loop device: {loop_device}")
    return loop_device

def makeFileSystem(loop_device):
    callCmd(f"sudo mkfs.fat -F32 {loop_device}p1")

def mountFs(device, boot, kernel):
    callCmd(f"mkdir -p mnt")
    callCmd(f"sudo mount {device}p1 mnt")
    callCmd(f"sudo mkdir -p mnt/EFI/BOOT")
    if "limine-uefi" in CONFIG["bootloader"]:
        callCmd(f"sudo cp {CONFIG['outDir'][0]}/limine.conf mnt/EFI/BOOT")
    callCmd(f"sudo mkdir -p mnt/boot")
    callCmd(f"sudo cp {kernel} mnt/boot")
    callCmd(f"sudo cp {CONFIG['outDir'][0]}/syscall.tbl mnt/boot")
    callCmd(f"sudo cp -r image/* mnt")
    callCmd(f"sudo cp {boot} mnt/EFI/BOOT")
    callCmd(f"sudo umount mnt")
    callCmd(f"sudo losetup -d {device}")
    callCmd(f"rm -rf mnt")


def buildImage(out_file, boot_file, kernel_file):
    callCmd(f"rm -f {out_file}")
    makeImageFile(out_file)
    makePartitionTable(out_file)
    LOOP_DEVICE=setupLoopDevice(out_file)
    makeFileSystem(LOOP_DEVICE)
    mountFs(LOOP_DEVICE, boot_file, kernel_file)

def buildLibc(directory, out_file):
    os.makedirs(CONFIG["outDir"][0]+'/'+directory, exist_ok=True)
    CONFIG["INCPATHS"] += [f'-I{directory}']
    files = glob.glob(directory+'/**', recursive=True)
    for file in files:
        if not os.path.isfile(file):
            continue
        if not checkExtension(file, ["c", "cc", "asm"]):
            continue
        basename = os.path.basename(os.path.dirname(os.path.realpath(__file__)))
        str_paths = ""
        for incPath in CONFIG["INCPATHS"]:
            str_paths += f" {incPath}"
        callCmd(f"cpp {str_paths} {file} -o ./tmp.txt")
        if not force_rebuild and compareFiles("./tmp.txt", os.path.abspath(f"/tmp/{basename}/cache/{file}")):
            continue
        callCmd(f"mkdir -p {CONFIG['outDir'][0]}/{os.path.dirname(file)}")
        callCmd(f"mkdir -p /tmp/{basename}/cache/{os.path.dirname(file)}")
        callCmd(f"cp ./tmp.txt /tmp/{basename}/cache/{file}")
        code = 0
        CONFIG["CFLAGS"] += CONFIG["INCPATHS"]
        if getExtension(file) == "c":
            code = buildC(file)
        elif getExtension(file) == "asm":
            code = buildASM(file)
        elif getExtension(file) == "cc":
            code = buildCXX(file)
        else:
            print(f"Invalid or unhandled extension {getExtension(file)}")
            exit(1)

        for incPath in CONFIG["INCPATHS"]:
            CONFIG["CFLAGS"].remove(incPath)

        if code != 0:
            callCmd(f"rm -f /tmp/{basename}/cache/{file}")
            exit(code)
    
    files = glob.glob(f"{CONFIG['outDir'][0]}/{directory}/**", recursive=True)
    obj_files = []
    for file in files:
        if not os.path.isfile(file):
            continue
        if not checkExtension(file, ["o"]):
            continue
        obj_files.append(file)
    obj_files_str = " ".join(obj_files)
    cmd = f"ar rcs {out_file} {obj_files_str}"
    print(f"AR    {out_file}")
    callCmd(cmd)

def limineBootloaderStuff():
    build_limine: bool = False
    if not os.path.exists("limine"):
        build_limine = True
    elif not os.path.exists("limine/bin/BOOTX64.EFI"):
        build_limine = True
    if build_limine:
        print("Building limine")
        callCmd("git clone --depth=1 https://github.com/limine-bootloader/limine", True)
        os.chdir("limine")
        callCmd("./bootstrap")
        callCmd("./configure --enable-uefi-x86-64")
        callCmd("make -B")
        os.chdir("..")
        callCmd("rm -rf ./limine/commands.txt")
    callCmd(f"cp ./util/limine.conf {CONFIG['outDir'][0]}/limine.conf")

def main():
    if "clean" in sys.argv:
        callCmd(f"rm -rf /tmp/{basename}")
        callCmd(f"rm -rf {CONFIG['outDir'][0]}")
    if "clean-all" in sys.argv:
        callCmd(f"rm -rf limine")
        callCmd(f"rm -rf /tmp/{basename}")
        callCmd(f"rm -rf {CONFIG['outDir'][0]}")
    if force_rebuild:
        print("Rebuilding...")
    print("> Creating necesarry dirs")
    callCmd(f"mkdir -p {CONFIG['outDir'][0]}")
    callCmd(f"mkdir -p {CONFIG['outDir'][0]}/kernel")
    if 'limine-uefi' in CONFIG["bootloader"]:
        print("> Limine bootloader selected")
        limineBootloaderStuff()
    else:
        print("TODO: Other bootloaders")
        exit(1)
    callCmd(f"cp kernel/syscall/syscall.tbl {CONFIG['outDir'][0]}")
    print("> Building LibC")
    buildLibc("klibc", f"{CONFIG['outDir'][0]}/libkc.a")
    print("> Building kernel")
    buildKernel("kernel")
    print("> Linking kernel")
    linkKernel(f"{CONFIG['outDir'][0]}/kernel", "util/kernel.ld", f"{CONFIG['outDir'][0]}/libkc.a")
    if "compile" in sys.argv:
        return
    buildImage(f"{CONFIG['outDir'][0]}/image.img", f"limine/bin/BOOTX64.EFI", f"{CONFIG['outDir'][0]}/kernel.elf")
    if "run" in sys.argv:
        callCmd(f"./script/run.sh {CONFIG['outDir'][0]}", True)
    username = getpass.getuser()
    callCmd(f"chown -R {username}:{username} ./*")

if __name__ == '__main__':
    main()