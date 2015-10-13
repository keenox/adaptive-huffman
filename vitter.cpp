/**
 * Adaptive Huffman
 *
 * Author: Djuned Fernando Djusdek
 *         5112.100.071
 *         Informatics - ITS
 */

#include <iostream>
#include <algorithm>
#include <vector>
#include <cstring>
#include <fstream>
#include <queue>
#include <bitset>

#define SYMBOL 256
#define NUMBER 512

typedef struct node{
	unsigned char symbol; // symbol, default string kosong
	int weight,           // bobot
	    number;           // nomor urut
	node *parent,         // orang tua
	     *left,           // anak kiri
	     *right;          // anak kanan
} node;

/**
 * UPDATE
 */

typedef std::pair<int, node*> my_pair;

void create_node(node **leaf, unsigned char symbol, bool is_nyt) {
	node *temp =  (node*) malloc(sizeof(node));
	if (is_nyt) {
		temp->symbol = 0x00;
		temp->weight = 0;
	} else {
		temp->symbol = symbol;
		temp->weight = 1;	
	}
	temp->parent = NULL;
	temp->left = NULL;
	temp->right = NULL;
		
	*leaf = temp;
	
	return;
}

void insert_node(node **root, node *leaf) {
	if (*root == NULL) {
		*root = leaf;
		return;
	
	} else {
		if ((*root)->weight == 0) {
			leaf->parent = (*root)->parent;
			(*root)->parent->left = leaf;
			return;
		}
		
		if ((*root)->left != NULL) {
			insert_node(&(*root)->left, leaf);
		}
		
		if ((*root)->right != NULL) {
			insert_node(&(*root)->right, leaf);
		}
	}
}

void merge_node(node **tree, node *left, node *right) {
	node *temp = (node*) malloc(sizeof(node));
	temp->weight = left->weight + right->weight;
	temp->left = left;
	temp->right = right;
	temp->left->parent = temp;
	temp->right->parent = temp;
	temp->symbol = 0x00;
	temp->parent = NULL;
	*tree = temp;
	
	return;
}

void search_higher_block(node **tree, int weight, int *number, int parent_number, node **position, char *l_r) {
	if ((*tree)->weight == weight && (*tree)->number > *number && (*tree)->number != parent_number) {
		*position = (*tree)->parent;
		*number = (*tree)->number;
		if ((*tree)->parent->left->number == (*tree)->number) {
			strcpy(l_r, "left");
		} else {
			strcpy(l_r, "right");
		}
	}
	
	if ((*tree)->left != NULL)
		search_higher_block(&(*tree)->left, weight, &*number, parent_number, &*position, l_r);
	
	if ((*tree)->right != NULL)
		search_higher_block(&(*tree)->right, weight, &*number, parent_number, &*position, l_r);
		
	return;
}

void switch_node(node *tree, char *l_r, node *sibling, char *l_r_sibling) {	
	if (strcmp(l_r, "left") == 0 && strcmp(l_r_sibling, "left") == 0) {
		node *temp = tree->left;
		tree->left = sibling->left;
		sibling->left = temp;
		
		tree->left->parent = tree;
		sibling->left->parent = sibling;
		
	} else if (strcmp(l_r, "left") == 0) {
		node *temp = tree->left;
		tree->left = sibling->right;
		sibling->right = temp;
		
		tree->left->parent = tree;
		sibling->right->parent = sibling;
		
	} else if (strcmp(l_r_sibling, "left") == 0) {
		node *temp = tree->right;
		tree->right = sibling->left;
		sibling->left = temp;
		
		tree->right->parent = tree;
		sibling->left->parent = sibling;
		
	} else {
		node *temp = tree->right;
		tree->right = sibling->right;
		sibling->right = temp;
		
		tree->right->parent = tree;
		sibling->right->parent = sibling;
		
	}

	return;
}

void queueing_node(node **tree, std::vector<my_pair> *queue, int deep) {
	(*queue).push_back(std::make_pair(deep, *tree));
	
	if ((*tree)->right != NULL) {
		queueing_node(&(*tree)->right, &*queue, deep+1);
	}
	
	if ((*tree)->left != NULL) {
		queueing_node(&(*tree)->left, &*queue, deep+1);
	}
	
	return;
}

void increment_weight(node **tree) {
	if ((*tree)->left != NULL && (*tree)->right != NULL)
		(*tree)->weight = (*tree)->left->weight + (*tree)->right->weight;
	else
		(*tree)->weight++;
	
	return;
}

