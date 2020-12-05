#! /usr/bin/python3

import sys, argparse, re, datetime
from collections import namedtuple

# Override Unicode tables for certain control chars
# that are expected to be found in normal text files.
force_space = {
    0x08, # backspace
    0x09, # tab
    0x0a, # newline
    0x0c, # form feed
    0x0d  # carriage return
}

# Hangul Jamo medial vowels and final consonants should be zero width.
force_compose = set()
force_compose.update(range(0x1160, 0x11ff +1))
force_compose.update(range(0xd7b0, 0xd7c6 +1))
force_compose.update(range(0xd7cb, 0xd7fb +1))

ucharInfo = namedtuple('ucharInfo', ['codepoint', 'type'])

def print_uni_file(datafile):
    ucharacters = []

    for line in datafile:
        line = re.sub('#.*', '', line.strip())
        fields = line.split(';')
        if len(fields)==0:
            continue

        codes = fields[0]
        lo_code = hi_code = -1
        match = re.match(r'([0-9a-f]+)\.\.([0-9a-f]+)', codes, flags=re.IGNORECASE)
        if match:
            lo_code = int(match.group(1), base=16)
            hi_code = int(match.group(2), base=16)
        else:
            lo_code = hi_code = int(codes, base=16)
        
        for c in range(lo_code, hi_code +1):
            mytype = 'Zs' if c in force_space else 'Mn' if c in force_compose else re.sub(r'\s', '', fields[TYPE_FIELD])
            ucharacters.append(ucharInfo(c, mytype))


    start_code=0; prev_code=0; runtype=''

    def start_run(ci):
        nonlocal start_code, prev_code, runtype
        start_code = prev_code = ci.codepoint
        runtype = ci.type

    print("/* Generated by", '"mkutable.py', ' '.join(sys.argv[1:]), '" on', 
        datetime.datetime.utcnow().isoformat(timespec='seconds') + "Z", "*/")

    ci_list = list(filter(lambda x: x.type in TYPE_NAMES or ALL_TYPES, ucharacters))
    start_run(ci_list[0])
    del ci_list[0]

    for ci in ci_list:
        if ci.type != runtype or ci.codepoint != prev_code + 1:
            print("\t{", f"{start_code:#07x},", f"{prev_code:#07x}", "},", f"/* {runtype} */")
            start_run(ci)
        else:
            prev_code = ci.codepoint


# exec
if __name__ == "__main__":
    parser = argparse.ArgumentParser('mkutable.py')
    parser.add_argument('-n', help='take non-matching types', action='store_true')
    parser.add_argument('-f', help='zero-based type field', type=int, default=2)
    parser.add_argument('types', nargs='+', type=str)
    parser.add_argument('datafile', metavar='UnicodeData.txt', type=argparse.FileType('r', encoding='UTF-8'))

    arguments = parser.parse_args()

    global ALL_TYPES, TYPE_FIELD, TYPE_NAMES
    ALL_TYPES = arguments.n
    TYPE_FIELD = arguments.f
    TYPE_NAMES = set(arguments.types)

    try:
        print_uni_file(arguments.datafile)
    except KeyboardInterrupt:
        pass
    except Exception as e:
        print(e, file=sys.stderr)
        sys.exit(1)
