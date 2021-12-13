#include <dirent.h>
#include <stdio.h>

#include <set>
#include <limits>
#include <list>
#include <string>

#include "fcm.hpp"

struct lang_FCM {
  FCM *fcm;
  string lang;
};

uint check_alphabet(FILE *fptr){
  set<char> alpha;
  char c = fgetc(fptr);
  do {
    if (alpha.find(c) == alpha.end()) {
      alpha.insert(c);
    }
    c = fgetc(fptr);
  } while (c != EOF);
  rewind(fptr);
  return alpha.size();
}

double get_numbits(FCM *fcm, FILE *fptr_t, uint k, float a) {
  double bits = 0;
  char context[k];
  int l = 0;

  // first k letters
  fgets(context, k + 1, fptr_t);

  // k+1 letter
  char next_char = fgetc(fptr_t);
  do {
    l++;
    bits += fcm->letter_entropy(context, next_char, a, fcm->getSymbolSize());

    // slide one
    for (uint i = 0; i < k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[k - 1] = next_char;

    next_char = fgetc(fptr_t);

  } while (next_char != EOF);

  return (double)bits / l;
}

void locatelang(list<lang_FCM> lang_list, FILE *fptr, float a, uint k, uint buffer_size ) {
  double bits = 0;
  double min_bits = 1000;
  string current_lang;
  string last_lang;
  string lang;
  uint buffer;
  bool first_position=1;
  char context[k];
  uint char_count; 
  uint symbols_size = check_alphabet(fptr); 

  // first k letters
  fgets(context, k + 1, fptr);
  char_count = k + 1;

  // k+1 letter
  char next_char = fgetc(fptr);
  do {
    min_bits = 1000;
    for (auto v : lang_list) {
      bits = v.fcm->letter_entropy(context, next_char, a, symbols_size);
      if (bits < min_bits) {
        min_bits = bits;
        lang = v.lang;
      }
    }
    if (lang.compare(last_lang) != 0) {
      buffer = 0;
      last_lang=lang;
    }
    if (current_lang.compare(lang) != 0) {
      buffer++;
      if(buffer == buffer_size && first_position){
        printf("%s: %d\n", lang.c_str(), 0);
        current_lang = lang;
        first_position=0;
        buffer=0;
      }
      else if (buffer == buffer_size) {
        current_lang = lang;
        printf("%s: %d\n", current_lang.c_str(), char_count - buffer);
        buffer = 0;
      }
    }

    char_count++;
    // slide one
    for (uint i = 0; i < k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[k - 1] = next_char;

    next_char = fgetc(fptr);

  } while (next_char != EOF);
}
