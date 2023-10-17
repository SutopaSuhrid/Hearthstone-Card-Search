#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "card.h"

/*
 * I've left these definitions in from the
 * solution program. You don't have to
 * use them, but the `dupe_check()` function
 * unit test expects certain values to be
 * returned for certain situations!
 */
#define DUPE -1
#define NO_DUPE -2

/*
 * These are the special strings you need to
 * print in order for the text in the terminal
 * to be bold, italic or normal (end)
 */
#define BOLD "\e[1m"
#define ITALIC "\e[3m"
#define END "\e[0m"

/*
 * You will have to implement all of these functions
 * as they are specifically unit tested by Mimir
 */
int dupe_check(unsigned, char*);
char *fix_text(char*);
void free_card(CARD_T*);
CARD_T *parse_card(char*);
void print_card(CARD_T*);

/*
 * We'll make these global again, to make
 * things a bit easier
 */
CARD_T **cards = NULL;
size_t total_cards = 0;
int card_cmp(const void *a, const void *b) {
  // Cast the void pointers to CARD_T pointers
  CARD_T *card_a = *(CARD_T**) a;
  CARD_T *card_b = *(CARD_T**) b;

  // Compare the id fields
  if (card_a->id < card_b->id) {
    return -1;
  }
  else if (card_a->id > card_b->id) {
    return 1;
  }
  else {
    return 0;
  }
}
void save_cards() {
  FILE *cards_file = fopen("cards.bin", "wb");
  FILE *index_file = fopen("index.bin", "wb");

  if (!cards_file || !index_file) {
    printf("Error: Unable to open output files.\n");
    return;
  }

  for (int i = 0; i < total_cards; i++) {
    fwrite(cards[i], sizeof(CARD_T), 1, cards_file);
    fwrite(&i, sizeof(int), 1, index_file);
  }

  fclose(cards_file);
  fclose(index_file);
}
int main(int argc, char **argv) {
	// TODO: 1. Open the file
	//       2. Read lines from the file...
	//          a. for each line, `parse_card()`
    //          b. add the card to the array
	//       3. Sort the array
	//       4. Print and free the cards
	//       5. Clean up!
	
	
  if (argc != 2) {
               printf("Can't operate.\n");
               exit(1);
      	}
  
  // 1. Open the file
  FILE* fp = fopen(argv[1], "r");

  if (fp == NULL) {
    printf("Eroor!!Can't open file!\n");
    return 1;
  }

  // 2. Read lines from the file...
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int arrSize = 0;
  cards = (CARD_T **)malloc(sizeof(CARD_T *));
  int titleFlag = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
      // Remove the newline character at the end of the line
    if (line[read - 1] == '\n') {
        line[read - 1] = '\0';
    }
    
    if (titleFlag == 0) {
      titleFlag = 1;
    } else {
      CARD_T *c = parse_card(line);
      if (c != NULL) {
        if (total_cards == arrSize) {
          CARD_T **temp;
          temp = realloc(cards, (total_cards + 1) * sizeof(CARD_T *));
          if (!temp) {
            printf("Can't realloc Cards");
            return -1;
          } else {
            arrSize += 1;
            cards = temp;
          }
        }
        cards[total_cards] = c;
        total_cards += 1;
      }
    }
    memset(line, 0, strlen(line));
  }

  // 3. Sort the array
  qsort(cards, total_cards, sizeof(CARD_T*), card_cmp);
  

  // 4. Print and free the cards
  for (int i = 0; i < total_cards; i++) {
    print_card(cards[i]);
    free_card(cards[i]);
  }
    // 5. Save cards to binary files
  save_cards();

  // 6. Clean up!
  free(line);
  free(cards);
  fclose(fp);

  return 0;
}
/*
 * This function has to return 1 of 3 possible values:
 *     1. NO_DUPE (-2) if the `name` is not found
 *        in the `cards` array
 *     2. DUPE (-1) if the `name` _is_ found in
 *        the `cards` array and the `id` is greater
 *        than the found card's id (keep the lowest one)
 *     3. The last case is when the incoming card has
 *        a lower id than the one that's already in
 *        the array. When that happens, return the
 *        index of the card so it may be removed...
 */
int dupe_check(unsigned id, char *name) {
  if (total_cards == 0) {
    return NO_DUPE;
  }

  for (int i = 0; i < total_cards; i++) {
    if (cards[i]->name && strcmp(cards[i]->name, name) == 0) {
      if (cards[i]->id <= id) {
        return DUPE;
      } else {
        return i;
      }
    }
  }

  return NO_DUPE;
}

/*
 * This function has to do _five_ things:
 *     1. replace every "" with "
 *     2. replace every \n with `\n`
 *     3. replace every </b> and </i> with END
 *     4. replace every <b> with BOLD
 *     5. replace every <i> with ITALIC
 *

 * The first three are not too bad, but 4 and 5
 * are difficult because you are replacing 3 chars
 * with 4! You _must_ `realloc()` the field to
 * be able to insert an additional character else
 * there is the potential for a memory error!
 */

