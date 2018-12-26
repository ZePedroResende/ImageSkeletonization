#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include <mpi.h> 

typedef struct stat *Stat;

int position, nh, *aux, *matrix, *linha;
int width, height, h, height_process, flag_global;
int* ret;
char *out;
char LINE[30];

void print_matrix(){
  for(int i = 0; i<height_process; i++){
    for(int j =0; j< width; j++){
      printf("%d\t", ret[i * width + j]);
    }
    printf("\n");
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
  max_i = i+1 < height_process;
  min_j = j-1 >= 0;
  max_j = j+1 < width;
  
  temp[0] = matrix[i * width + j];

  if(min_i){
    temp[1] = matrix[(i -1) * width + j];

    if(min_j){
      temp[8] = matrix[(i -1) * width + (j-1)];
    }

    if(max_j){
      temp[2] = matrix[(i -1) * width + (j+1)];
    }
  }
  if(max_i){
    temp[5] = matrix[(i +1) * width + j];

    if(min_j){
      temp[6] = matrix[(i +1) * width + (j-1)];
    }
    if(max_j){
      temp[4] = matrix[(i +1) * width + (j+1)];
    }
  }

  if(min_j){
    temp[7] = matrix[i * width + (j-1)];
  }

  if(max_j){
    temp[3] = matrix[i * width + (j+1)];
  }

  return i1(temp) &&  i2(temp)  && i3(temp, metodo);  
}


int print_output_last(int rank, FILE *fout){
  int i, j;

  for(i =1; i < nh+1; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  printf("Skeleton done!\n");

  return 0;
}

int print_output_init(FILE *fout){
  int i, j;

  printf("Printing first process!\n");

  for(i =0; i < h; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  printf("Finishing first process!\n");

  return 0;
}

int print_output(FILE *fout){
  int i, j;

  for(i =1; i < h+1; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  return 0;
}

void copy_matrix(){
  memcpy( ret, aux, width * height * sizeof(int));
}

void copy_matrix_mpi(int lines){
  memcpy( matrix, aux, width * lines * sizeof(int));
}

int process_file_last(int alt){
  int i, j, index, flag;
  flag = 0;
  
    for(i = 1; i < nh+1; i++){
      for(j =0; j < width; j++){
        index = i * width +j;
        if(matrix[index] && can_be_removed(i,j,alt)){
          aux[index] = 0;

          flag = 1;
        }      
      } 
    }
    copy_matrix_mpi(nh+1);

  return flag;
}

int process_file(int alt){
  int i, j, index, flag;
  flag = 0;

    for(i = 1; i < h+1; i++){
      for(j =0; j < width; j++){
        index = i * width +j;
        if(matrix[index] && can_be_removed(i,j,alt)){
          aux[index] = 0;

          flag = 1;
        }      
      } 
    }
    copy_matrix_mpi(h+2);

  return flag;
}

int process_file_init(int alt){
  int i, j, index, flag;
  flag = 0;

    for(i = 0; i < h; i++){
      for(j =0; j < width; j++){
        index = i * width +j;
        if(matrix[index] && can_be_removed(i,j,alt)){
          aux[index] = 0;

          flag = 1;
        }      
      } 
    }
    copy_matrix_mpi(h+1);

  return flag;
}

void readPgmFile(FILE * fin){
  int i, j, r, temp;
  fgets(LINE, 30, fin);
  skip_comments(fin);

  r = fscanf(fin, "%d %d", &width, &height);

  if(r<0){
    exit(1);
  }

  printf("%d x %d\nInitializing...\n", width, height);

  //int matrix[width][weigh];
  ret = (int *) malloc(width * height * sizeof(int));
  aux = (int *) malloc(width * height * sizeof(int));
  linha = (int *) malloc(width * sizeof(int));
  //array simula matrix matrix[i * col + j];

  printf("%d x %d\nInitializing...\n", width, height);

  
  for(i=0; i < height ; i++){ 
    for(j=0; j < width; j++){ 
      r = fscanf(fin, "%d ", &temp);
      if(r<0){
        exit(1);
      }
      aux[(i * width) +j] = temp;
    }
  }

  copy_matrix(); 
  printf("Processing file\n");
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

  *out_path = '\0';
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

      out =  (char*) malloc(strlen(files[i]) + 4 * sizeof(char)); 
      output_file(files[i], out);
      printf("file %s\n", out);

      if ((fout = fopen( out, "ab+"))==NULL){
        fprintf(stderr, "Failed to open output\n");
        exit(1);
      }

      readPgmFile(fin);

      fclose(fin);
    }

  }

  free(buffer);

  return 0;
}

int main(int argc, char *argv[]){
  
  int rank, size, flag, msg;

  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank( MPI_COMM_WORLD, &rank );
  MPI_Comm_size( MPI_COMM_WORLD, &size );

  FILE *fout;

  if(argc < 2){
    printf("No input files !\n ./skeleton_seq [IMAGE] \n");
    MPI_Finalize();
    return 0; 
  }

  //Parse file; Calculate the number of lines for each process
  if(rank==0){

    process_files(argc-1, &argv[1]);
    h = height/size;
    MPI_Bcast( &height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast( &width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast( ret, width * height, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast( &h, 1, MPI_INT, 0, MPI_COMM_WORLD);

  }else{

    MPI_Bcast( &height, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast( &width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    ret = (int *) malloc(width * height * sizeof(int));
    MPI_Bcast( ret, width * height, MPI_INT, 0, MPI_COMM_WORLD);
    linha = (int *) malloc(width * sizeof(int));
    MPI_Bcast( &h, 1, MPI_INT, 0, MPI_COMM_WORLD);

  }


  // Waits until the file is parsed
  MPI_Barrier( MPI_COMM_WORLD );
  printf("rank: %d, width: %d, height: %d, h: %d\n", rank, width, height, h);

  if (rank == 0) {
    height_process = h+1;
    free(aux);

    //Copy the first h+1 lines to the process 0's matrix and aux
    aux = (int *) malloc(width * (h+1) * sizeof(int));
    matrix = (int *) malloc(width * (h+1) * sizeof(int));  
    memcpy( matrix, ret, width * (h+1) * sizeof(int));
    memcpy( aux, ret, width * (h+1) * sizeof(int));

    int flag1 = 0;
    int flag2 = 0;
    int flag = 0;

    do {
      
      //flag: check if its the first iteration
      //flagr: check if the process #rank+1 terminated
      if (flag) {

        memcpy( linha, &matrix[(h-1)*width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));
        MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );
        memcpy( &matrix[h*width], linha, width * sizeof(int));
        memset(linha, 0, width * sizeof(int));

      }

      flag1 = process_file_init(0);

      memcpy( linha, &matrix[(h-1)*width], width * sizeof(int));
      MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
      memset(linha, 0, width * sizeof(int));
      MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );
      memcpy( &matrix[h*width], linha, width * sizeof(int));
      memset(linha, 0, width * sizeof(int));

      flag2 = process_file_init(1);

      flag = (flag1 || flag2);

      MPI_Reduce( &flag, &flag_global, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
      if(!flag_global){
        flag=0;
      }else{
        flag=1;
      }

      MPI_Bcast( &flag, 1, MPI_INT, 0, MPI_COMM_WORLD);

    }while(flag);

  }else if (rank == size-1 ) {
    int hi = rank * h;
    //Calculate how many lines will the last process iterate
    nh = height - hi; 
    height_process = nh + 1;

    //Copy the last nh lines to the last process' matrix and aux
    aux = (int *) malloc(width * height_process * sizeof(int));
    matrix = (int *) malloc(width * height_process * sizeof(int)); 
    memcpy( matrix, &ret[(hi-2) * width], width * height_process * sizeof(int));
    memcpy( aux, &ret[(hi-2) * width], width * height_process * sizeof(int));

    int flag = 0;
    int flag1 = 0;
    int flag2 = 0;

    do {

      //check if its the first iteration
      if (flag){

        MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        memcpy( matrix, linha, width * sizeof(int));
        memset(linha, 0, width * sizeof(int));
        memcpy( linha, &matrix[width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));
        
      } 
      
      flag1 = process_file_last(0);

      MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
      memcpy( matrix, linha, width * sizeof(int));
      memset(linha, 0, width * sizeof(int));
      memcpy( linha, &matrix[width], width * sizeof(int));
      MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
      memset(linha, 0, width * sizeof(int));
        
      
      flag2 = process_file_last(1);

      flag = (flag1 || flag2);

      MPI_Reduce( &flag, &flag_global, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
      MPI_Bcast( &flag, 1, MPI_INT, 0, MPI_COMM_WORLD);

    }while(flag);

  }else{
    int hi = rank * h;
    height_process = h + 2;

    //Copy the respective h+2 lines to the process #rank's matrix and aux
    aux = (int *) malloc(width * (h+2) * sizeof(int));
    matrix = (int *) malloc(width * (h+2) * sizeof(int)); 
    memcpy( matrix, &ret[hi-1], width * (h+2) * sizeof(int));
    memcpy( aux, &ret[hi-1], width * (h+2) * sizeof(int));

    int flag = 0;
    int flag1 = 0;
    int flag2 = 0;

    do {
    
      //flag: check if its the first iteration
      //flagr1: check if the process #rank-1 terminated
      //flagr2: check if the process #rank+1 terminated
      if(flag){
        MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        memcpy( matrix, linha, width * sizeof(int));
        memset(linha, 0, width * sizeof(int));
        memcpy( linha, &matrix[width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));

        memcpy( linha, &matrix[h * width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));
        MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );
        memcpy( &matrix[(h+1) * width], linha, width * sizeof(int));
        memset(linha, 0, width * sizeof(int));

      }

      flag1 = process_file(0);

      MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
      memcpy( matrix, linha, width * sizeof(int));
      memset(linha, 0, width * sizeof(int));
      memcpy( linha, &matrix[width], width * sizeof(int));
      MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
      memset(linha, 0, width * sizeof(int));

      memcpy( linha, &matrix[h * width], width * sizeof(int));
      MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
      memset(linha, 0, width * sizeof(int));
      MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );
      memcpy( &matrix[(h+1) * width], linha, width * sizeof(int));
      memset(linha, 0, width * sizeof(int));

      flag2 = process_file(1);

      flag = (flag1 || flag2);

      MPI_Reduce( &flag, &flag_global, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
      MPI_Bcast( &flag, 1, MPI_INT, 0, MPI_COMM_WORLD);

    }while(flag);

  }


  MPI_Barrier( MPI_COMM_WORLD );


  if ( rank==0 ) {

    if ((fout = fopen( out, "ab+"))==NULL){
      fprintf(stderr, "Failed to open output\n");
      exit(1);
    }

    fprintf(fout,"%s\n", LINE);
    fprintf(fout, "%d %d\n", width, height);

    int i;

    free(aux);
    aux = (int *) malloc(width * height * sizeof(int));

    memcpy(aux, matrix, h * width * sizeof(int));

    free(matrix);
    matrix = (int *) malloc(width * (h+2) * sizeof(int)); 

    for(i=1; i < size-1; i++){

      MPI_Bcast( matrix, (h+2) * width, MPI_INT, i, MPI_COMM_WORLD);
      memcpy(&aux[i*h*width], &matrix[width], h * width * sizeof(int));
      memset(matrix, 0, width * (h+2) * sizeof(int));

    }

    free(matrix);

    MPI_Bcast( &nh, 1, MPI_INT, size-1, MPI_COMM_WORLD);

    matrix = (int *) malloc(width * (nh+1) * sizeof(int)); 

    MPI_Bcast( matrix, nh * width, MPI_INT, size-1, MPI_COMM_WORLD);
    memcpy(&aux[i*h*width], matrix, nh * width * sizeof(int));

    int j;

    for(i =0; i < height; i++){
      for(j =0; j < width; j++){
        fprintf(fout, "%d ", aux[i * width + j]);
      }
      fprintf(fout, "\n");
    }

    printf("Skeleton done!\n");

    fclose(fout);

  }else if ( rank==size-1 ) {

    free(aux);
    aux = (int *) malloc(width * (h+2) * sizeof(int)); 

    for(int i=1; i < size-1; i++){
      
      MPI_Bcast( aux, (h+2) * width, MPI_INT, i, MPI_COMM_WORLD);
      memset(aux, 0, width * (h+2) * sizeof(int));

    }

    MPI_Bcast( &nh, 1, MPI_INT, size-1, MPI_COMM_WORLD);
    MPI_Bcast( &matrix[width], nh * width, MPI_INT, size-1, MPI_COMM_WORLD);

  }else{
    memset(aux, 0, width * (h+2) * sizeof(int));

    for(int i=1; i < size-1; i++){
      if(i==rank){
        MPI_Bcast( matrix, (h+2) * width, MPI_INT, i, MPI_COMM_WORLD);
      }else{
        MPI_Bcast( aux, (h+2) * width, MPI_INT, i, MPI_COMM_WORLD);
        memset(aux, 0, width * (h+2) * sizeof(int));
      }

    }

    MPI_Bcast( &nh, 1, MPI_INT, size-1, MPI_COMM_WORLD);
    MPI_Bcast( &aux[width], nh * width, MPI_INT, size-1, MPI_COMM_WORLD);

  }
  
  MPI_Barrier( MPI_COMM_WORLD );

  MPI_Finalize();

  printf("That's all folks !\n");

  return 0; 
}
