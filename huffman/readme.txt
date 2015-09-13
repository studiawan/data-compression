
This is readme for Huffman source code from Data Compression Book. 
Issue these commands in your terminal to compile Huffman source code:

cc -ohuff-c main-c.c huff.c ../lib/bitio.c ../lib/errhand.c
cc -ohuff-e main-e.c huff.c ../lib/bitio.c ../lib/errhand.c

To run the executable and test the compression with alice29.txt from Cantrbry dataset, simply type this line in the terminal:

./huff-c ../cantrbry/alice29.txt ../result/alice29.huff

To decompress the result, type this command:

./huff-e ../result/alice29.huff ../result/alice29.txt

To check the integrity of decompressed file and the original one, use sha256sum:

sha256sum ../result/alice29.txt ../cantrbry/alice29.txt


