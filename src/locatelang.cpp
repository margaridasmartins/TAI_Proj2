#include <dirent.h>

#include <list>

#include "fcm.hpp"
#include "lang.hpp"

int main(int argc, char *argv[]) {
  string help_text =
      "Usage:\n"
      "  ./locatelang models_dir filename_t context_size alpha \n"
      "Required:\n"
      "  models_dir       The name of the directory with the language files\n"
      "  filename_t       The name of the file with the text under analysis\n"
      "  context_size   The size of the context which translates into the "
      "order of the model\n"
      "  alpha          The value for the smoothing parameter\n"
      "Example:\n"
      "  ./locatelang ?? ?? 2 0.5\n";

  if (argc < 5) {
    printf("ERR: Incorrect number of arguments\n\n%s", help_text.c_str());
    exit(1);
  }

  uint k;
  float a;
  char models_dir[100];
  char filename_t[100];
  sprintf(models_dir, "%s", argv[1]);
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
  dp = opendir(models_dir);
  if (dp == NULL) {
    perror("opendir: Path does not exist or could not be read.");
    return -1;
  }
  double min_entropy = std::numeric_limits<double>::infinity();
  string lang;
  string s;
  list<lang_FCM> lang_fcm;

  while ((entry = readdir(dp))) {
    if (entry->d_name[0] != '.') {
      FILE *fptr;
      s = entry->d_name;
      char filename[100];
      sprintf(filename, "%s/%s",models_dir, s.c_str());

      if ((fptr = fopen(filename, "r")) == NULL) {
        printf("ERR: File \"%s\" not found\n", filename);
        exit(2);
      }
      FCM *fcm = new FCM(k);
      fcm->train(fptr, a);
      lang_fcm.push_back({fcm, s});
      fclose(fptr);
    }
  }
  locatelang(lang_fcm, fptr_t, a, k);
  closedir(dp);
  fclose(fptr_t);

  return 0;
}