void find_external_symbol(node **tree, unsigned char symbol, node **position) {
	if ((*tree)->left != NULL)
		find_external_symbol(&(*tree)->left, symbol, &*position);
		
	if ((*tree)->symbol == symbol) {
		*position = *tree;
	}
	
	if ((*tree)->right != NULL)
		find_external_symbol(&(*tree)->right, symbol, &*position);
		
	return;
}

void print_tree(node **tree, int deep) {
	if((*tree)->left != NULL)
		print_tree(&(*tree)->left, deep+1);
	
	if((*tree)->right != NULL)
		print_tree(&(*tree)->right, deep+1);
	
	char symbol_re[3];
	sprintf(symbol_re, "%2x", (*tree)->symbol);
	
	if (symbol_re[0] == ' ')
		symbol_re[0] = '0';
	
	printf("%d 0x%s %c %d %3d", deep, symbol_re, (*tree)->symbol, (*tree)->number, (*tree)->weight);
	
	if ((*tree)->weight == 0)
		std::cout << " << NYT";
	else if ((*tree)->left == NULL)
		printf(" << 0x%s", symbol_re);
	
	std::cout << '\n';
	
	return;
}

bool my_sort(my_pair p, my_pair q) {
	return p.first < q.first;
}

void update(node **tree, unsigned char symbol, std::vector<unsigned char> *dictionary) {
	// search in dictionary
	std::vector<unsigned char>::iterator it;
	it = std::search_n ((*dictionary).begin(), (*dictionary).end(), 1, symbol);
	
	node *temp;
	
	// NYT
	if (it != (*dictionary).end()) {
		find_external_symbol(&*tree, symbol, &temp);
		
		node *inner_temp = NULL;
		char l_r[10];
		if (temp->parent->left->number == temp->number) {
			strcpy(l_r, "left");
		} else {
			strcpy(l_r, "right");
		}
		
		char l_r_sibling[10];
		int number = temp->number;
		search_higher_block(&*tree, temp->weight, &number, temp->parent->number, &inner_temp, l_r_sibling);
		if (inner_temp != NULL) {
			switch_node(temp->parent, l_r, inner_temp, l_r_sibling);
		}
				
	} else {
		node *new_nyt;
		create_node(&new_nyt, 0x00, true);
		node *new_node;
		create_node(&new_node, symbol, false);

		node *old_nyt = NULL;
		merge_node(&old_nyt, new_nyt, new_node);
		
		insert_node(&*tree, old_nyt);
		
		// goto old nyt
		temp = old_nyt;
		
		// give number
		std::vector<my_pair> queue;
		queueing_node(&*tree, &queue, 0);
		std::sort(queue.begin(), queue.end(), my_sort);
		
		int num = NUMBER;
		for (int i=0; i<queue.size(); i++) {
			queue.at(i).second->number = num--;
//			std::cout << queue.at(i).second->symbol << " " << queue.at(i).second->number << '\n';
		}
		
		(*dictionary).push_back(symbol);

	}
	
	// increment weight
	increment_weight(&temp);
	
	// for checking tree
//	print_tree(&*tree, 0);
//	std::cout << '\n';
	
	while(temp->parent != NULL) {
		// go to parent node
		temp = temp->parent;
		
		// if not root
		if (temp->parent != NULL)
		{
			node *inner_temp = NULL;
			
			char l_r[10];
			if (temp->parent->left->number == temp->number) {
				strcpy(l_r, "left");
			} else {
				strcpy(l_r, "right");
			}
			
			char l_r_sibling[10];
			int number = temp->number;
			search_higher_block(&*tree, temp->weight, &number, temp->parent->number, &inner_temp, l_r_sibling);
			if (inner_temp != NULL) {
				switch_node(temp->parent, l_r, inner_temp, l_r_sibling);
			}
			
		}
		increment_weight(&temp);
		
		std::vector<my_pair> queue;
		queueing_node(&*tree, &queue, 0);
		
		std::sort(queue.begin(), queue.end(), my_sort);
		
		int num = NUMBER;
		for (int i=0; i<queue.size(); i++) {
			queue.at(i).second->number = num--;
		}
		
		// for checkint tree
//		print_tree(&*tree, 0);
//		std::cout << '\n';
	}
	
	*tree = temp;
	
	return;
}

/**
 * ENCODE
 */

