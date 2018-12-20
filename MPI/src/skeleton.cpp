#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>
#include <mpi.h> 

typedef struct stat *Stat;

int position, *aux, *matrix, *linha;
static int width, height, h;
static int* ret;
static FILE* fout;

void print_matrix(){
  for(int i = 0; i<height; i++){
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

  //#pragma omp parallel for reduction(+:total)
  for(res = 0; res < 8; res++){
    total += (temp[res+1] > 0);
  }

  return ((2 <= total) && (total <= 6));
}

int i2(int *temp){
  int res,trans = 0;

  trans += temp[8] != temp[1];

  //#pragma omp parallel for reduction(+:trans)
  for(res = 2; res < 9 && trans <3 ; res++){
    trans += temp[res-1] != temp[res];
  }

  return (trans == 2);
}

int i3(int *temp, int metodo){
  return ( metodo & 1 ? !temp[3] || !temp[5] || ( !temp[1]  && !temp[7] ) : !temp[1] || !temp[7] || ( !temp[3] && !temp[5] ));
}

int process_temp(int i, int j, int maxw, int metodo, int * temp){
  int total = 0, trans = 0;
  int indexs[8] = {(i-1)*maxw+j, (i-1)*maxw+j+1, (i*maxw)+j+1, 
                  (i+1)*maxw+j+1, (i+1)*maxw+j, (i+1)*maxw+j-1, 
                  i*maxw+(j-1), (i-1)*maxw+j-1};

  total = temp[indexs[0]] + temp[indexs[1]]  
    + temp[indexs[2]] + temp[indexs[3]]
    + temp[indexs[4]] + temp[indexs[5]] 
    + temp[indexs[6]] + temp[indexs[7]];

  int i1 = ((2 <= total) && (total <= 6));


  trans = (temp[indexs[0]] != temp[indexs[1]]) + (temp[indexs[1]] != temp[indexs[2]])
    + (temp[indexs[2]] != temp[indexs[3]]) + (temp[indexs[3]] != temp[indexs[4]])
    + (temp[indexs[4]] != temp[indexs[5]]) + (temp[indexs[5]] != temp[indexs[6]])
    + (temp[indexs[6]] != temp[indexs[7]]) + (temp[indexs[7]] != temp[indexs[0]]);

  int i2 = (trans == 2);
  
  int i3 = ( metodo & 1 ? !temp[indexs[2]] 
  || !temp[indexs[4]] 
  || ( !temp[indexs[0]]  && !temp[indexs[6]] ) : !temp[indexs[0]] 
  || !temp[indexs[6]] 
  || ( !temp[indexs[2]] && !temp[indexs[4]] ));

  
  return i1 && i2 && i3;
  
}

int can_be_removed(int w, int maxw, int h, int maxh, int metodo ){

  // Temp total tamanho total da temp (deve ser +1 em tudo da ativa)
  int tw, th, tsizew, tsizeh, tsize;
  // Temp ativa pontos uteis 
  int aw, ah, asizew, asizeh;
 
  tw = 0;
  th = 0;
  tsizew = 2 + maxw -w;
  tsizeh = 2 + maxh - h;
  tsize = tsizew * tsizeh ;
  
  aw  = 1;
  ah = 1;
  asizew = 1 + maxw - w;
  asizeh = 1 + maxh - h;

  int* temp = (int *) malloc( tsize * sizeof(int) );
  memset(temp,0, tsize * sizeof(int));

//ta mal apartir daqui
  for(int i = th, x=  h-1 ; i<tsizeh; i++, x++){
    for(int j = tw, y = w-1  ; j<tsizew; j++, y++){
      if(x >= 0 && x <= height && y <= width && y >= 0){
        temp[i * tsizew + j] = ret[x * width + y] ;
      }
    }
  }

  
  int index, flag;

  flag =0;  

  int imax = asizeh - ah;
  int jmax = asizew - aw;
  int loopi, loopj;
  //int loop;

  for(int i =0 ; i < imax; i++){
    for(int j = 0 ; j < jmax; j++){
      index = (h + i) * width + w + j;
      loopi = ah + i;
      loopj = aw + j;
   /*   loop = (temp[loopi * tsizew + loopj] && process_temp(loopi,loopj,tsizew,metodo,temp));
      flag += loop;
      if(loop) aux[index] = 0;*/

      if(temp[loopi * tsizew + loopj] && process_temp(loopi,loopj,tsizew,metodo,temp)){
        aux[index] = 0;
        flag=1;
      } 
    }
  }


  free(temp);

  return flag;
}

int print_output_last(){
  int i, j;

  int hi = rank * h;
  int hf = height-hi;

  for(i =1; i < hf+1; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  printf("Skeleton done!");

  return 0;
}

int print_output_init(){
  int i, j;

  for(i =0; i < h; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  printf("Skeleton done!");

  return 0;
}

int print_output(){
  int i, j;

  for(i =1; i < h+1; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", matrix[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  printf("Skeleton done!");

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
  int hi = rank * h;
  int hf = height-hi;
  
  for(alt=0; alt < 2; alt++){
    for(i = 1; i < hf+1; i++){
      for(j =0; j < width; j++){
        index = i * width +j;
        if(matrix[index] && can_be_removed(i,j,alt)){
          aux[index] = 0;

          flag = 1;
        }      
      } 
    }
    copy_matrix_mpi(hf+1);
  }

  printf("Writing the output file\n");

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

  printf("Writing the output file\n");

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

  printf("Writing the output file\n");

  return flag;
}

void readPgmFile(FILE * fin, FILE * fout){
  char LINE[30];
  int i, j, r, temp;
  fprintf(fout,"%s\n", fgets(LINE, 30, fin));
  skip_comments(fin);

  r = fscanf(fin, "%d %d", &width, &height);

  if(r<0){
    exit(1);
  }

  fprintf(fout, "%d %d\n", width, height);
  printf("%d x %d\nInitializing...\n", width, height);

  //int matrix[width][weigh];
  ret = (int *) malloc(width * height * sizeof(int));
  aux = (int *) malloc(width * height * sizeof(int));
  matrix = (int *) malloc(width * height * sizeof(int));  
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

int process_files(int number_files, char *files[], FILE* fout){
  FILE *fin;
  char *out;

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

      readPgmFile(fin, fout);

      fclose(fin);
      fclose(fout);
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

  if(argc < 2){
    printf("No input files !\n ./skeleton_seq [IMAGE] \n");
    MPI_Finalize();
    return 0; 
  }

  //Parse file; Calculate the number of lines for each process
  if (rank == 0) {
    process_files(argc-1, &argv[1], fout);
    h = height/size;
  }

  // Waits until the file is parsed
  MPI_Barrier( MPI_COMM_WORLD );

  if (rank == 0) {

    //Copy the first h+1 lines to the process 0's matrix and aux
    memcpy( matrix, ret, width * (h+1) * sizeof(int));
    memcpy( aux, ret, width * (h+1) * sizeof(int));

    int flag = 0;
    int flagr = 1;
    do {

      //flag: check if its the first iteration
      //flagr: check if the process #rank+1 terminated
      if (flag && flagr) {

        MPI_Recv( linha, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );

        linha[0]==2 ? flagr=0 : flagr=1;

        if(flagr){
          memcpy( &matrix[h], linha, width * sizeof(int));
        }
        
        memset(linha, 0, sizeof(linha));

      }

      flag = process_file_init(fout, rank, size);

      if(flagr){

        memcpy( linha, &matrix[h-1], width * sizeof(int));
        MPI_Send( linha, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        memset(linha, 0, sizeof(linha));

      }

    }while(flag);

    //send the first int=2 to show that this process terminated
    memset(linha, 0, sizeof(linha));
    linha[0]=2;
    MPI_Send( linha, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);

  }
  else if (rank == size-1 ) {
    int hi = rank * h;
    //Calculate how many lines will the last process iterate
    int nh = height - hi; 

    //Copy the last nh lines to the last process' matrix and aux
    memcpy( matrix, &ret[hi-1], width * (nh+1) * sizeof(int));
    memcpy( aux, &ret[hi-1], width * (nh+1) * sizeof(int));

    int flag = 0;
    int flagr = 1;
    do {

      //check if its the first iteration
      if (flag && flagr){

        MPI_Recv( linha, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        
        linha[0]==2 ? flagr=0 : flagr=1;

        if(flagr){
          memcpy( matrix, linha, width * sizeof(int));
        }
        
        memset(linha, 0, sizeof(linha));
        
      } 

      flag = process_file(fout, rank, size);

      if(flagr){

        memcpy( linha, &matrix[1], width * sizeof(int));
        MPI_Send( linha, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
        memset(linha, 0, sizeof(linha));

      }


    }while(flag);
    
    //send the first int=2 to show that this process terminated
    memset(linha, 0, sizeof(linha));
    linha[0]=2;
    MPI_Send( linha, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);

  }
  else{
    int hi = rank * h;

    //Copy the respective h+2 lines to the process #rank's matrix and aux
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

        MPI_Recv( linha, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
        linha[0]==2 ? flagr1=0 : flagr1=1;

        if(flagr1){
          memcpy( matrix, linha, width * sizeof(int));
        }

        memset(linha, 0, sizeof(linha));
        
        MPI_Recv( linha, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD, &status );
        linha[0]==2 ? flagr2=0 : flagr2=1;
        
        if(flagr2){
          memcpy( &matrix[h+1], linha, width * sizeof(int));
        }

        memset(linha, 0, sizeof(linha));
        
      }

      flag = process_file(fout, rank, size);
      if(flagr1){
        memcpy( linha, &matrix[1], width * sizeof(int));
        MPI_Send( linha, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
        memset(linha, 0, sizeof(linha));
      }

      if(flagr2){ 
        memcpy( linha, &matrix[h], width * sizeof(int));
        MPI_Send( linha, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
        memset(linha, 0, sizeof(linha));
      }

    }while(flag);

    //send the first int=2 to show that this process terminated
    memset(linha, 0, sizeof(linha));
    linha[0]=2;
    MPI_Send( linha, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD);
    MPI_Send( linha, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);

  }

  MPI_Barrier( MPI_COMM_WORLD );

  if ( rank==0 ) {
    print_output_init();
    msg=rank;
    MPI_Send( &msg, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
  }
  else if ( rank==size-1 ) {
    MPI_Recv( &msg, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
    print_output_last();
  }
  else{
    MPI_Recv( &msg, 1, MPI_INT, rank-1, 0, MPI_COMM_WORLD, &status );
    print_output();
    msg=rank;
    MPI_Send( &msg, 1, MPI_INT, rank+1, 0, MPI_COMM_WORLD);
  }
  
  MPI_Finalize();

  printf("That's all folks !\n");

  return 0; 
}
