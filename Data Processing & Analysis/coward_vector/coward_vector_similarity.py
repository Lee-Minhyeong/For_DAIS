#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import pickle
import math # sqrt

###############################################################################
def cosine_similarity(t_vector, c_vector):
    sum_t = 0
    sum_c = 0
    sum_mul = 0

    for key, value in t_vector.items():
        sum_t += value * value
        if key in c_vector:
            sum_mul += value * c_vector[key]
    for key, value in c_vector.items():
        sum_c += value * value

    cosine_value = sum_mul / (math.sqrt(sum_t) * math.sqrt(sum_c))

    return cosine_value

###############################################################################
def most_similar_words(word_vectors, target, topN=10):

    result = {}

    if target not in word_vectors:
        return result

    first_coward = word_vectors[target]
    for f_key, f_value in first_coward.items():
        if f_key == target or f_key not in target:
            continue

        cosine_value = cosine_similarity(first_coward, word_vectors[f_key])
        if cosine_value > 0.001:
            result[f_key] = cosine_value

        second_coward = word_vectors[f_key]
        for sc_key, sc_value in second_coward.items():
            if sc_key == target or sc_key not in target:
                continue

            cosine_value = cosine_similarity(first_coward, word_vectors[sc_key])
            if cosine_value > 0.001:
                result[sc_key] = cosine_value

    return sorted(result.items(), key=lambda x: x[1], reverse=True)[:topN]

###############################################################################
def print_words(words):
    for word, score in words:
        print("%s\t%.3f" %(word, score))

###############################################################################
def search_most_similar_words(word_vectors, topN=10):

    print('\n검색할 단어를 입력하세요(type "^D" to exit): ', file=sys.stderr)
    query = sys.stdin.readline().rstrip()

    while query:
        # result : list of tuples, sorted by cosine similarity
        result = most_similar_words(word_vectors, query, topN)
        
        if result:
            print_words(result)
        else:
            print('\n결과가 없습니다.')

        print('\n검색할 단어를 입력하세요(type "^D" to exit): ', file=sys.stderr)
        query = sys.stdin.readline().rstrip()
    
###############################################################################
if __name__ == "__main__":

    if len(sys.argv) != 2:
        print( "[Usage]", sys.argv[0], "in-file(pickle)", file=sys.stderr)
        sys.exit()

    topN = 30
    
    with open(sys.argv[1],"rb") as fin:
        word_vectors = pickle.load(fin)
    
    search_most_similar_words(word_vectors, topN)
