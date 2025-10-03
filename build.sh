mkdir -p out
gcc index.c parser/parser.c -o out/index
./out/index
rm -rf out