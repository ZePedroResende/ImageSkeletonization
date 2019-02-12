#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <libgen.h>
#include <errno.h>
#include <omp.h>
#include "papi.h"
#define TIME_RESOLUTION 1000000// time measuring resolution (us)

typedef struct stat *Stat;

int width, height, position, *ret, *aux, nbwr, bw, nbhr, bh, nbw, nbh, BLOCK;
long long unsigned initial_time;
timeval t;
int NUM_EVENTS=0;
int *PAPI_events;
long long *counters;

void printResults (long long unsigned tt) {
		  printf("%llu", tt);
}

void start (void) {
		  gettimeofday(&t, NULL);
		    initial_time = t.tv_sec * TIME_RESOLUTION + t.tv_usec;
}

long long unsigned stop (void) {
		  gettimeofday(&t, NULL);
		    long long unsigned final_time = t.tv_sec * TIME_RESOLUTION + t.tv_usec;

			  return final_time - initial_time;
}

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

int can_be_removed(int i, int j, int metodo ){
  int w, maxw, h, maxh;

  if(i==(nbw-1)){
    w=i*bw;
    maxw = width;
    h=j*bh;

    if(j==(nbh-1)){
      maxh=height;
    }else{
      maxh= j*bh + bh;
    }
  }else{
    w=i*bw;
    maxw = i*bw + bw;
    h=j*bh;
    if(j==(nbh-1)){
      maxh=height;
    }else{
      maxh= j*bh + bh;
    }
  }

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
  int loop;

  for(int i =0 ; i < imax; i++){
    for(int j = 0 ; j < jmax; j++){
      index = (h + i) * width + w + j;
      loopi = ah + i;
      loopj = aw + j;

      loop = (temp[loopi * tsizew + loopj] && process_temp(loopi,loopj,tsizew,metodo,temp));
      aux[index] = loop ? 0 : aux[index];
      flag = loop ? 1 : flag;

      /*
      if(temp[loopi * tsizew + loopj] && process_temp(loopi,loopj,tsizew,metodo,temp)){
        aux[index] = 0;
        flag=1;
      } */
    }
  }


  free(temp);

  return flag;
}

int print_output(FILE * fout){
  int i, j;

  for(i =0; i < height; i++){
    for(j =0; j < width; j++){
      fprintf(fout, "%d ", ret[i * width + j]);
    }
    fprintf(fout, "\n");
  }

  //printf("Skeleton done!");

  return 0;
}

void copy_matrix(){
  memcpy(ret,aux, width * height * sizeof(int));
} 

int process_file(FILE * fout){
  int i, j, alt, flag;
  double start, end;

  flag =0;
  

  nbw = width/BLOCK;
  nbwr = width%BLOCK;

  if(nbwr>0){
    nbw++;
  }
  bw = width/nbw;

  nbh = height/BLOCK;
  nbhr = height%BLOCK;

  if(nbhr>0){
    nbh++;
  }

  bh = height / nbh;

  do{
    flag=0;
    for(alt=0; alt < 2; alt++){
      for(i =0; i < nbw; i++){
        #pragma omp parallel for
        for(j =0; j < nbh; j++){
          flag = can_be_removed(i, j, alt) ? 1 : flag;
        } 
      }

      copy_matrix();
    }
  }while(flag);
  //printf("Writing the output file\n");

  print_output(fout);

  return 0;
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
  //printf("%d x %d\nInitializing...\n", width, height);

  //int matrix[width][weigh];
  ret = (int *) malloc(width * height * sizeof(int));
  aux = (int *) malloc(width * height * sizeof(int));
  //array simula matrix matrix[i * col + j];

  //printf("%d x %d\nInitializing...\n", width, height);

  
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
  //printf("Processing file\n");

  process_file(fout);
}

int file_exists(char *file_name, Stat *buffer){
  int result;

  if ((result = stat(file_name, *buffer))){
    fprintf(stderr,"File %s doesn't exist!\n", file_name);
  }

  return result;
}



void open_files(char* in, char *out){
  FILE *fin, *fout;
  Stat buffer = (Stat) malloc(sizeof(struct stat));

  if(!file_exists(in, &buffer)){

    if ((fin = fopen(in, "r"))==NULL){
      fprintf(stderr, "Failed to open input");
      exit(1);
    }

    if ((fout = fopen( out, "ab+"))==NULL){
      fprintf(stderr, "Failed to open output\n");
      exit(1);
    }

    readPgmFile(fin, fout);

    fclose(fin);
    fclose(fout);
  } else {
    fprintf(stderr, "Can't find file %s", in);
  }

  free(buffer);
}


void run_dotproduct(char *in, char *out, void (*f) (char*, char*)){
  long long unsigned tt;

  start();
  f(in,out);
  tt = stop();
  printResults(tt);
}

void papi ( char *in, char *out, void (*f) (char*, char*), char* tag ) {


  int i;
  PAPI_start_counters( PAPI_events, NUM_EVENTS );

  f(in,out);

  PAPI_stop_counters( counters, NUM_EVENTS );

  if (!strcmp("mr",tag) ) {
    printf("%f", (counters[1]*100.0f)/(counters[0]*1.0f) );
  }
  else {
    for (i=0; i<NUM_EVENTS; i++)
      printf("%lld",counters[i]);
  }
  printf("\n");
}

int main (int argc, char *argv[]) {

  BLOCK = atoi(argv[1]);
  char *in =  argv[2];
  char *out = argv[3];

  void (*implement) (char*, char*);

    implement = open_files;

    if ( !strcmp("time",argv[4]) ) {
        run_dotproduct(in,out,implement);
    } else {
        PAPI_library_init(PAPI_VER_CURRENT);
        if ( !strcmp("mrl1",argv[4]) ) {
            NUM_EVENTS = 2;
            counters = (long long *) malloc (NUM_EVENTS *sizeof(long long) );
            PAPI_events = (int*) malloc (sizeof(int) * NUM_EVENTS);
            PAPI_events[0] = PAPI_LD_INS;
            PAPI_events[1] = PAPI_L2_DCR;
        }
        else if ( !strcmp("mrl2",argv[4]) ) {
            NUM_EVENTS = 2;
            counters = (long long *) malloc (NUM_EVENTS *sizeof(long long) );
            PAPI_events = (int*) malloc (sizeof(int) * NUM_EVENTS);
            PAPI_events[0] = PAPI_L2_DCR;
            PAPI_events[1] = PAPI_L3_DCR;    
        }
        else if ( !strcmp("mrl3",argv[4]) ) {
            NUM_EVENTS = 2;
            counters = (long long *) malloc (NUM_EVENTS *sizeof(long long) );
            PAPI_events = (int*) malloc (sizeof(int) * NUM_EVENTS);
            PAPI_events[0] = PAPI_L3_TCA;
            PAPI_events[1] = PAPI_L3_TCM;
        }
        else if ( !strcmp("L3_TCM",argv[4]) ) {
            NUM_EVENTS = 1;
            counters = (long long *) malloc (NUM_EVENTS *sizeof(long long) );
            PAPI_events = (int*) malloc (sizeof(int) * NUM_EVENTS);
            PAPI_events[0] = PAPI_L3_TCM;
        }
        else if ( !strcmp("FP_INS",argv[4]) ) {
            NUM_EVENTS = 1;
            counters = (long long *) malloc (NUM_EVENTS *sizeof(long long) );
            PAPI_events = (int*) malloc (sizeof(int) * NUM_EVENTS);
            PAPI_events[0] = PAPI_FP_INS;
        }
        else {
            return 1;
        }
        papi( in, out, implement, argv[4] );
    }

  return 0;
}
