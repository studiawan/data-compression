bikin file executable
gcc -o main-c arith.c lib/main-c.c lib/errhand.c lib/bitio.c
gcc -o main-e arith.c lib/main-e.c lib/errhand.c lib/bitio.c

encoding
main-c data/alice29.txt result/alice29.txt.arith
main-c data/asyoulik.txt result/asyoulik.txt.arith
main-c data/cp.html result/cp.html.arith
main-c data/fields.c result/fields.c.arith
main-c data/grammar.lsp result/grammar.lsp.arith
main-c data/kennedy.xls result/kennedy.xls.arith
main-c data/lcet10.txt result/lcet10.txt.arith
main-c data/plrabn12.txt result/plrabn12.txt.arith
main-c data/ptt5 result/ptt5.arith
main-c data/sum result/sum.arith
main-c data/xargs.1 result/xargs.1.arith

decoding
main-e result/alice29.txt.arith resultdecoding/alice29.txt
main-e result/asyoulik.txt.arith resultdecoding/asyoulik.txt
main-e result/cp.html.arith resultdecoding/cp.html
main-e result/fields.c.arith resultdecoding/fields.c
main-e result/grammar.lsp.arith resultdecoding/grammar.lsp
main-e result/kennedy.xls.arith resultdecoding/kennedy.xls
main-e result/lcet10.txt.arith resultdecoding/lcet10.txt
main-e result/plrabn12.txt.arith resultdecoding/plrabn12.txt
main-e result/ptt5.arith resultdecoding/ptt5
main-e result/sum.arith resultdecoding/sum
main-e result/xargs.1.arith resultdecoding/xargs.1