void get_the_code(node **tree, unsigned char symbol, char *do_code, char *code) {
	char temp[SYMBOL];
	if ((*tree)->symbol == symbol && (*tree)->left == NULL && (*tree)->right == NULL) {
		strcpy(code, do_code);
		return;
	}
	
	strcpy(temp, do_code);
	if ((*tree)->left != NULL) {
		get_the_code(&(*tree)->left, symbol, strncat(temp, "0", 1), code);
	}
	
	strcpy(temp, do_code);
	if ((*tree)->right != NULL) {
		get_the_code(&(*tree)->right, symbol, strncat(temp, "1", 1), code);
	}
		
	return;
}

void get_nyt_code(node **tree, char *do_code, char *code) {
	char temp[SYMBOL];
	if ((*tree)->weight == 0 && (*tree)->left == NULL && (*tree)->right == NULL) {
		strcpy(code, do_code);
		return;
	}
	
	strcpy(temp, do_code);
	if ((*tree)->left != NULL) {
		get_nyt_code(&(*tree)->left, strncat(temp, "0", 1), code);
	}
	
	strcpy(temp, do_code);
	if ((*tree)->right != NULL) {
		get_nyt_code(&(*tree)->right, strncat(temp, "1", 1), code);
	}
		
	return;
}

void get_standard_code(unsigned char symbol, char *code) {
	unsigned char temp;
//	for (int i=7; i>=0; i--) {
//		temp = symbol & 0x01;
//		if (temp == 0x01) {
//			code[i+1] = '1';
//		} else {
//			code[i+1] = '0';
//		}
//		symbol = symbol >> 1;
//	}
//	code[0] = '0';
	
	for (int i=7; i>=0; i--) {
		temp = symbol & 0x01;
		if (temp == 0x01) {
			code[i] = '1';
		} else {
			code[i] = '0';
		}
		symbol = symbol >> 1;
	}
	
	return;
}

void write_to_file(std::ofstream *file, char *byte) {
	unsigned char temp;
	temp = temp & 0x00;
	for (int i=0; i<8; i++) {
		if (byte[i] == '1') {
			temp = temp ^ 0x01;
		}
		
		if (i != 7) {
			temp = temp << 1;
		}
	}
	*file << temp;
}

void encode(node **tree, unsigned char symbol, std::vector<unsigned char> *dictionary, std::queue<char> *code_write, std::ofstream *file) {
	// search in dictionary
	std::vector<unsigned char>::iterator it;
	it = std::search_n ((*dictionary).begin(), (*dictionary).end(), 1, symbol);
	
	char all_codes[SYMBOL / 8] = "";
	
	// symbol exist
	if (it != (*dictionary).end()) {
		char do_code[1] = "";
		char code[SYMBOL / 8] = "";
		
		get_the_code(&*tree, symbol, do_code, code);
		
		strcat(all_codes, "(");
		strcat(all_codes, code);
		strcat(all_codes, ")");
		
		for (int i=0; i<strlen(code); i++) {
			(*code_write).push(code[i]);
		}
		
	} else {
		char do_code[1] = "";
		char nyt_code[SYMBOL / 8] = "";
		
		if (*tree != NULL)
			get_nyt_code(&*tree, do_code, nyt_code);
		
		char code[10];
//		code[9] = '\0';
		code[8] = '\0';
		
		get_standard_code(symbol, code);
		
		strcat(all_codes, "(");
		if(strlen(nyt_code) > 0) {
			strcat(all_codes, nyt_code);
			strcat(all_codes, "-");
		}
		
		strcat(all_codes, code);
		strcat(all_codes, ")");
		
		for (int i=0; i<strlen(nyt_code); i++) {
			(*code_write).push(nyt_code[i]);
		}
		
		for (int i=0; i<strlen(code); i++) {
			(*code_write).push(code[i]);
		}
		
	}
	
	// call update procedure
	update(&*tree, symbol, &*dictionary);
	
	// write to file
	while ((*code_write).size() >= 8) {
		char code_to_write[8];
		for (int i=0; i<8; i++) {
			code_to_write[i] = (*code_write).front();
			(*code_write).pop();
		}
		write_to_file(&*file, code_to_write);
	}
	
	std::cout << all_codes;
}

/**
 * DECODE
 */

bool read_from_file(std::ifstream *file, std::queue<char> *code_read) {
	char temp;
	
	temp = temp & 0x00;
	
	char inner_temp;
	
	if ((*file).get(temp)) {
		unsigned char inner_temp = (unsigned char) temp;
		
		for (int i=0; i<8; i++) {
			if ((inner_temp & 0x80) == 0x80) {
//				std::cout << '1';
				(*code_read).push('1');
			}
			else {
//				std::cout << '0';
				(*code_read).push('0');
			}
			inner_temp = inner_temp << 1;
		}
		return true;
	}
	else {
		return false;
	}
}

