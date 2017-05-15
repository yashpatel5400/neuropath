"""
__name__ = predict.py
__author__ = Yash Patel
__description__ = Script to run executables heavily relying on the
performance of branch predictors to compare their performance. Compares
the default ones packaged in the gem5 environment with the custom ones
"""

import argparse

import m5
from m5.objects import *

def simulate_BP(predictor, executable):
    """
    Given ints corresponding to the branch predictor to use in the gem5
    environment in addition to the executable to test on, runs the
    simulations, outputting results to the gem5/m5cached directories. The
    output is copied from the standard output directory to that specified
    by the executable
    @param predictor The address of the branch to look up.
    @param bp_history Pointer to any bp history state.
    @return void
    """

    # create the system we are going to simulate
    system = System()

    # Set the clock fequency of the system (and all of its children)
    system.clk_domain = SrcClockDomain()
    system.clk_domain.clock = '1GHz'
    system.clk_domain.voltage_domain = VoltageDomain()

    # Set up the system
    system.mem_mode = 'timing'               # Use timing accesses
    system.mem_ranges = [AddrRange('8192MB')] # Create an address range

    # Create a simple CPU
    system.cpu = TimingSimpleCPU()

    # --------------------------- Main Alteration ---------------------------- #
    # default branch predictors of the gem5 simulator ecosystem
    branchPredictors = [
        LocalBP(),      # simple local history branch predictor
        TournamentBP(), # mixed global/local history predictor
        BiModeBP(),     # 2-bit history mode predictor
        LTAGE(),        # often best-performing current mainstream predictor
        AlwaysBP(),     # always true branch predictor (static)
        NeuroBP()       # single perceptron neural branch predictor
    ]

    system.cpu.branchPred = branchPredictors[predictor]
    # ----------------------------------------------------------------------- #

    # Create a memory bus, a coherent crossbar, in this case
    system.membus = SystemXBar()

    # Hook the CPU ports up to the membus
    system.cpu.icache_port = system.membus.slave
    system.cpu.dcache_port = system.membus.slave

    # create the interrupt controller for the CPU and connect to the membus
    system.cpu.createInterruptController()
    system.cpu.interrupts[0].pio = system.membus.master
    system.cpu.interrupts[0].int_master = system.membus.slave
    system.cpu.interrupts[0].int_slave = system.membus.master

    # Create a DDR3 memory controller and connect it to the membus
    system.mem_ctrl = DDR3_1600_8x8()
    system.mem_ctrl.range = system.mem_ranges[0]
    system.mem_ctrl.port = system.membus.master

    # Connect the system up to the membus
    system.system_port = system.membus.slave

    # Create a process for a simple "Hello World" application
    process = Process()

    # Set the command
    # cmd is a list which begins with the executable (like argv)
    commands = [
        ["tests/test-progs/predict/graph/connected-components", "--with-gem5=True",
         "--with-cmov=True", "tests/test-progs/predict/graph/small.graph"],  # 0
        ["tests/test-progs/predict/graph/connected-components", "--with-gem5=True",
         "--with-cmov=True", "tests/test-progs/predict/graph/medium.graph"], # 1
        ["tests/test-progs/predict/graph/connected-components", "--with-gem5=True",
         "--with-cmov=True", "tests/test-progs/predict/graph/large.graph"],  # 2
        
        ["tests/test-progs/predict/stanford/Bubblesort"], # 3
        ["tests/test-progs/predict/stanford/IntMM"],      # 4
        ["tests/test-progs/predict/stanford/Oscar"],      # 5
        ["tests/test-progs/predict/stanford/Perm"],       # 6
        ["tests/test-progs/predict/stanford/Puzzle"],     # 7
        ["tests/test-progs/predict/stanford/Queens"],     # 8
        ["tests/test-progs/predict/stanford/Quicksort"],  # 9
        ["tests/test-progs/predict/stanford/RealMM"],     # 10
        ["tests/test-progs/predict/stanford/Towers"],     # 11
        ["tests/test-progs/predict/stanford/Treesort"],   # 12

        ["tests/test-progs/predict/examples/primes"],     # 13
        ["tests/test-progs/predict/examples/shell_sort"]  # 14
    ]

    process.cmd = commands[executable]

    # Set the cpu to use the process as its workload and create thread contexts
    system.cpu.workload = process
    system.cpu.createThreads()

    # set up the root SimObject and start the simulation
    root = Root(full_system = False, system = system)

    # instantiate all of the objects we've created above
    m5.instantiate()

    print "Beginning simulation!"
    exit_event = m5.simulate()
    print 'Exiting @ tick %i because %s' % (m5.curTick(), exit_event.getCause())

# --------------------------- Runs simulation ---------------------------- #
parser = argparse.ArgumentParser(
    description='Predict branches for different executables')
parser.add_argument('--pred', metavar='pred', type=int,
                    help="""type of branch predictor to be used by the CPU:
                    (0-2)  : Connected components executables
                    (3-12) : Stanford benchmarking executables
                    (13-14): Custom benchmarks
                    """)
parser.add_argument('--exec', metavar='exec', type=int,
                    help="""executable to be used for testing/benchmarking:
                    (0) LocalBP
                    (1) TournamentBP
                    (2) Bimodal
                    (3) LTAGE
                    (4) StaticBP
                    (5) NeuralBP
"""
)

args = parser.parse_args()
simulate_BP(predictor=vars(args)["pred"], executable=vars(args)["exec"])
