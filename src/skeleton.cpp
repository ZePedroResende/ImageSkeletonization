#include <cstdio>
#include <stdlib.h>
#include <sys/stat.h>
typedef struct stat *Stat;

int file_exists(char *file_name, Stat *buffer){
  int result;

  if ((result = stat(file_name, *buffer))){
    printf("File %s doesn't exist!\n", file_name);
  }

  return result;
}


int process_files(int number_files, char *files[]){

  for(int i = 0; i < number_files; i++){

    Stat buffer = (Stat) malloc(sizeof(struct stat));

    if(!file_exists(files[i], &buffer)){
      printf("Existe !\n");
    }

    free(buffer);

  }
  return 0;
}



int main(int argc, char *argv[]){
  if(argc < 2){
    printf("No input files !\n ./skeleton_seq [IMAGE] \n");
  }else{
    process_files(argc-1, &argv[1]);
  }

  //readPgmFile();

  printf("That's all folks !\n");
  return 0;
}
