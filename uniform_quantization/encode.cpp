#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <queue>

bool read_from_file_instansly(std::ifstream *file, unsigned char *symbol) {
	char temp;
	if ((*file).get(temp)) {
		symbol[0] = temp;
		
		return true;
		
	} else {
		return false;
		
	}
}

int main(int argc, char* argv[]) {
	std::ifstream in;
	in.open(argv[2], std::ios::in | std::ios::binary);
	
	// read pgm
	char temp[128];
	
	int width, height;
	int i=0;
	do {
		(*in).getline(temp, 128);
		if (i==1) {
			if (strstr(temp, " ") != NULL) {
				sscanf(temp, "%d %d", &width, &height);
			} else {
				sscanf(temp, "%d", &width);
				(*in).getline(temp, 128);
				sscanf(temp, "%d", &height);
			}
		}
		
		if (temp[0] == '#')
			continue;
		else
			i++;
		
	} while(i<3);
	
	int size = width * height;
	
	unsigned char symbol[1];
	
	unsigned char buffer[size];
	
	for (int iter=0; iter<size; iter++) {
		read_from_file_instansly(&*in, symbol);
		buffer[iter] = symbol;
	}
	
	// uniform
	
	// write to file
	
	return 0;
}
