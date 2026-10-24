"""
Triage Macros for zone-related panics.

Supported panic strings from xnu/osfmk/kern/zalloc.c:
    - "a freed zone element has been modified in zone %s..."
    - "zalloc: zone map exhausted while allocating from zone %s..."

To support more zone panic strings:
    1. Add a regex for the panic string with a named capture group 'zone'.
    2. Extend ZoneTriage() and CheckZoneBootArgs() to detect your regex.
    3. Create a handler function for the new panic type.
"""

from xnu import *
import sys
import shlex
import re
import os.path
from utils import *
import xnudefines


# Globals
panic_string = None

# Panic string regex patterns
ZONE_ELEMENT_MODIFIED = (
    r".*a freed zone element has been modified in zone (?P<zone>.+): "
    r"expected (0x)?([0-9A-Fa-f]*)? but found (0x)?([0-9A-Fa-f]*)?, "
    r"bits changed (0x)?([0-9A-Fa-f]*)?, at offset ([0-9]*)? of ([0-9]*)? "
    r"in element (?P<element>0x[0-9A-Fa-f]*), cookies (0x)?([0-9A-Fa-f]*)? "
    r"(0x)?([0-9A-Fa-f]*)?.*"
)
ZONE_MAP_EXHAUSTED = (
    r".*zalloc: zone map exhausted while allocating from zone .+, "
    r"likely due to memory leak in zone (?P<zone>.+) "
    r"\(([0-9]*)? total bytes, ([0-9]*)? elements allocated\).*"
)


@lldb_command("zonetriage")
def zone_triage(cmd_args=None):
    """
    Calls a function specific to the type of zone panic based on the panic string.
    """
    global panic_string
    if panic_string is None:
        try:
            panic_string = lldb_run_command("paniclog").split("\n", 1)[0]
        except Exception:
            return

    if re.match(ZONE_ELEMENT_MODIFIED, panic_string):
        zone_triage_freed_element()
    elif re.match(ZONE_MAP_EXHAUSTED, panic_string):
        zone_triage_memory_leak()
    else:
        print("zonetriage does not currently support this panic string.")


@lldb_command("zonetriage_freedelement")
def zone_triage_freed_element(cmd_args=None):
    """
    Runs zstack_findelem on the element and zone being logged based on the panic string regex.
    """
    global panic_string
    if panic_string is None:
        try:
            panic_string = lldb_run_command("paniclog").split("\n", 1)[0]
        except Exception:
            return

    check_zone_boot_args()

    print(f"(lldb) zstack_showzonesbeinglogged\n{lldb_run_command('zstack_showzonesbeinglogged')}\n")

    match = re.search(ZONE_ELEMENT_MODIFIED, panic_string)
    if not match or "zone" not in match.group() or "element" not in match.group():
        return

    element = match.group("element")
    zone = match.group("zone")
    btlog = find_zone_btlog(zone)

    if btlog is not None:
        print(f"(lldb) zstack_findelem {btlog} {element}")
        output = lldb_run_command(f"zstack_findelem {btlog} {element}")
        output = re.sub(
            r"Scanning is ongoing. [0-9]* items scanned since last check.\n",
            "",
            output,
        )
        print(output)


@lldb_command("zonetriage_memoryleak")
def zone_triage_memory_leak(cmd_args=None):
    """
    Runs zstack_findtop and zstack_findleak on all zones being logged.
    """
    global kern
    check_zone_boot_args()

    print(f"(lldb) zstack_showzonesbeinglogged\n{lldb_run_command('zstack_showzonesbeinglogged')}\n")

    for zval in kern.zones:
        if zval.zlog_btlog:
            print(f"{zval.zone_name}:")
            print(f"(lldb) zstack_findtop -N 5 0x{zval.zlog_btlog:x}")
            print(lldb_run_command(f"zstack_findtop -N 5 0x{zval.zlog_btlog:x}"))
            print(f"(lldb) zstack_findleak 0x{zval.zlog_btlog:x}")
            print(lldb_run_command(f"zstack_findleak 0x{zval.zlog_btlog:x}"))


def check_zone_boot_args(cmd_args=None):
    """
    Check boot args to see if the zone is being logged. Suggest new args if not.
    """
    global panic_string
    if panic_string is None:
        try:
            panic_string = lldb_run_command("paniclog").split("\n", 1)[0]
        except Exception:
            return

    panic_regex = ""
    if re.match(ZONE_ELEMENT_MODIFIED, panic_string):
        panic_regex = ZONE_ELEMENT_MODIFIED
    elif re.match(ZONE_MAP_EXHAUSTED, panic_string):
        panic_regex = ZONE_MAP_EXHAUSTED

    values = re.search(panic_regex, panic_string)
    if not values or "zone" not in values.group():
        return

    zone = values.group("zone")
    boot_args = lldb_run_command("showbootargs")
    correct = re.search(r"zlog([1-9]|10)?=" + re.sub(" ", ".", zone), boot_args)

    if correct is None:
        print("Current boot-args:\n" + boot_args)
        print("You may need to include: -zc -zp zlog([1-9]|10)?=" + re.sub(" ", ".", zone))


def find_zone_btlog(zone):
    """
    Return the btlog address (0x%lx) for the given zone name.
    """
    global kern
    for zval in kern.zones:
        if zval.zlog_btlog and zone == f"{zval.zone_name}":
            return f"0x{zval.zlog_btlog:x}"
    return None