void get_char_from_code(std::queue<char> *code_read, unsigned char *character) {
	unsigned char temp;
	temp = temp & 0x00;
	for (int i=0; i<8; i++) {
		if ((*code_read).front() == '1') {
			temp = temp ^ 0x01;
		}
		
		if (i != 7) {
			temp = temp << 1;
		}
		(*code_read).pop();
	}
	character[0] = temp;
}

void write_to_file_instansly(std::ofstream *file, unsigned char symbol) {
	*file << symbol;
}

void decode(node **tree, std::vector<unsigned char> *dictionary, std::queue<char> *code_read, std::ifstream *file, std::ofstream *out_file) {
	
	bool oke = true;
	
	// 4 byte
	while ((*code_read).size() < 32 && oke) {
		oke = read_from_file(&*file, &*code_read);
	}
	
	node *temp = *tree;
	
	if (temp == NULL) {
		unsigned char symbol[1];
		get_char_from_code(&*code_read, symbol);
		write_to_file_instansly(&*out_file, symbol[0]);
//		printf("%2x", symbol[0]);
		
		// call update procedure
		update(&*tree, symbol[0], &*dictionary);
		temp = *tree;
		
	} else {
		while (temp->left != NULL && temp->right != NULL && (*code_read).size() > 0) {
			if ((*code_read).front() == '0') {
				temp = temp->left;
				(*code_read).pop();
				
			} else {
				temp = temp->right;
				(*code_read).pop();
				
			}
			
			// 4 byte
			while ((*code_read).size() < 32 && oke) {
				oke = read_from_file(&*file, &*code_read);
			}
			
		}
		
		if ((*code_read).size() == 0) {
			return;
		}
		
		if (temp->weight == 0) {
			unsigned char symbol[1];
			get_char_from_code(&*code_read, symbol);
			write_to_file_instansly(&*out_file, symbol[0]);
			
			// call update procedure
			update(&*tree, symbol[0], &*dictionary);
			temp = *tree;
			
		} else {
			write_to_file_instansly(&*out_file, temp->symbol);
			
			// call update procedure
			update(&*tree, temp->symbol, &*dictionary);
			temp = *tree;
			
		}
		
	}
	
}

bool read_from_file_instansly(std::ifstream *file, unsigned char *symbol) {
	char temp;
	if ((*file).get(temp)) {
		symbol[0] = temp;
		
		return true;
	}
	else {
		return false;
	}
	
}

int main() {
	// ENCODE
	
	node *root;
	root = NULL;

	std::vector<unsigned char> dictionary;
	std::queue<char> code_write;
	
	std::ofstream file;
	file.open("temp.ah", std::ios::out | std::ios::binary);
	
	std::cout << "m = 2^e + r, m = 256 -> e = 8, r = 0" << '\n' << '\n';
	
	std::cout << "string representation" << '\n' << '\n';
	
	std::ifstream file_in;
	
	file_in.open("awal.txt", std::ios::in | std::ios::binary);
	
	unsigned char symbol[1];
	
	while (read_from_file_instansly(&file_in, symbol)) {
		encode(&root, symbol[0], &dictionary, &code_write, &file);
	}
	
	file_in.close();
	
	std::cout << '\n' << '\n';
	
//	std::cout << "huffman tree" << '\n' << '\n'; 
//	print_tree(&root, 0);
//	std::cout << '\n';
	
	int offset = code_write.size();
	std::cout << "sisa " << offset << " bit" << '\n';
	
	// write to file for offset
	if (code_write.size() > 0) {
		while (code_write.size() < 8) {
			code_write.push('0');
		}
		
		char code_to_write[8];
		for (int i=0; i<8; i++) {
			if (i == offset)
				std::cout << "-";
			std::cout << code_write.front();
			code_to_write[i] = code_write.front();
			code_write.pop();
			
		}
			
		write_to_file(&file, code_to_write);
	}
	
	file.close();
	
	// DECODE
	
	file.open("restore.txt", std::ios::out | std::ios::binary);
	
	file_in.open("temp.ah", std::ios::in | std::ios::binary);
	
	dictionary.clear();
	
	root = NULL;
	
	std::queue<char> code_read;
	
	do {
		decode(&root, &dictionary, &code_read, &file_in, &file);
	} while (code_read.size() > 0);
	
	file_in.close();
	
	file.close();

	return 0;
}
