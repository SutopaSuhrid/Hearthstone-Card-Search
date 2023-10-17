#include <stdio.h>
#include <stdlib.h>
#include "card.h"

CARD_T *read_card(FILE *cards_file, int index) {
  CARD_T *card = (CARD_T *)malloc(sizeof(CARD_T));
  fseek(cards_file, index * sizeof(CARD_T), SEEK_SET);
  fread(card, sizeof(CARD_T), 1, cards_file);
  return card;
}

int main() {
  FILE *cards_file = fopen("cards.bin", "rb");
  FILE *index_file = fopen("index.bin", "rb");

  if (!cards_file || !index_file) {
    printf("Error: Unable to open input files.\n");
    return 1;
  }

  int total_cards;
  fseek(index_file, 0, SEEK_END);
  total_cards = ftell(index_file) / sizeof(int);

  printf("Total cards: %d\n", total_cards);

  int index;
  while (1) {
    printf("Enter card index to search (negative number to quit): ");
    scanf("%d", &index);

    if (index < 0) {
      break;
    }

    if (index >= total_cards) {
      printf("Invalid index. Please try again.\n");
    } else {
      CARD_T *card = read_card(cards_file, index);
      print_card(card);
      free_card(card);
    }
  }

  fclose(cards_file);
  fclose(index_file);

  return 0;
}

