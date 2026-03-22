from string import Template
from argparse import ArgumentParser

if __name__ == "__main__":
    ap = ArgumentParser()
    ap.add_argument("in_file")
    ap.add_argument("out_file")
    ap.add_argument("dcc_start")
    ap.add_argument("dcc_mem_size")

    args = ap.parse_args()

    temp = Template(open(args.in_file, "r").read())
    open(args.out_file, "w").write(temp.substitute(DCC_LOAD_OFFSET=args.dcc_start, DCC_MEMORY_SIZE_KB=args.dcc_mem_size))