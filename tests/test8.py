#!/bin/python3

import subprocess
import os
import time
import sys

colors = {'grey': 30, 'red': 31, 'green': 32, 'yellow': 33,
          'blue': 34, 'magenta': 35, 'cyan': 36, 'white': 37}

def colorize(s, color, bold=False):
    if os.getenv('ANSI_COLORS_DISABLED') is None and color in colors:
        if bold:
            return '\033[1m\033[%dm%s\033[0m' % (colors[color], s)
        else:
            return '\033[%dm%s\033[0m' % (colors[color], s)
    else:
        return s

def print_time(chrono):
    return (colorize(' TIME[','yellow') + colorize(str(chrono),'yellow')
    + colorize(']  ','yellow'))

def print_space(t,n):
    if len(t) < n:
        for i in range (len(t),n):
            t += ' '
    return t

def result_tests(passed, total, chrono, color='cyan'):
    print(colorize('╔══════════════════════════════════════════════════════╗',color))
    print(colorize('║',color) +  "test pass    :",passed,"/",total,(34 - len(str(passed)) - len(str(total))) * " ", colorize('║',color))
    print(colorize('║',color) +  "test failed  :",total - passed,"/",total,(34 - len(str(total - passed)) - len(str(total))) * " ", colorize('║',color))
    print(colorize('║',color) +  "total time   :",chrono, (37 - len(str(chrono))) * " ", colorize('║',color))
    print(colorize('╚══════════════════════════════════════════════════════╝',color))

def print_error(ls):
    s = ""
    for i in range(len(ls)):
        if len(ls) - i - 1 > 0:
            s += colorize('  │','red') * (len(ls) - i - 1)
        if True:
            s += colorize('  └──────➤   ','red') + ls[i]
        print(s);
        s = ""

def tests(directory, binary):
    total = 0
    passed = 0
    start = time.time()
    ls = []
    my_env = os.environ.copy()
    my_env['DIR'] = directory
    my_env['BINARY'] = binary
    print(colorize(directory, 'white', True))
    for s in os.listdir(directory):
        if not s.startswith('Makefile'):
            continue
        my_env['MAKEFILE'] = s
        ref = subprocess.Popen([binary, "-f", directory + s],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=my_env)
        ref_out, ref_err = ref.communicate()
        ref_ret = ref.returncode
        ref_out = ref_out.decode('utf-8')[:-1]
        my = subprocess.Popen(['make', "-f", directory + s],
                stdout=subprocess.PIPE, stderr=subprocess.PIPE, env=my_env)
        my_out,my_err = ref.communicate()
        my_ret = ref.returncode
        my_out = my_out.decode('utf-8')[:-1]
        my_err = my_err.decode('utf-8')[:-1]
        if my_ret != ref_ret or (my_ret and my_err == "") \
                or (not my_ret and my_err != "") or my_out != ref_out:
            if my_ret and my_err == "":
                ls.append("returned with error but did not output message: " + str(my_ret));
            if not my_ret and my_err != "":
                ls.append("returned with success but did output message on error:\n" \
                        + my_err + '\n');
            if my_ret != ref_ret:
                ls.append("returned with " + str(my_ret) + ", expected " + str(ref_ret))
            if my_out != ref_out:
                ls.append("bad output: got:\n" + my_out + "\nExcpected:\n" + my_err)
            print(colorize('[KO] ', 'red'), s)
            print_error(ls)
        else:
            print(colorize('[OK] ','green'), s)
            passed += 1
        total += 1
        ls = []
    end = time.time()
    chrono = round(end-start,4)
    result_tests(passed, total, chrono)
    return (total, passed, chrono)

if __name__ == '__main__':
    tests_res = []
    total_res = 0
    passed_res = 0
    chrono_res = 0
    path = sys.argv[2] + "tests/"

    if not os.path.exists(sys.argv[1]) or not os.path.isdir(path):
        print(sys.argv[0] + ": Error: The binary does not exist")
    else:
        for s in os.listdir(path):
            tests_res.append(tests(path + s + "/", sys.argv[1]))
            print()

    if len(tests_res) > 1:
        print()
        print(colorize(" -------------------- FINAL RESULTS -------------------", 'yellow', True))
        for (total, passed, chrono) in tests_res:
            total_res += total
            passed_res += passed
            chrono_res += chrono
        result_tests(passed_res, total_res, chrono_res)
