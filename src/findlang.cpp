#include "lang.hpp"

using namespace std;

int main(int argc, char *argv[]) {
  string help_text =
      "Usage:\n"
      "  ./lang models_dir filename_t context_size alpha \n"
      "Required:\n"
      "  models_dir     The name of the directory with the language models"
      "  filename_t       The name of the file with the text under analysis\n"
      "  context_size   The size of the context which translates into the order of the model\n"
      "  alpha          The value for the smoothing parameter\n"
      "Example:\n"
      "  ./lang ?? 2 0.5\n";

  if (argc < 4) {
    printf("ERR: Incorrect number of arguments\n\n%s", help_text.c_str());
    exit(1);
  }

  uint k;
  float a;
  char filename_t[100];
  sprintf(filename_t, "%s", argv[2]);
  k = atoi(argv[3]);
  a = atof(argv[4]);

  FILE *fptr_t;
  if ((fptr_t = fopen(filename_t, "r")) == NULL) {
    printf("ERR: File \"%s\" not found\n", filename_t);
    exit(2);
  }

  struct dirent *entry;
  DIR *dp;

  dp = opendir(argv[1]);
  if (dp == NULL) {
    perror("opendir: Path does not exist or could not be read.");
    return -1;
  }

  double min_entropy = std::numeric_limits<double>::infinity();
  string lang;
  string s;

  while ((entry = readdir(dp))) {
    if (entry->d_name[0] != '.') {
      FILE *fptr;
      s = entry->d_name;
      char filename[100];
      sprintf(filename, "%s/%s", argv[1], s.c_str());

      if ((fptr = fopen(filename, "r")) == NULL) {
        printf("ERR: File \"%s\" not found\n", filename);
        exit(2);
      }
      FCM *fcm = new FCM(k);
      fcm->train(fptr, 0);
      double nbits = get_numbits(fcm, fptr_t, k, 0.001);
      if (nbits < min_entropy) {
        min_entropy = nbits;
        
        lang = s.substr(0, s.find("."));
      }
      rewind(fptr_t);
      fclose(fptr);
    }
  }
  fclose(fptr_t);
  closedir(dp);
  printf("%s: %f avg bits\n", lang.c_str(), min_entropy);

  return 0;
}