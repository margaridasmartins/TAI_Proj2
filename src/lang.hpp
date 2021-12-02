#include "fcm.hpp"

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
    bits += fcm->letter_entropy(context, next_char, a);

    // slide one
    for (uint i = 0; i < k - 1; i++) {
      context[i] = context[i + 1];
    }
    context[k - 1] = next_char;

    next_char = fgetc(fptr_t);

  } while (next_char != EOF);


  return (double)bits/l;
}