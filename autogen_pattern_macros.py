import argparse

def write_make_patterns(fp, n, i):
    if(i == 1):
        fp.write("#define MAKE_PATTERN_1(name, pattern_name, pattern, ...) static constexpr auto ptn_ ## name ## _1 = PATTERN(pattern);\n")
        write_make_patterns(fp, n, i+1)
    elif(i <= n):
        fp.write("#define MAKE_PATTERN_%s(name, pattern_name, pattern, ...) static constexpr auto ptn_ ## name ## _%s = PATTERN(pattern); CONCATENATE(MAKE_PATTERN_%s(name, __VA_ARGS__),)\n" % (i, i, i-1))
        write_make_patterns(fp, n, i+1)

def write_name_patterns(fp, n, i):
    if(i == 1):
        fp.write("#define NAME_PATTERN_1(name, pattern_name, pattern, ...) PatternWrapper{ pattern_name, ptn_ ## name ## _1 }\n")
        write_name_patterns(fp, n, i+1)
    elif(i <= n):
        fp.write("#define NAME_PATTERN_%s(name, pattern_name, pattern, ...) PatternWrapper{ pattern_name, ptn_ ## name ## _%s },CONCATENATE(NAME_PATTERN_%s(name, __VA_ARGS__),)\n" % (i, i, i-1))
        write_name_patterns(fp, n, i+1)

def write_FOR_EACH2_RSEQ_N(fp,n):
    fp.write("#define FOR_EACH2_RSEQ_N")
    write_FOR_EACH2_RSEQ_N_REC(fp,n)
    fp.write('\n')

def write_FOR_EACH2_RSEQ_N_REC(fp,n):
    if(n > 1):
        fp.write(' %s, 0,' % n)
        write_FOR_EACH2_RSEQ_N_REC(fp,n-1)
    elif(n == 1):
        fp.write(' 1')
  

def write_FOR_EACH2_ARG_N(fp, n):
    fp.write("#define FOR_EACH2_ARG_N(")
    write_FOR_EACH2_ARG_N_REC(fp,n,1)
    fp.write(" N, ...) N\n")

def write_FOR_EACH2_ARG_N_REC(fp, n, i):
    if(i <= n):
        fp.write(' __%s, __%s_,' % (i, i))
        write_FOR_EACH2_ARG_N_REC(fp,n,i+1)

def write_file(n):
    with open("patterns_macros.hpp", "w") as fp:
        fp.write("// This file has been automatically generated using autogen_pattern_macros.py\n\n")
        write_make_patterns(fp, n, 1)
        write_name_patterns(fp, n, 1)
        write_FOR_EACH2_ARG_N(fp, n)
        write_FOR_EACH2_RSEQ_N(fp, n)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--n", type=int)
    args = parser.parse_args()
    if hasattr(args, "n"):
        write_file(args.n)
        print("File written.")
    else:
        print("Invalid args. Provide the number N with the flag --n")

if __name__ == "__main__":
    main()