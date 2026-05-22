#!/usr/bin/env python3
# coding: utf-8

import sys

###############################################################################
def get_morphs_tags(tagged):
    p_tag = list()
    for i in tagged:
        p_tag.append(i)

    for i in range(len(p_tag) - 2):
        if p_tag[i] == '+':
            p_tag[i] = ''
            if p_tag[i + 1] == '/' and p_tag[i + 2] != '/':
                p_tag[i] = '+'
            if p_tag[i + 1] == '/' and p_tag[i + 2] == '/':
                p_tag[i] = ''

        if p_tag[i] == '/':
            p_tag[i] = ''
            if p_tag[i + 1] == '/':
                p_tag[i] = '/'
            if p_tag[i + 1] != '/':
                p_tag[i] = ''

    temp = list()
    temp_text = ''

    for i in range(len(p_tag)):
        if p_tag[i] == '':
            temp.append(temp_text)
            temp_text = ''
        if i == len(p_tag) - 1:
            temp_text += p_tag[i]
            temp.append(temp_text)

        temp_text += p_tag[i]

    f_result = list()
    for i in range(0, len(temp) - 1, 2):
        t = (temp[i], temp[i + 1])
        f_result.append(t)

    return f_result

###############################################################################
if __name__ == "__main__":

    if len(sys.argv) != 2:
        print( "[Usage]", sys.argv[0], "in-file", file=sys.stderr)
        sys.exit()

    with open(sys.argv[1]) as fin:

        for line in fin.readlines():

            # 2 column format
            segments = line.split('\t')

            if len(segments) < 2: 
                continue

            # result : list of tuples
            result = get_morphs_tags(segments[1].rstrip())
        
            for morph, tag in result:
                print(morph, tag, sep='\t')
