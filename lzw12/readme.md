Compresi Program
gcc -w ../lib/bitio.c ../lib/errhand.c ../lib/main-c.c lzw12.c -o lzw12-c


Decompresi Program
gcc -w ../lib/bitio.c ../lib/errhand.c ../lib/main-e.c lzw12.c -o lzw12-c

running Compresi Program
./lzw12-c [dataset/input_file] [output/output_file]

running Decompresi Program
./lzw12-c [output/output_file] [hasil_decode/nama_file]