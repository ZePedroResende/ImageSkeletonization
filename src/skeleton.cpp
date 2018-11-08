#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>

typedef struct stat *Stat;

int **matrix, width, height, Maxval;

void skip_comments(FILE * pgmFile){
  char c = fgetc(pgmFile);

  if (c == '#'){
    while(fgetc(pgmFile) != '\n');
  } else {
    ungetc(c, pgmFile);
  }
}

void readPgmFile(FILE * fin, FILE * fout){
  char LINE[30];
  int i, j, r, temp;
  fprintf(fout,"%s\n", fgets(LINE, 30, fin));
  skip_comments(fin);

  r = fscanf(fin, "%d %d %d", &width, &height, &Maxval);

  fprintf(fout, "%d %d\n%d\n", width, height, Maxval);
  printf("%d x %d\nInitializing...\n", width, height);

  /*int matrix[width][weigh];
  matrix = (int *) malloc(width * height * sizeof(int));
   array simula matrix matrix[i * col + j];
*/
  
  matrix = (int **) malloc(height * sizeof(int *)); 

  printf("%d x %d\nInitializing...\n", width, height);

  for (i=0; i<height; i++){
    matrix[i] = (int *) malloc(width * sizeof(int)); 
  }

  printf("%d x %d\nInitializing...\n", width, height);

  for(i=0; i < height ; i++){ 
    for(j=0; j < width; j++){ 
      r = fscanf(fin, "%d ", &temp);
      matrix[i][j] = temp;
    }
  }
}
int can_be_removed(int i, int j, int metodo){
  int temp[9], res = 1, iter;
  memset(temp,0,sizeof(temp));

  /*
  temp[0] = matrix[i][j];
  temp[1] = i - 1 > 0                       ? matrix[i-1][j]   : 0;
  temp[2] = i - 1 > 0      && j + 1 < width ? matrix[i-1][j+1] : 0;
  temp[3] = j + 1 < width                   ? matrix[i]  [j+1] : 0;
  temp[4] = i + 1 < height && j + 1 < width ? matrix[i+1][j+1] : 0;
  temp[5] = i + 1 < height                  ? matrix[i+1][j]   : 0;
  temp[6] = i + 1 < height && j - 1 > 0     ? matrix[i+1][j-1] : 0;
  temp[7] = j - 1 > 0                       ? matrix[i]  [j-1] : 0;
  temp[8] = i - 1 > 0      && j - 1 > 0     ? matrix[i-1][j-1] : 0;
  */
 
  int min_i, max_i, min_j, max_j;
  min_i = i-1 > 0;
  max_i = i+1 < height;
  min_j = j-1 > 0;
  max_j = j+1 < width;

  temp[0] = matrix[i][j];

  if(min_i){
    temp[1] = matrix[i-1][j];

    if(min_j){
      temp[8] = matrix[i-1][j-1];
    }

    if(max_j){
      temp[2] = matrix[i-1][j+1];
    }
  }
  if(max_i){
    temp[5] = matrix[i+1][j];

    if(min_j){
      temp[6] = matrix[i+1][j-1];
    }
    if(max_j){
      temp[4] = matrix[i+1][j+1];
    }
  }

  if(min_j){
    temp[7] = matrix[i][j-1];
  }

  if(max_j){
    temp[3] = matrix[i][j+1];
  }
  
  int total = 0;
  for(res = 1; res < 9; res++){
    total += (temp[res] > 0);
  }
  if(2 <= total && total <= 6){
    return 0;
  }
  int trans = 0;
  for(res = 2; res < 8 && trans <2 ; res++){
    if(temp[res-1] != temp[res]){
      trans++;
    }
  }
  if(trans> 1){
    return 0;
  }

  return   metodo & 1 ? temp[3] == 0 && temp[5] == 0 && ( temp[1] == 0 || temp[7] == 0 ) : temp[1] == 0 && temp[7] == 0 && ( temp[3] == 0 || temp[5] == 0 );
}

int process_file(){
  int alt, i, j, temp, flag;
  do{
    flag = 0;

    for(alt=0; alt < 2; alt++){
      for(i =0; i < height; i++){
        for(j =0; j < width; j++){
          if( can_be_removed(i,j,alt)){
            matrix[i][j] = 0;
            flag = 1;
          }      
        } 
      }
    }

  }while(flag);
  return 0;
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
  
  return 0;
}

int process_files(int number_files, char *files[]){
  FILE *fin, *fout;

  Stat buffer = (Stat) malloc(sizeof(struct stat));

  for(int i = 0; i < number_files; i++){

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

  }

  free(buffer);

  return 0;
}

int main(int argc, char *argv[]){

  if(argc < 2){
    printf("No input files !\n ./skeleton_seq [IMAGE] \n");
  }else{
    process_files(argc-1, &argv[1]);
  }

  printf("That's all folks !\n");
  return 0;
}
