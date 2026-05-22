#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import math # sqrt

###############################################################################
def read_frequency(filename):
    freqs = dict()
    with open(filename, 'r', encoding='utf-8') as fin:
        for line in fin.readlines():
            line = line.strip().split("\t")
            if len(line) == 2:
                freqs[line[0]] = line[1]
            if len(line) == 3:
                key = list()
                key.append(line[0])
                key.append(line[1])
                freqs[tuple(key)] = line[2]

    return freqs

###############################################################################
def calc_tscore(filename, unigrams, unigram_context, uni_N, cutoff):
    freqs = read_frequency(file_stem+".2gram")
    t_scores = dict()
    for key, value in freqs.items():
        if float(value) < cutoff:
            continue
        o = float(value)
        e0 = int(unigram_context[key[0]]) * int(unigrams[key[1]]) / int(uni_N)
        t_scores[key] = (o - e0) / math.sqrt(o)

        key1 = list()
        key1.append(key[1])
        key1.append(key[0])

        e1 = int(unigram_context[key[1]]) * int(unigrams[key[0]]) / int(uni_N)
        t_scores[tuple(key1)] = (o - e1) / math.sqrt(o)
     
    return t_scores

###############################################################################
def print_tscore(filename, t_scores):   
    with open(filename, 'w', encoding='utf-8') as fin:
        for key, value in sorted(t_scores.items()):
            if value > 0 and key[1] not in key[0]:
                print("%s\t%s\t%.3f" %(key[0], key[1], value), file = fin)
 
###############################################################################
if __name__ == "__main__":

    CUTOFF = 5 # 공기빈도가 이 값 이상인 경우만 t점수를 계산
    
    if len(sys.argv) < 2:
        print( "[Usage]", sys.argv[0], "in-file(s)", file=sys.stderr)
        sys.exit()

    for input_file in sys.argv[1:]:
        
        print( 'processing %s' %input_file, file=sys.stderr)

        file_stem = input_file
        pos = input_file.find(".")
        if pos != -1:
            file_stem = input_file[:pos] # ex) "2017.2gram" -> "2017"
    
        print("\tLoading %s.1gram" %file_stem, file=sys.stderr)
        unigrams = read_frequency(file_stem+".1gram")
        
        print("\tLoading %s.1gram_context" %file_stem, file=sys.stderr)
        unigram_context = read_frequency(file_stem+".1gram_context")
        
        uni_N = unigrams['#Total'] # unigram 빈도 합
        
        t_scores = calc_tscore(input_file, unigrams, unigram_context, uni_N, CUTOFF)
        
        print_tscore(file_stem+".tscore", t_scores)
