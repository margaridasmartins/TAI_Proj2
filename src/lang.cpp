#include "fcm.hpp"
#include "lang.hpp"

int main(int argc, char *argv[]){
    string help_text =
      "Usage:\n"
      "  ./lang filename_r filename_t context_size alpha \n"
      "Required:\n"
      "  filename_r       The name of the file with a text representing the class r i\n"
      "  filename_t       The name of the file with the text under analysis\n"
      "  context_size   The size of the context which translates into the order of the model\n"
      "  alpha          The value for the smoothing parameter\n"
      "Example:\n"
      "  ./lang ?? ?? 2 0.5\n";

    if (argc < 5) {
      printf("ERR: Incorrect number of arguments\n\n%s", help_text.c_str());
      exit(1);
    }
    
    uint k;
    float a;
    char filename_r[100];
    char filename_t[100];
    sprintf(filename_r, "%s", argv[1]);
    sprintf(filename_t, "%s", argv[1]);
    k = atoi(argv[3]);
    a = atof(argv[4]);

    FILE *fptr;

    if ((fptr = fopen(filename_r, "r")) == NULL) {
        printf("ERR: File \"%s\" not found\n", filename_r);
        exit(2);
    }

    FILE *fptr_t;
    if ((fptr_t = fopen(filename_t, "r")) == NULL) {
        printf("ERR: File \"%s\" not found\n", filename_t);
        exit(2);
    }

    FCM *fcm = new FCM(k);
    fcm->train(fptr, 0);
    printf("%d",get_numbits(fcm, fptr_t, k, a));
    fclose(fptr);

    return 0;
}