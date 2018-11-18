#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

typedef struct stat *Stat;

int width, height, position, *ret, *aux;

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

int process_temp(int i, int j, int maxi, int metodo, int * temp){
  int total = 0, trans = 0;

  total = (temp[(i-1)*maxi+j] > 0) + (temp[(i-1)*maxi+j+1] > 0) 
    + (temp[(i*maxi)+j+1] > 0) + (temp[(i+1)*maxi+j+1] > 0) 
    + (temp[(i+1)*maxi+j] > 0) + (temp[(i+1)*maxi+j-1] > 0) 
    + (temp[i*maxi+j-1] > 0) + (temp[(i-1)*maxi+j-1] > 0);

  int i1 = ((2 <= total) && (total <= 6));

  trans += temp[(i-1)*maxi+j-1] != temp[(i-1)*maxi+j];

  trans = (temp[(i-1)*maxi+j] != temp[(i-1)*maxi+j+1]) + (temp[(i-1)*maxi+j+1] != temp[(i*maxi)+j+1])
    + (temp[(i*maxi)+j+1] != temp[(i+1)*maxi+j+1]) + (temp[(i+1)*maxi+j+1] != temp[(i+1)*maxi+j])
    + (temp[(i+1)*maxi+j] != temp[(i+1)*maxi+j-1]) + (temp[(i+1)*maxi+j-1] != temp[i*maxi+j-1])
    + (temp[i*maxi+j-1] != temp[(i-1)*maxi+j-1]) + (temp[(i-1)*maxi+j-1] != temp[(i-1)*maxi+j]);

  int i2 = (trans == 2);
  
  int i3 = ( metodo & 1 ? !temp[(i*maxi)+j+1] 
  || !temp[(i+1)*maxi+j] 
  || ( !temp[(i-1)*maxi+j]  && !temp[i*maxi+j-1] ) : !temp[(i-1)*maxi+j] 
  || !temp[i*maxi+j-1] 
  || ( !temp[(i*maxi)+j+1] && !temp[(i+1)*maxi+j] ));

  printf("%d %d %d \n", i1, i2, i3);
  
  return i1 && i2 && i3;
  
}

int can_be_removed(int w, int maxw, int h, int maxh, int metodo ){

  // Temp total tamanho total da temp (deve ser +1 em tudo da ativa)
  int tw, tmaxw, th, tmaxh, tsizew, tsizeh, tsize;
  // Temp ativa pontos uteis 
  int aw, amaxw, ah, amaxh, asizew, asizeh, asize;
  //Temp a preencher a contar com os vizinhos que precisam de ser copiados
  int pw, pmaxw, ph, pmaxh, psizew, psizeh, psize;
 
  tw = 0;
  tmaxw = 2 + maxw - w;
  th = 0;
  tmaxh = 2 + maxh - h;
  tsizew = 2 + maxw -w;
  tsizeh = 2 + maxh - h;
  tsize = tsizew * tsizeh ;
  
  aw  = 1;
  amaxw = 1 + maxw - w;
  ah = 1;
  amaxh = 1 + maxh - h;
  asizew = 1 + maxw - w;
  asizeh = 1 + maxh - h;
  asize = asizew * asizeh;
 
  pw  = !(w> 0);
  pmaxw = tmaxw - !( maxw < width );
  ph = !(h> 0);;
  pmaxh = tmaxh - !(maxh < height);
  psizew = 1 + pmaxw - pw;
  psizeh = 1 + pmaxh - ph;
  psize = psizew * psizeh;
  

  int* temp = (int *) malloc( tsize * sizeof(int) );
  memset(temp,0, tsize * sizeof(int));
  


//ta mal apartir daqui
  for(int i = th,x=resh ; i<tsizeh; i++, x= 0){
    for(int j = tw, y = resw; j<tsizew; j++, y=0){
      temp[i * tsizew + j] = ret[x * width + y]; 
    }
  }

  print_matrix();

  for(int i = th,x=resh ; i<tsizeh; i++, x= 0){
    for(int j = tw, y = resw; j<tsizew; j++, y=0){
     printf("(%d,%d)\t", temp[i * tsizew + j] ,ret[x * width + y]) ;
    }
    printf("\n"); 
  }
  int index, flag;

  flag =0;
  for(int i = ah,x=h ; i<asizeh; i++, x= 0){
    for(int j = aw, y = w; j<asizew; j++, y=0){
      index = x * width + y;
      if(temp[i * tsizew + j] && process_temp(i,j,tsizew,metodo,temp)){
        aux[index] = 0;
        flag=1;
        
      } 
    }
  }
  

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
  int i, j, alt, flag, nbw, nbwr, bw, nbh, nbhr, bh;

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
    for(alt=0; alt < 2; alt++){
      //#pragma omp parallel for collapse(2) private(index) num_threads(32)
      for(i =0; i < nbw; i++){
        for(j =0; j < nbh; j++){
          if(i==(nbw-1)){
            if(j==(nbh-1)){
              flag = can_be_removed(i*bw, width, j*bh, height, alt);
            }else{
              flag = can_be_removed(i*bw, width, j*bh, j*bh + bh, alt);
            }
          }else{
            if(j==(nbh-1)){
              flag = can_be_removed(i*bw, i*bw + bw, j*bh, height, alt);
            }else{
              flag = can_be_removed(i*bw, i*bw + bw, j*bh, j*bh +bh, alt);
            }
          }
          /*index = i * width +j;
          if(ret[index] && can_be_removed(i,j,alt)){
            aux[index] = 0;

            flag = 1;
          }*/  
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
