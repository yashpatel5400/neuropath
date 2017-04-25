"""
__name__ = settings.py
__author__ = Yash Patel
__description__ = Settings for parsing in the files (i.e. meta
data of dump format)
"""

# ----------------------------- Trace Headers -------------------------
# counter for the micro-ops within a macro-op.
UOP         = 0

# address of the instruction being executed. 
PC          = 1

# first source register: A value of -1 indicates that no register is
# read for the source.
SRC1        = 2

# second source register
SRC2        = 3

# destination registers. A value of -1 indicates that no register written
DEST        = 4

# indicates if the instruction reads or writes the condition codes
# signifies if the micro-op reads the flags ('R'), writes the flags
# ('W'), or neither ('-')
FLAGS       = 5

# indicates if a micro-op is "taken" to the target PC ('T'),
# "not taken" and thus proceeds to the fall through PC ('N'), or a
# non-branch micro-operation ('-')
BRANCH      = 6

# indicates if a micro-op loads from memory ('L'), stores to memory
# ()S'), or neither ('-')
LD          = 7

# indicates the value of any immediate to the instruction
IMMEDIATE   = 8

# indicates the address accessed by a load or store. If the micro-op
# doesn't access memory, this field will have the value zero
MEMADDR     = 9

# indicates the address of the instruction that immediately follows
# this instruction in memory; if this instruction is a not-taken
# branch, this field holds the address of the instruction that will be executed.
FALLTHROUGH = 10

# indicates the address that a branch will go to if the branch is taken
TARGET      = 11

# printable string that corresponds to the x86 assembly instruction
MACRO       = 12

# printable string that describes the specific micro-ops
MICRO       = 13
