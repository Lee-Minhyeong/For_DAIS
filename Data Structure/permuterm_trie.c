#include <stdio.h>
#include <stdlib.h>	// malloc
#include <string.h>	// strdup
#include <ctype.h>	// isupper, tolower

#define MAX_DEGREE	27 // 'a' ~ 'z' and EOW
#define EOW			'$' // end of word

// used in the following functions: trieInsert, trieSearch, triePrefixList
#define getIndex(x)		(((x) == EOW) ? MAX_DEGREE-1 : ((x) - 'a'))

// TRIE type definition
typedef struct trieNode {
	int 			index; // -1 (non-word), 0, 1, 2, ...
	struct trieNode	*subtrees[MAX_DEGREE];
} TRIE;

////////////////////////////////////////////////////////////////////////////////
// Prototype declarations

/* Allocates dynamic memory for a trie node and returns its address to caller
	return	node pointer
			NULL if overflow
*/
TRIE *trieCreateNode(void);

/* Deletes all data in trie and recycles memory
*/
void trieDestroy( TRIE *root);

/* Inserts new entry into the trie
	return	1 success
			0 failure
*/
// 주의! 엔트리를 중복 삽입하지 않도록 체크해야 함
// 대소문자를 소문자로 통일하여 삽입
// 영문자와 EOW 외 문자를 포함하는 문자열은 삽입하지 않음
int trieInsert( TRIE *root, char *str, int dic_index);

/* Retrieve trie for the requested key
	return	index in dictionary (trie) if key found
			-1 key not found
*/
int trieSearch( TRIE *root, char *str);

/* prints all entries in trie using preorder traversal
*/
void trieList(TRIE* root, char* dic[]);

/* prints all entries starting with str (as prefix) in trie
	ex) "abb" -> "abbas", "abbasid", "abbess", ...
	this function uses trieList function
*/
void triePrefixList( TRIE *root, char *str, char *dic[]);

/* makes permuterms for given str
	ex) "abc" -> "abc$", "bc$a", "c$ab", "$abc"
	return	number of permuterms
*/
int make_permuterms( char *str, char *permuterms[]);

/* recycles memory for permuterms
*/
void clear_permuterms( char *permuterms[], int size);

/* wildcard search
	ex) "ab*", "*ab", "a*b", "*ab*"
	this function uses triePrefixList function
*/
void trieSearchWildcard( TRIE *root, char *str, char *dic[]);

////////////////////////////////////////////////////////////////////////////////
TRIE* trieCreateNode(void) {
	TRIE* nTrie = (TRIE*)malloc(sizeof(TRIE));
	if (nTrie == NULL)
		return NULL;
	nTrie->index = -1;
	for (int i = 0; i < MAX_DEGREE; i++)
		nTrie->subtrees[i] = NULL;
	return nTrie;
}

void trieDestroy(TRIE* root) {
	if (root != NULL) {
		for (int i = 0; i < 27; i++)
			trieDestroy(root->subtrees[i]);
		free(root);
	}
}

int trieInsert(TRIE* root, char* str, int dic_index) {
	TRIE* cur = root;
	for (int i = 0; i < strlen(str); i++)
		if (getIndex(str[i]) > MAX_DEGREE)
			return 0;
	for (int i = 0; i < strlen(str); i++) {
		if (cur->subtrees[getIndex(str[i])] == NULL)
			cur->subtrees[getIndex(str[i])] = trieCreateNode();
		if (i != strlen(str) - 1)
			cur = cur->subtrees[getIndex(str[i])];
	}
	cur->index = dic_index;
	return 1;
}

int trieSearch(TRIE* root, char* str) {
	TRIE* cur = root;
	for (int i = 0; i < strlen(str); i++) {
		if (cur->subtrees[getIndex(str[i])] == NULL)
			return -1;
		cur = cur->subtrees[getIndex(str[i])];
	}
	return cur->index;
}

void trieList(TRIE* root, char* dic[]) {
	if (root != NULL) {
		if (root->index != -1)
			printf("%s\n", dic[root->index]);
		for (int i = 0; i < 27; i++)
				trieList(root->subtrees[i], dic);
	}
}

