#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <queue>

#define SYMBOL 256

bool read_from_file_instansly(std::ifstream *file, unsigned char *symbol) {
	char temp;
	if ((*file).get(temp)) {
		symbol[0] = temp;
		
		return true;
	} else {
		return false;
	}
}

void write_to_file_instansly(std::ofstream *file, unsigned char symbol) {
	*file << symbol;
	
	return;
}

int pow2(int iter) {
	int p = 2;
	for (int i=1; i<iter; i++) {
		p <<= 1;
	}
	
	return p;
}

void uq_enc(unsigned char *buffer_in, unsigned char *buffer_out, int size, int bit_len) {
	int split = pow2(bit_len);
	int range = SYMBOL/split;
	int mid = range/2;
	
	for (int iter=0; iter<size; ++iter) {
//		buffer_out[iter] = (unsigned char)(((int)buffer_in[iter] / range) + 1);
		buffer_out[iter] = (unsigned char)((((int)buffer_in[iter] / range) + 1) * mid);
	}
	
	return;
}

int main(int argc, char* argv[]) {
	/** argv = {
	 *      0 := this program (.exe)
	 *      1 := filname in (.pgm)
	 *      2 := filname out (.pgm)
	 *      3 := bit
	 *  }
	 */
	
	std::cout << argv[1] << '\n' << argv[2] << '\n';
	
	std::ifstream in;
	in.open(argv[1], std::ios::in | std::ios::binary);
	
	std::ofstream out;
	out.open(argv[2], std::ios::out | std::ios::binary);
	
	int bit_len;
	sscanf(argv[3], "%d", &bit_len);
	
	char temp[128];
	
	int width, height;
	int i=0;
	do {
		in.getline(temp, 128);
		out << temp << '\n';
		if (i==1) {
			if (strstr(temp, " ") != NULL) {
				sscanf(temp, "%d %d", &width, &height);
			} else {
				sscanf(temp, "%d", &width);
				in.getline(temp, 128);
				out << temp << '\n';
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
	unsigned char buffer_in[size];
	unsigned char buffer_out[size];
	
	memset(buffer_in, 0, size);
	memset(buffer_out, 0, size);
	
	/**
	 * read from file
	 */
	symbol[0] = 0x00;
	
	for (int iter=0; iter<size; ++iter) {
		read_from_file_instansly(&in, symbol);
		buffer_in[iter] = symbol[0];
	}
	
	/**
	 * uniform
	 */
	if (bit_len < 8) {
		uq_enc(buffer_in, buffer_out, size, bit_len);
	}
	else {
		memcpy(buffer_out, buffer_in, size);
	}
	
	/**
	 * write to file
	 */
	symbol[0] = 0x00;
	
	for (int iter=0; iter<size; ++iter) {
		symbol[0] = buffer_out[iter];
		write_to_file_instansly(&out, symbol[0]);
	}
	
	return 0;
}
