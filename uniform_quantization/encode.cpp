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

void uq_enc(unsigned char *buffer_in, std::queue<char> *code_write, int size, int bit_len) {
	int split = pow2(bit_len);
	int range = SYMBOL/split;
	int mid = range/2;
	
	for (int iter=0; iter<size; ++iter) {
		unsigned char temp = (unsigned char)(((int)buffer_in[iter] / range) + 1);
		
		temp <<= (8-bit_len);
		for (int j=0; j<bit_len; j++) {
			if ((temp & 0x80) == 0x80) {
				(*code_write).push('1');
				
			} else {
				(*code_write).push('0');
				
			}
			temp <<= 1;
		}
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
	
	if (argc != 3) {
		printf("\aUsage: %s <filename in> <bit>\n",argv[0]);
		
		return 1;
		
    } else {
//		std::cout << argv[1] << '\n' << argv[2] << '\n';
		
		std::ifstream in;
		in.open(argv[1], std::ios::in | std::ios::binary);
		
		std::ofstream out;
		char filename_out[strlen(argv[1])];
		sscanf(argv[1], "%[^.]", filename_out);
		strcat(filename_out, ".uq");
		out.open(filename_out, std::ios::out | std::ios::binary);
		
		int bit_len;
		sscanf(argv[2], "%d", &bit_len);
		
		if (bit_len >= 8 || bit_len < 1) {
			printf("\aBit must between 0-8\n",argv[0]);
		
			return 1;
			
		} else {
			char temp[128];
			
			int width, height;
			int i=0;
			do {
				in.getline(temp, 128);
				if (i==1) {
					if (strstr(temp, " ") != NULL) {
						sscanf(temp, "%d %d", &width, &height);
					} else {
						sscanf(temp, "%d", &width);
						in.getline(temp, 128);
						sscanf(temp, "%d", &height);
					}
				}
				
				if (temp[0] == '#')
					continue;
				else
					i++;
				
			} while(i<3);
			
			// initiate file with width, height, bit use & offset
			out.write((char*)&width, 4);
			out.write((char*)&height, 4);
			out << (unsigned char) 0x00; // bit use & offset
			
			int size = width * height;
			
			unsigned char symbol[1];
			unsigned char buffer_in[size];
			memset(buffer_in, 0, size);
			
			std::queue<char> code_write;
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
			uq_enc(buffer_in, &code_write, size, bit_len);
			
			/**
			 * write to file
			 */
			symbol[0] = 0x00;
			
			int offset = 0;
			
			std::cout << code_write.size() << '\n';
			
			while(code_write.size() >= 8) {
				write_to_file(&out, &code_write);
			}
			
			std::cout << code_write.size() << '\n';
			
			if (code_write.size() > 0) {
				offset = 8 - code_write.size();
				
				while (code_write.size() < 8) {
					code_write.push('0');
				}
				
				write_to_file(&out, &code_write);
			}
			
			out.seekp(8);
			
			char write_offset[1];
			write_offset[0] = (char) bit_len;
			write_offset[0] <<= 4;
			write_offset[0] ^= (char) offset;
			
			out.write(write_offset, 1);
			
			std::cout << bit_len << " " << offset << " " << write_offset[0] << '\n';
			
		}
		
	}
	
	return 0;
}