void triePrefixList(TRIE* root, char* str, char* dic[]) {
	TRIE* cur = root;
	int cnt = 0;
	for (int i = 0; i < strlen(str) - 1; i++) {
		if (cur->subtrees[getIndex(str[i])] == NULL)
			break;
		cur = cur->subtrees[getIndex(str[i])];
		cnt++;
	}
	if (cnt == strlen(str) - 1)
		trieList(cur, dic);
}

int make_permuterms(char* str, char* permuterms[]) {
	char tp, * temp = (char*)malloc(sizeof(char) * (strlen(str) + 2));
	int j = 0;
	for (int i = 0; i < strlen(str)+2; i++) {
		temp[i] = str[i];
		if (i >= strlen(str))
			temp[i] = 0;
	}
	temp[strlen(str)] = EOW;
	for (int i = 0; i < strlen(temp); i++) {
		permuterms[j] = strdup(temp);
		j++;
		for (int p = 1; p < strlen(temp); p++) {
			tp = temp[p-1];
			temp[p-1] = temp[p];
			temp[p] = tp;
		}
	}
	free(temp);
	return j;
}

void clear_permuterms(char* permuterms[], int size) {
	for (int i = 0; i < size; i++)
		free(permuterms[i]);
}

void trieSearchWildcard(TRIE* root, char* str, char* dic[]) {
	char tp, * temp = (char*)malloc(sizeof(char) * (strlen(str) + 2));
	for (int i = 0; i < strlen(str) + 2; i++) {
		temp[i] = str[i];
		if (i >= strlen(str))
			temp[i] = 0;
	}
	if (temp[0] == '*' && temp[strlen(str) - 1] == '*') {
		for (int p = 1; p < strlen(str); p++)
			temp[p - 1] = str[p];
		temp[strlen(str) - 1] = 0;
	}
	else {
		temp[strlen(str) + 1] = 0;
		temp[strlen(str)] = EOW;
		for (int i = 0; i <= strlen(temp); i++) {
			if (temp[strlen(temp) - 1] == '*')
				break;
			for (int p = 0; p < strlen(temp) - 1; p++) {
				tp = temp[p];
				temp[p] = temp[p + 1];
				temp[p + 1] = tp;
			}
		}
	}
	triePrefixList(root, temp, dic);
	free(temp);
}

int main(int argc, char **argv)
{
	TRIE *permute_trie;
	char *dic[100000];

	int ret;
	char str[100];
	FILE *fp;
	char *permuterms[100];
	int num_p; // # of permuterms
	int word_index = 0;
	
	if (argc != 2)
	{
		fprintf( stderr, "Usage: %s FILE\n", argv[0]);
		return 1;
	}
	
	fp = fopen( argv[1], "rt");
	if (fp == NULL)
	{
		fprintf( stderr, "File open error: %s\n", argv[1]);
		return 1;
	}
	
	permute_trie = trieCreateNode(); // trie for permuterm index
	
	while (fscanf( fp, "%s", str) != EOF)
	{
		num_p = make_permuterms(str, permuterms);

		for (int i = 0; i < num_p; i++)
			trieInsert(permute_trie, permuterms[i], word_index);

		clear_permuterms(permuterms, num_p);

		dic[word_index++] = strdup(str);
	}
	
	fclose( fp);
	
	printf( "\nQuery: ");
	while (fscanf( stdin, "%s", str) != EOF)
	{
		// wildcard search term
		if (strchr( str, '*')) 
		{
			trieSearchWildcard( permute_trie, str, dic);
		}
		// keyword search
		else 
		{
			ret = trieSearch( permute_trie, str);
			
			if (ret == -1) printf( "[%s] not found!\n", str);
			else printf( "[%s] found!\n", dic[ret]);
		}
		printf( "\nQuery: ");
	}

	for (int i = 0; i < word_index; i++)
		free( dic[i]);
	
	trieDestroy( permute_trie);
	
	return 0;
}
