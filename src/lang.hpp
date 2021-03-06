#include <dirent.h>
#include <stdio.h>

#include <limits>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "fcm.hpp"

struct lang_FCM {
  FCM *fcm1;
  FCM *fcm2;
  FCM *fcm3;
  string lang;
};

struct lang_k {
  FCM *fcm;
  string lang;
};

struct lang_location {
  uint location;
  string lang;
};

bool compare_lang_location(const lang_location &a, const lang_location &b) {
  return a.location < b.location;
}

uint check_alphabet(FILE *fptr) {
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

double get_numbits(FCM *fcm, FILE *fptr_t, uint k, float a, uint symbol_size) {
  double bits = 0;
  char context[k];
  int l = 0;

  // first k letters
  fgets(context, k + 1, fptr_t);

  // k+1 letter
  char next_char = fgetc(fptr_t);
  do {
    l++;
    bits += fcm->letter_entropy(context, next_char, a, symbol_size);

    // slide one
    for (uint i = 0; i < k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[k - 1] = next_char;

    next_char = fgetc(fptr_t);

  } while (next_char != EOF);

  return (double)bits / l;
}

vector<lang_location> locatelang(const list<lang_FCM> lang_list, FILE *fptr,
                                 float a, uint buffer_size,
                                 bool skip_first_line) {
  uint max_k = 5;
  char context[max_k];

  string curr_lang, last_lang, lang;
  vector<lang_location> locations;

  uint buffer = 0;
  uint char_count = max_k;
  uint symbols_size = check_alphabet(fptr);

  if (skip_first_line) {
    char ignore[1000];
    fgets(ignore, 1000, fptr);
  }

  // first k letters
  fgets(context, max_k + 1, fptr);

  // k+1 letter
  char next_char = fgetc(fptr);
  do {
    double min_bits = 10000;
    for (auto v : lang_list) {
      double bits =
          .6 * v.fcm3->letter_entropy(context, next_char, a, symbols_size) +
          .2 * v.fcm2->letter_entropy((context + 1), next_char, a, symbols_size) +
          .2 * v.fcm1->letter_entropy((context + 2), next_char, a, symbols_size);
      if (bits < min_bits) {
        min_bits = bits;
        lang = v.lang;
      }
    }

    if (lang.compare(last_lang) != 0) {
      // reset buffer if last lang is different from this lang
      buffer = 0;
      last_lang = lang;
    }

    if (curr_lang.compare(lang) != 0) {
      buffer++;

      if (buffer == buffer_size) {
        // change current lang to the best lang found buffer_size times
        uint loc = char_count - buffer;
        if (locations.empty()) {
          // first time
          loc = 0;
        }
        printf("  %s: %d\n", lang.c_str(), loc);
        locations.push_back({loc, lang});

        curr_lang = lang;
        buffer = 0;
      }
    }
    // ignore continuation chars
    if ((next_char & 0xc0) != 0xc0) {
      char_count++;
    }

    // slide one
    for (uint i = 0; i < max_k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[max_k - 1] = next_char;

    next_char = fgetc(fptr);

  } while (next_char != EOF);

  return locations;
}

vector<lang_location> locatelang_k(const list<lang_k> lang_list, FILE *fptr,
                                   float a, uint k, uint buffer_size,
                                   bool skip_first_line) {
  char context[k];

  string curr_lang, last_lang, lang;
  vector<lang_location> locations;

  uint buffer = 0;
  uint char_count = k;
  uint symbols_size = check_alphabet(fptr);

  if (skip_first_line) {
    char ignore[1000];
    fgets(ignore, 1000, fptr);
  }

  // first k letters
  fgets(context, k + 1, fptr);

  // k+1 letter
  char next_char = fgetc(fptr);
  do {
    double min_bits = 10000;

    for (auto v : lang_list) {
      double bits = v.fcm->letter_entropy(context, next_char, a, symbols_size);
      if (bits < min_bits) {
        min_bits = bits;
        lang = v.lang;
      }
    }

    if (lang.compare(last_lang) != 0) {
      // reset buffer if last lang is different from this lang
      buffer = 0;
      last_lang = lang;
    }

    if (curr_lang.compare(lang) != 0) {
      buffer++;

      if (buffer == buffer_size) {
        // change current lang to the best lang found buffer_size times
        uint loc = char_count - buffer;
        if (locations.empty()) {
          // first time
          loc = 0;
        }
        printf("  %s: %d\n", lang.c_str(), loc);
        locations.push_back({loc, lang});

        curr_lang = lang;
        buffer = 0;
      }
    }
    // ignore continuation chars
    if ((next_char & 0xc0) != 0xc0) {
      char_count++;
    }

    // slide one
    for (uint i = 0; i < k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[k - 1] = next_char;

    next_char = fgetc(fptr);

  } while (next_char != EOF);

  return locations;
}
