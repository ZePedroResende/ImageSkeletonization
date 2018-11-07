#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>

typedef struct stat *Stat;

int **matrix, width, height, Maxval;

void skip_comments(FILE * pgmFile){
  char c = fgetc(pgmFile);
  if (c == '#') while(fgetc(pgmFile) != '\n');
  else ungetc(c, pgmFile);
}

void readPgmFile(FILE * fin, FILE * fout){
  char LINE[30];
  int i, j, r, temp;

  fprintf(fout,"%s\n", fgets(LINE, 30, fin));
  skip_comments(fin);
  r = fscanf(fin, "%d %d %d", &width, &height, &Maxval);
  fprintf(fout, "%d %d\n%d\n", width, height, Maxval);

  printf("%d x %d\nInitializing...\n", width, height);


  //int matrix[width][weigh];
  //matrix = (int *) malloc(width * height * sizeof(int));
  // array simula matrix matrix[i * col + j]

  matrix = (int **) malloc(height * sizeof(int *)); 

  printf("%d x %d\nInitializing...\n", width, height);
  for (i=0; i<height; i++) 
    matrix[i] = (int *) malloc(width * sizeof(int)); 

  printf("%d x %d\nInitializing...\n", width, height);

  for(i=0; i < height ; i++){ /* Initialize the local rep'n */
    for(j=0; j < width; j++){ /* Initialize the local rep'n */
      r = fscanf(fin, "%d ", &temp);
      matrix[i][j] = temp;
    }
  }
}

int file_exists(char *file_name, Stat *buffer){
  int result;

  if ((result = stat(file_name, *buffer))){
    printf("File %s doesn't exist!\n", file_name);
  }

  return result;
}


int output_file(char *in_path, char *out_path){
  char  *dname, *bname, *dirc, *basec;
  *out_path = NULL;
  dirc = strdup(in_path);
  basec = strdup(in_path);
  dname = dirname(dirc);
  bname = basename(basec);
  if(strcmp(dname,".")){
    strcat(out_path, dname);
    strcat(out_path, "/");
  }
  strcat(out_path, "out_");
  strcat(out_path, bname);
  
  return 1;
}

int process_files(int number_files, char *files[]){
  FILE *fin, *fout;
  for(int i = 0; i < number_files; i++){

    Stat buffer = (Stat) malloc(sizeof(struct stat));

    if(!file_exists(files[i], &buffer)){

      if ((fin = fopen(files[i], "r"))==NULL){
        fprintf(stderr, "Failed to open input");
        exit(1);
      }
      char out[strlen(files[i]) + 4]; 
      output_file(files[i], out);
      printf("file %s\n", out);

      if ((fout = fopen( out, "ab+"))==NULL){
        fprintf(stderr, "Failed to open output\n");
        exit(1);
      }

      readPgmFile(fin, fout);

      fclose(fin);
      fclose(fout);
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
