#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include <mpi.h> 

typedef struct stat *Stat;

int position, nh, *aux, *matrix, *linha;
int width, height, h, height_process;
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

int process_file_last(FILE * fout, int rank, int size){
  int alt, i, j, index, flag;
  flag = 0;
  
  for(alt=0; alt < 2; alt++){
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
  }

  printf("finishing rank: %d fim: %d \n", rank, nh+1);

  return flag;
}

int process_file(FILE * fout, int rank, int size){
  int alt, i, j, index, flag;
  flag = 0;
  for(alt=0; alt < 2; alt++){
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
  }

  printf("finishing rank: %d fim: %d \n", rank, h+1);

  return flag;
}

int process_file_init(FILE * fout, int rank, int size){
  int alt, i, j, index, flag;
  flag = 0;

  for(alt=0; alt < 2; alt++){
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
  }

  printf("finishing rank: %d fim: %d \n", rank, h);

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

    int flag = 0;
    int flagr = 1;
    do {
      
      //flag: check if its the first iteration
      //flagr: check if the process #rank+1 terminated
      if (flag && flagr) {

        MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );

        linha[0]==2 ? flagr=0 : flagr=1;

        if(flagr){
          memcpy( &matrix[h*width], linha, width * sizeof(int));
        }
        
        memset(linha, 0, width * sizeof(int));

      }

      flag = process_file_init(fout, rank, size);

      if(flagr){

        memcpy( linha, &matrix[(h-1)*width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));

      }

    }while(flag);

    //send the first int=2 to show that this process terminated
    if(flagr){

      MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );

      if(flagr){
        memset(linha, 0, width * sizeof(int));
        linha[0]=2;
        MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
      }

    }

  }else if (rank == size-1 ) {
    int hi = rank * h;
    //Calculate how many lines will the last process iterate
    nh = height - hi; 
    height_process = nh + 1;

    //Copy the last nh lines to the last process' matrix and aux
    aux = (int *) malloc(width * (nh+1) * sizeof(int));
    matrix = (int *) malloc(width * (nh+1) * sizeof(int)); 
    memcpy( matrix, &ret[hi-1], width * (nh+1) * sizeof(int));
    memcpy( aux, &ret[hi-1], width * (nh+1) * sizeof(int));

    int flag = 0;
    int flagr = 1;

    do {

      //check if its the first iteration
      if (flag && flagr){

        MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        
        linha[0]==2 ? flagr=0 : flagr=1;

        if(flagr){

          memcpy( matrix, linha, width * sizeof(int));
          memset(linha, 0, width * sizeof(int));
          memcpy( linha, &matrix[width], width * sizeof(int));
          MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
          
        }

          memset(linha, 0, width * sizeof(int));
        
      } 

      flag = process_file_last(fout, rank, size);

    }while(flag);
    
    if(flagr){

      //send the first int=2 to show that this process terminated
      MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
      linha[0]==2 ? flagr=0: flagr=1;
      if(flagr){
        memset(linha, 0, width * sizeof(int));
        linha[0]=2;
        MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
      }
      
    }

  }else{
    int hi = rank * h;
    height_process = h + 2;

    //Copy the respective h+2 lines to the process #rank's matrix and aux
    aux = (int *) malloc(width * (h+2) * sizeof(int));
    matrix = (int *) malloc(width * (h+2) * sizeof(int)); 
    memcpy( matrix, &ret[hi-1], width * (h+2) * sizeof(int));
    memcpy( aux, &ret[hi-1], width * (h+2) * sizeof(int));

    int flag = 0;
    int flagr1 = 1;
    int flagr2 = 1;

    do {
    
      //flag: check if its the first iteration
      //flagr1: check if the process #rank-1 terminated
      //flagr2: check if the process #rank+1 terminated
      if (flag && (flagr1 || flagr2)){

        MPI_Recv( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        linha[0]==2 ? flagr1=0 : flagr1=1;

        if(flagr1){
          memcpy( matrix, linha, width * sizeof(int));
        }

        memset(linha, 0, width * sizeof(int));
        
        MPI_Recv( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );
        linha[0]==2 ? flagr2=0 : flagr2=1;
        
        if(flagr2){
          memcpy( &matrix[(h+1)*width], linha, width * sizeof(int));
        }

        memset(linha, 0, width * sizeof(int));
        
      }

      flag = process_file(fout, rank, size);
      if(flagr1){

        memcpy( linha, &matrix[width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));

      }

      if(flagr2){ 

        memcpy( linha, &matrix[h*width], width * sizeof(int));
        MPI_Ssend( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        memset(linha, 0, width * sizeof(int));

      }

    }while(flag);

    //send the first int=2 to show that this process terminated
    memset(linha, 0, width * sizeof(int));
    linha[0]=2;
    MPI_Send( linha, width, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
    MPI_Send( linha, width, MPI_INT, rank+1, 0, MPI_COMM_WORLD);

  }


  MPI_Barrier( MPI_COMM_WORLD );


  if ( rank==0 ) {

    if ((fout = fopen( out, "ab+"))==NULL){
      fprintf(stderr, "Failed to open output\n");
      exit(1);
    }

    fprintf(fout,"%s\n", LINE);
    fprintf(fout, "%d %d\n", width, height);

    //print_output_init(fout);

    msg=rank;
    int i;

    free(matrix);
    matrix = (int *) malloc(width * (h+2) * sizeof(int)); 

    for(i=1; i < size-1; i++){

      MPI_Ssend( &msg, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Recv( matrix, (h+2) * width, MPI_INT, i, 0, MPI_COMM_WORLD, &status );

      print_output(fout);
      memset(matrix,0, (h+2) * width * sizeof(int));

    }

    free(matrix);

    MPI_Ssend( &msg, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD);
    MPI_Recv( &nh, 1, MPI_INT, size-1, 0, MPI_COMM_WORLD, &status);

    matrix = (int *) malloc(width * (nh+1) * sizeof(int));

    MPI_Recv( matrix, width * (nh+1), MPI_INT, size-1, 0, MPI_COMM_WORLD, &status );

    printf("\n\n\n\n\n PRINTING LAST ONE \n\n\n\n\n");

    print_output_last(size-1, fout);

    fclose(fout);

  }else if ( rank==size-1 ) {

    MPI_Recv( &msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status );
    MPI_Ssend( &nh, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Ssend( matrix, width * (nh+1), MPI_INT, 0, 0, MPI_COMM_WORLD);

  }else{

    MPI_Recv( &msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status );
    MPI_Ssend( matrix, (h+2) * width, MPI_INT, 0, 0, MPI_COMM_WORLD);

  }
  
  MPI_Barrier( MPI_COMM_WORLD );

  MPI_Finalize();

  printf("That's all folks !\n");

  return 0; 
}