char *fix_text(char *text) {
    if (!text) return NULL;

    size_t len = strlen(text);
    size_t new_size = len * 3;
    char *new_text = (char *)malloc(new_size + 1);
    if (!new_text) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len;) {
        if (text[i] == '\"' && text[i + 1] == '\"') {
            new_text[j++] = text[i++];
            i++;
        } else if (text[i] == '\\' && text[i + 1] == 'n') {
            new_text[j++] = '\n';
            i += 2;
        } else if (strncmp(&text[i], "</b>", 4) == 0 || strncmp(&text[i], "</i>", 4) == 0) {
            memcpy(&new_text[j], END, strlen(END));
            j += strlen(END);
            i += 4;
        } else if (strncmp(&text[i], "<b>", 3) == 0 || strncmp(&text[i], "<i>", 3) == 0) {
            if (strncmp(&text[i], "<b>", 3) == 0) {
                memcpy(&new_text[j], BOLD, strlen(BOLD));
            } else {
                memcpy(&new_text[j], ITALIC, strlen(ITALIC));
            }
            j += strlen(BOLD);
            i += 3;
        } else {
            new_text[j++] = text[i++];
        }
    }
    new_text[j] = '\0';
    //free(text);

    return new_text;
}

/*
 * This short function simply frees both fields
 * and then the card itself
 */
void free_card(CARD_T *card) {
  free(card->name);
  free(card->text);
  free(card);
  total_cards -= 1;
}

/*
 * This is the tough one. There will be a lot of
 * logic in this function. Once you have the incoming
 * card's id and name, you should call `dupe_check()`
 * because if the card is a duplicate you have to
 * either abort parsing this one or remove the one
 * from the array so that this one can go at the end.
 *
 * To successfully parse the card text field, you
 * can either go through it (and remember not to stop
 * when you see two double-quotes "") or you can
 * parse backwards from the end of the line to locate
 * the _fifth_ comma.
 *
 * For the fields that are enum values, you have to
 * parse the field and then figure out which one of the
 * values it needs to be. Enums are just numbers!
 */
CARD_T *parse_card(char *line) {
  //line[strlen(line) - 1] = ',';
  
  int maxSize = strlen(line);
  
  int flag = 0;
  int quoteFlag = 0;
  int j = 0;
  
  char *text = (char *)malloc(maxSize * sizeof(char));
  memset(text, 0, maxSize);

  CARD_T *c = malloc(sizeof(CARD_T));
  for (int i = 0; i < maxSize; i++) {
    
    if (line[i] != ',' || quoteFlag == 1) {
      
      if (line[i] == '\"' && line[i + 1] == ',') {
        quoteFlag = 0;
      } else if (line[i] == '\"' && line[i - 1] == ',') {
        quoteFlag = 1;
      } else {
        text[j] = line[i];
        j++;
      }
    } 
    
    else {
    
      switch (flag) {
      case 0:
        c->id = (unsigned int)atoi(text);
        break;
      case 1:
        c->name = malloc(strlen(text) + 1);
        strcpy(c->name, text);
        int dupeResult = dupe_check(c->id, c->name);
        
        if (dupeResult == -1) {
          free(c->name);
          free(text);
          free(c);
          return NULL;
        } else if (dupeResult != -2) {
          free_card(cards[dupeResult]);
          for (int k = dupeResult; k < total_cards; k++) {
            CARD_T *card = cards[k + 1];
            cards[k] = card;
          }
        }

        break;
      case 2:
        c->cost = (unsigned int)atoi(text);
        break;
      case 3: {
      char *fixed_text = fix_text(text);
      c->text = fixed_text;
      break;
  }

      case 4:
        c->attack = (unsigned int)atoi(text);
        break;
      case 5:
        c->health = (unsigned int)atoi(text);
        break;
      case 6:
        for (int k = 0; k < 4; k++) {
          if (strcasecmp(type_str[k], text) == 0) {
            c->type = k;
            break;
          }
        }
        break;
      case 7:
        for (int k = 0; k < 11; k++) {
          if (strcasecmp(class_str[k], text) == 0) {
            c->class = k;
            break;
          }
        }
        break;
      case 8:

        for (int k = 0; k < 5; k++) {
          if (strcasecmp(rarity_str[k], text) == 0) {
            c->rarity = k;
           

            break;
          }
        }
        break;
      }
      flag += 1;
      memset(text, 0, maxSize);
      j = 0;
    }
  }
  free(text);
  return c;
}

/*
 * Because getting the card class centered is such
 * a chore, you can have this function for free :)
 */
void print_card(CARD_T *card) {
  printf("%-29s %2d\n", card->name, card->cost);
  unsigned length = 15 - strlen(class_str[card->class]);
  unsigned remainder = length % 2;
  unsigned margins = length / 2;
  unsigned left = 0;
  unsigned right = 0;
  if (remainder) {
    left = margins + 2;
    right = margins - 1;
  } else {
    left = margins + 1;
    right = margins - 1;
  }
  printf("%-6s %*s%s%*s %9s\n", type_str[card->type], left, "",
         class_str[card->class], right, "", rarity_str[card->rarity]);
  printf("--------------------------------\n");
  printf("%s\n", card->text);
  printf("--------------------------------\n");
  printf("%-16d%16d\n\n", card->attack, card->health);
}

