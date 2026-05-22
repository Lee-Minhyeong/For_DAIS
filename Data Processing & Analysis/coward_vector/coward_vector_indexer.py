#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import pickle

###############################################################################
def vector_indexing(filename):
    
    word_vectors = {}
    with open(filename, 'r', encoding='utf-8') as fin:
        for line in fin.readlines():
            line = line.strip().split('\t')

            if line[0] not in word_vectors:
                word_vectors[line[0]] = {}

            word_vectors[line[0]][line[1]] = float(line[2])

    return word_vectors
    
###############################################################################
if __name__ == "__main__":

    if len(sys.argv) < 2:
        print( "[Usage] %s in-file out-file(pickle)" %sys.argv[0], file=sys.stderr)
        sys.exit()

    filename = sys.argv[1]
    print("processing %s ..." %filename, file=sys.stderr)
    
    # 공기어 벡터 저장 (dictionary of dictionary)
    word_vectors = vector_indexing(filename)

    print("# of entries = %d" %len(word_vectors), file=sys.stderr)

    with open(sys.argv[2],"wb") as fout:
        print("saving %s" %sys.argv[2], file=sys.stderr)
        pickle.dump(word_vectors, fout)
