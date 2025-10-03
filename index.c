#include <stdio.h>
#include "parser/parser.h"

int main() {
    int item_count = 0;
    Item *items = read_items("data/the_sack.txt", &item_count);

    for (int i = 0; i < item_count; i++) {
        printf("{name: %s, value: %f, weight: %f}\n", items[i].name, items[i].value, items[i].weight);
    }
}