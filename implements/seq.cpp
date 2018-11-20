#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>

typedef struct stat *Stat;

int ***matrix, width, height, position ;

void print_matrix(){
  for(int k = 0; k< 2; k ++){
    for(int i = 0; i<height; i++){
      for(int j =0; j< width; j++){
        printf("%d\t", matrix[k][i][j]);
      }
      printf("\n");
    }
  }

  printf("\n\n");
}

void skip_comments(FILE * pgmFile){
  char c = fgetc(pgmFile);

  if (c == '#'){
    while(fgetc(pgmFile) != '\n');
  } else {
    ungetc(c, pgmFile);
  }
}

int i1(int *temp){
  int  res,total = 0;

  for(res = 1; res < 9; res++){
    total += (temp[res] > 0);
  }

  return ((2 <= total) && (total <= 6));
}

int i2(int *temp){
  int res,trans = 0;

  trans += temp[8] != temp[1];
  for(res = 2; res < 9 && trans <3 ; res++){
    trans += temp[res-1] != temp[res];
  }

  return (trans == 2);
}

int i3(int *temp, int metodo){
  return ( metodo & 1 ? !temp[3] || !temp[5] || ( !temp[1]  && !temp[7] ) : !temp[1] || !temp[7] || ( !temp[3] && !temp[5] ));
}

int can_be_removed(int i, int j, int metodo ){
  int temp[9];
  memset(temp,0,sizeof(temp));

  int min_i, max_i, min_j, max_j;
  min_i = i-1 >= 0;
  max_i = i+1 < height;
  min_j = j-1 >= 0;
  max_j = j+1 < width;

  temp[0] = matrix[position][i][j];

  if(min_i){
    temp[1] = matrix[position][i-1][j];

    if(min_j){
      temp[8] = matrix[position][i-1][j-1];
    }

    if(max_j){
      temp[2] = matrix[position][i-1][j+1];
    }
  }
  if(max_i){
    temp[5] = matrix[position][i+1][j];

    if(min_j){
      temp[6] = matrix[position][i+1][j-1];
    }
    if(max_j){
      temp[4] = matrix[position][i+1][j+1];
    }
  }

  if(min_j){
    temp[7] = matrix[position][i][j-1];
  }

  if(max_j){
    temp[3] = matrix[position][i][j+1];
  }

  return i1(temp) &&  i2(temp)  && i3(temp, metodo);  
}

int print_output(FILE * fout){
  int i, j;

  for(i =0; i < height; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[position][i][j]);
    }
    fprintf(fout, "\n");
  }

  printf("Skeleton done!");

  return 0;
}

void copy_matrix(){
  int i,j;
  for(i =0; i < height; i++){
    for(j =0; j < width; j++){

      matrix[position][i][j] = matrix[!position][i][j];
    }      
  }
} 

int process_file(FILE * fout){
  int alt, i, j, acc = 0, flag;
  position = acc &1;
  do{
    flag = 0;
    for(alt=0; alt < 2; alt++){
      for(i =0; i < height; i++){
        for(j =0; j < width; j++){
          if(matrix[position][i][j] && can_be_removed(i,j,alt)){
            matrix[!position][i][j] = 0;
            flag = 1;
          }      
        } 
      }
      copy_matrix();

      acc++;
      position = acc &1;
    }


  }while(flag);

  printf("Writing the output file\n");

  print_output(fout);

  return 0;
}

void readPgmFile(FILE * fin, FILE * fout){
  char LINE[30];
  int i, j, k, r, temp;
  fprintf(fout,"%s\n", fgets(LINE, 30, fin));
  skip_comments(fin);

  r = fscanf(fin, "%d %d", &width, &height);

  fprintf(fout, "%d %d\n", width, height);
  printf("%d x %d\nInitializing...\n", width, height);

  /*int matrix[width][weigh];
    matrix = (int *) malloc(width * height * sizeof(int));
    array simula matrix matrix[i * col + j];
    */

  matrix = (int ***) malloc(2 * sizeof(int **)); 
  for(k = 0; k < 2; k++){
    matrix[k] = (int **) malloc(height * sizeof(int *)); 
  }

  printf("%d x %d\nInitializing...\n", width, height);

  for(k = 0; k < 2; k++){
    for (i=0; i<height; i++){
      matrix[k][i] = (int *) malloc(width * sizeof(int)); 
    }
  }

  printf("%d x %d\nInitializing...\n", width, height);

  for(i=0; i < height ; i++){ 
    for(j=0; j < width; j++){ 
      r = fscanf(fin, "%d ", &temp);
      matrix[0][i][j] = temp;
      matrix[1][i][j] = temp;
    }
  }

  printf("Processing file\n");

  process_file(fout);
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
  char *out;

  Stat buffer = (Stat) malloc(sizeof(struct stat));

  for(int i = 0; i < number_files; i++){

    if(!file_exists(files[i], &buffer)){

      if ((fin = fopen(files[i], "r"))==NULL){
        fprintf(stderr, "Failed to open input");
        exit(1);
      }

      out = (char *) malloc(strlen(files[i]) + 4 * sizeof(char));
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
