#include <iostream>
#include <algorithm>
#include <cstring>
#include <fstream>
#include <queue>

#define SYMBOL 256

bool read_from_file(std::ifstream *file, std::queue<char> *code_read) {
	char temp;
	
	temp &= 0x00;
	
	if ((*file).get(temp)) {
		for (int i=0; i<8; i++) {
			if ((temp & 0x80) == 0x80) {
				(*code_read).push('1');
				
			} else {
				(*code_read).push('0');
				
			}
			temp <<= 1;
		}
		
		return true;
		
	} else {
		return false;
		
	}
}

bool read_from_file_instansly(std::ifstream *file, unsigned char *symbol) {
	char temp;
	if ((*file).get(temp)) {
		symbol[0] = temp;
		
		return true;
	} else {
		return false;
	}
}

void write_to_file(std::ofstream *file, std::queue<char> *code_write) {
	unsigned char temp;
	temp = temp & 0x00;
	for (int i=0; i<8; i++) {
		if ((*code_write).front() == '1') {
			temp ^= 0x01;
		}
		
		if (i != 7) {
			temp <<= 1;
		}
		
		(*code_write).pop();
	}
	*file << temp;
	
	return;
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

void uq_dec(std::queue<char> *code_read, unsigned char *buffer_out, int bit_len, int offset) {
	int split = pow2(bit_len);
	int range = SYMBOL/split;
	int mid = range/2;
	
	int iter = 0;
	
	// * mid
	while((*code_read).size() > offset) {
		unsigned char temp = 0x00;
		
		for (int j=0; j<bit_len; j++) {
			if ((*code_read).front() == '1') {
				temp ^= 0x01;	
			}
			
			if (j != bit_len) {
				temp <<= 1;
			}
			
			(*code_read).pop();
		}
		
		buffer_out[iter++] = (unsigned char)((int)temp * mid);
	}
	
	return;
}

int main(int argc, char* argv[]) {
	/** argv = {
	 *      0 := this program (.exe)
	 *      1 := filname in (.pgm)
	 *      2 := bit
	 *  }
	 */
	
	if (argc != 2) {
		printf("\aUsage: %s <filename in>\n",argv[0]);
		
		return 1;
		
	} else {
//		std::cout << argv[1] << '\n' << argv[2] << '\n';
		
		std::ifstream in;
		in.open(argv[1], std::ios::in | std::ios::binary);
		
		std::ofstream out;
		char filename_out[strlen(argv[1]) + 12];
		sscanf(argv[1], "%[^.]", filename_out);
		strcat(filename_out, "-restore.pgm");
		out.open(filename_out, std::ios::out | std::ios::binary);
		
		int bit_len;
		
		char temp[10];
		
		in.read(temp, 9);
		
		int width = *(int*)&temp[0];
		int height = *(int*)&temp[4];
		bit_len = (unsigned short) (temp[8] >> 4);
		int offset = (unsigned short) (temp[8] & 0x0F);
		
		char temp_out[128];
		
		out << "P5\n# Create By AHIwCV\n";
		sprintf(temp_out, "%d %d\n", width, height);
		out << temp_out;
		out << "255\n";
		
		int size = width * height;
		
		unsigned char buffer_out[size];
		memset(buffer_out, 0, size);
		
		std::queue<char> code_read;
		
		/**
		 * read from file
		 */
		bool oke = true;
		
		while (oke) {
			oke = read_from_file(&in, &code_read);
		}
		
		/**
		 * uniform
		 */
		uq_dec(&code_read, buffer_out, bit_len, offset);
		
		/**
		 * write to file
		 */
		unsigned char symbol[1];
		symbol[0] = 0x00;
		
		for (int iter=0; iter<size; ++iter) {
			symbol[0] = buffer_out[iter];
			write_to_file_instansly(&out, symbol[0]);
		}
	}
	
	return 0;
}
