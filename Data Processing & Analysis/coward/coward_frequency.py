#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from collections import defaultdict
from itertools import combinations

###############################################################################
def get_word_freq(filename):
    word_freq = defaultdict(int)
    total_unigram_count = 0
    with open(filename, 'r', encoding='utf-8') as fin:
        for line in fin.readlines():
            line = line.strip()
            word = line.split()
            word = set(word)
            for w in word:
                word_freq[w] += 1

    for key, value in word_freq.items():
        total_unigram_count += value

    return word_freq, total_unigram_count

###############################################################################
def print_word_freq(filename, word_freq):
    with open(filename, 'w', encoding='utf-8') as fin:
        for key, value in sorted(word_freq.items()):
            print("%s\t%d" %(key, value), file = fin)

###############################################################################
def get_coword_freq(filename):
    coword_freq = defaultdict(int)
    word_context_size = defaultdict(int)
    with open(filename, 'r', encoding = 'utf-8') as fin:
        word_freq, total_unigram_count = get_word_freq(filename)

        for line in fin.readlines():
            line = line.strip()
            word = line.split()
            word = set(word)
            for w1, w2 in combinations(word, 2):
                key = list()
                if w1 < w2:
                    key.append(w1)
                    key.append(w2)
                    key = tuple(key)
                else:
                    key.append(w2)
                    key.append(w1)
                    key = tuple(key)
                coword_freq[key] += 1

            for key, value in word_freq.items():
                if key in word:
                    word_context_size[key] += len(word)

        word_freq["#Total"] = total_unigram_count
                
    return word_freq, coword_freq, word_context_size

###############################################################################
def print_coword_freq(filename, coword_freq):
    with open(filename, 'w', encoding='utf-8') as fin:
        for key, value in sorted(coword_freq.items()):
            print("%s\t%s\t%d" %(key[0], key[1], value), file = fin)


###############################################################################
if __name__ == "__main__":

    if len(sys.argv) < 2:
        print( "[Usage]", sys.argv[0], "in-file(s)", file=sys.stderr)
        sys.exit()

    for input_file in sys.argv[1:]:
        
        print( 'processing %s' %input_file, file=sys.stderr)
        
        file_stem = input_file
        pos = input_file.find(".")
        if pos != -1:
            file_stem = input_file[:pos] # ex) "2017.tag.context" -> "2017"
        
        # 1gram, 2gram, 1gram context 빈도를 알아냄
        word_freq, coword_freq, word_context_size = get_coword_freq(input_file)

        # unigram 출력
        print_word_freq(file_stem+".1gram", word_freq)
        
        # bigram(co-word) 출력
        print_coword_freq(file_stem+".2gram", coword_freq)

        # unigram context 출력
        print_word_freq(file_stem+".1gram_context", word_context_size)
