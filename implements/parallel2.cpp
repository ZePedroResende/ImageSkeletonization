#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

typedef struct stat *Stat;

int width, height, position, *ret, *aux, nbwr, bw, nbhr, bh, nbw, nbh;

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

  printf("Skeleton done!");

  return 0;
}

void copy_matrix(){
  memcpy(ret,aux, width * height * sizeof(int));
} 

int process_file(FILE * fout){
  int i, j, alt, flag;

  flag =0;

  nbw = width/32;
  nbwr = width%32;

  if(nbwr>0){
    nbw++;
  }
  bw = width/nbw;

  nbh = height/32;
  nbhr = height%32;

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

  printf("Writing the output file\n");

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
  printf("%d x %d\nInitializing...\n", width, height);

  //int matrix[width][weigh];
  ret = (int *) malloc(width * height * sizeof(int));
  aux = (int *) malloc(width * height * sizeof(int));
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

  if(argc < 2){
    printf("No input files !\n ./skeleton_seq [IMAGE] \n");
  }else{
    process_files(argc-1, &argv[1]);
  }

  printf("That's all folks !\n");
  return 0;
}

