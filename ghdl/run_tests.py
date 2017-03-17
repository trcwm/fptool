#
# Run FPTool tests
#

from subprocess import call

# create the VHDL file
call(["../build/debug/fptool", "..\\tests\\csd_test.fp", "-o", "csd_test.vhdl", "-d"])

# concatenate prolog and epilog for VHDL file
filenames = ['csd_test_prolog.vhdl', 'csd_test.vhdl', 'csd_test_epilog.vhdl']
with open('csd_test_final.vhdl', 'w') as outfile:
    for fname in filenames:
        with open(fname) as infile:
            outfile.write(infile.read())

# run ghdl on it
# ghdl -c csd_test_final.vhdl -r tb --wave=csd_test_final.ghw
call(["ghdl", "-c", "csd_test_final.vhdl", "-r", "tb", "--wave=csd_test_final.ghw"])

