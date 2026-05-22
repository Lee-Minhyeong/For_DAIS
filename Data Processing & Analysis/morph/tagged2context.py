#!/usr/bin/env python3
# coding: utf-8

import sys
import get_morphs_tags as mf

###############################################################################
# 색인어 추출
def get_index_terms( mt_list):
    single_list = ['NNG', 'NNP', 'SH', 'SL']
    multi_list = ['NR', 'NNB', 'SN']
    f_result = list()
    cnt = 0
    temp_text = ''

    for i in range(len(mt_list) - 1):
        if mt_list[i][1] in single_list + multi_list:
            cnt += 1
            temp_text += mt_list[i][0]

        if mt_list[i][1] in ['NNG', 'NNP', 'SH']:
            f_result.append(mt_list[i][0])

        if mt_list[i][1] == 'SL':
            if i >= 1:
                if mt_list[i + 1][1] not in (single_list + multi_list) and mt_list[i - 1][1] not in (single_list + multi_list):
                    f_result.append(mt_list[i][0])
            if i == 0:
                if mt_list[i + 1][1] not in (single_list + multi_list):
                    f_result.append(mt_list[i][0])

        if mt_list[i][1] not in single_list + multi_list:
            if cnt > 1:
                f_result.append(temp_text)
                cnt = 0
                temp_text = ''
            else:
                cnt = 0
                temp_text = ''

        if i == len(mt_list) - 2:
            if mt_list[i + 1][1] in single_list + multi_list:
                if cnt == 0:
                    if mt_list[i + 1][1] in single_list:
                        f_result.append(mt_list[i + 1][0])
                else:
                    if mt_list[i + 1][1] in ['NNG', 'NNP', 'SH']:
                        f_result.append(mt_list[i + 1][0])
                    temp_text += mt_list[i + 1][0]
                    f_result.append(temp_text)
            else:
                if cnt > 1:
                    f_result.append(temp_text)

    if len(mt_list) == 1:
        if mt_list[0][1] in single_list:
            f_result.append(mt_list[0][0])

    return f_result

###############################################################################
# Converting POS tagged corpus to a context file
def tagged2context( input_file, output_file):
    try:
        fin = open( input_file, "r")
    except:
        print( "File open error: ", input_file, file=sys.stderr)
        sys.exit()

    try:
        fout = open( output_file, "w")
    except:
        print( "File open error: ", output_file, file=sys.stderr)
        sys.exit()

    for line in fin.readlines():
    
        # 빈 라인 (문장 경계)
        if line[0] == '\n':
            print("", file=fout)
            continue

        try:
            ej, tagged = line.split(sep='\t')
        except:
            print(line, file=sys.stderr)
            continue

        # 형태소, 품사 추출
        # return : list of tuples
        result = mf.get_morphs_tags(tagged.rstrip())
        
        # 색인어 추출
        # return : list
        terms = get_index_terms(result) 
        
        # 색인어 출력
        for term in terms:
            print(term, end=" ", file=fout)
        
    fin.close()
    fout.close()
    
###############################################################################
if __name__ == "__main__":

    if len(sys.argv) < 2:
        print( "[Usage]", sys.argv[0], "file(s)", file=sys.stderr)
        sys.exit()

    for input_file in sys.argv[1:]:
        output_file = input_file + ".context"
        print( 'processing %s -> %s' %(input_file, output_file), file=sys.stderr)
        
        # 형태소 분석 파일 -> 문맥 파일
        tagged2context( input_file, output_file)
