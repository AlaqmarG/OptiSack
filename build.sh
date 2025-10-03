mkdir -p out
g++ index.cpp parser/parser.cpp -o out/index
./out/index
rm -rf out