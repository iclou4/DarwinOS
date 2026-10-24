"""
XNU Triage commands
"""

from xnu import *
import sys
import shlex
import re
import os.path
from utils import *
import xnudefines


# Macro: xi
def output_address(cmd_args=None):
    """
    Returns address and symbol corresponding to it without newline.

    Parameters:
        <address whose symbol is needed>
    """
    if not cmd_args:
        print("No arguments passed")
        print(output_address.__doc__)
        return False

    addr = unsigned(cmd_args[0])
    cmd_str = f"image lookup -a {addr:#x}"
    cmd_out = lldb_run_command(cmd_str)

    if len(cmd_out) != 0 and cmd_out != "ERROR:":
        cmd_lines = cmd_out.split("\n")
        if cmd_lines:
            parts = cmd_lines[1].split("`")
            if parts:
                sym_parts = parts[1].split(" at")
                if sym_parts:
                    symbol_str = f"{addr:#x} <{sym_parts[0]:s}>"
                    return symbol_str
    return ""


@lldb_command("xi")
def symbolicate_with_instruction(cmd_args=None):
    """
    Prints out address and symbol similar to x/i.
    Usage: xi <address whose symbol is needed>
    """
    if not cmd_args:
        print("No arguments passed")
        print(symbolicate_with_instruction.__doc__)
        return False

    addr = ArgumentStringToInt(cmd_args[0])
    print(output_address([addr]))


# Macro: newbt
@lldb_command("newbt")
def new_bt(cmd_args=None):
    """
    Prints all the instructions by walking the given stack pointer.
    """
    if not cmd_args:
        print("No arguments passed")
        print(new_bt.__doc__)
        return False

    sp = ArgumentStringToInt(cmd_args[0])
    while sp != 0:
        offset = 8 if kern.arch == "x86_64" or kern.arch.startswith("arm64") else 4
        link_register = dereference(kern.GetValueFromAddress(sp + offset, "uintptr_t *"))
        cmd_str = f"di -s {link_register:#x} -c 1"
        cmd_out = lldb_run_command(cmd_str)

        if len(cmd_out) != 0:
            lines = cmd_out.split("\n")
            if lines:
                address = output_address([unsigned(link_register)])
                if address is None:
                    address = f"0x{unsigned(link_register):x} <???>"
                print(f"{address}: {lines[1].split(':', 1)[1]}")
        sp = dereference(kern.GetValueFromAddress(unsigned(sp), "uintptr_t *"))


# Macro: parseLR
@lldb_command("parseLR")
def parse_lr(cmd_args=None):
    """
    Decode the LR value from panic log into source code location.
    """
    global paniclog_data
    panic_found = 1

    if not paniclog_data:
        if kern.arch == "x86_64":
            paniclog_data += returnfunc("\n(lldb) paniclog\n", "paniclog -v")
        else:
            paniclog_data += returnfunc("\n(lldb) paniclog\n", "paniclog")

    if panic_found == 1:
        srch_string = r"lr:\s+0x[a-fA-F0-9]+\s"
        lr_pc_srch = re.findall(srch_string, paniclog_data)
        if lr_pc_srch:
            print(paniclog_data, lr_pc_srch)
            for match in lr_pc_srch:
                sp = match.strip("lr: ")
                print(sp)
                print(f"(lldb) list *{sp}")
                print(lldb_run_command(f"list *{sp}"))
    else:
        print("Currently unsupported on x86_64 architecture")


# Macro: parseLRfromfile
@lldb_command("parseLRfromfile")
def parse_lr_from_file(cmd_args=None):
    """
    Decode the LR value from file into source code location.
    """
    with open("/tmp/lrparsefile", "r") as f:
        parse_data = f.read()

    srch_string = r"lr:\s+0x[a-fA-F0-9]+\s"
    lr_pc_srch = re.findall(srch_string, parse_data)

    if lr_pc_srch:
        print(paniclog_data, lr_pc_srch)
        for match in lr_pc_srch:
            sp = match.strip("lr: ")
            print(sp)
            print(f"(lldb) list *{sp}")
            print(lldb_run_command(f"list *{sp}"))
