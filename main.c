#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
//#include "fractal.h"


void newLine(FILE* fDes){
   int k ;
   char c ;
   char* bufc = &c ;
   do{
      k = fread(bufc,sizeof(char),1,fDes) ;
      if(k == 0){
         return ;
      }
   }while(c != '\n') ;
}
   
char* getWord(FILE* fDes){
   char* word = malloc(sizeof(char)*64);
   char c ;
   char* bufc = &c;
   int i = 0 ;
   int k ;
   do{
      if(i > 0){
         *(word + i - 1) = c; 
      }   
      k = fread(bufc,sizeof(char),1,fDes) ;
      if(k == 0){
         word = '\0' ;
         return word;
      }
      i++ ;
   }while(c != ' ' && c != '\n' && c != '\0' && k > 0) ;
   *(word + i) = '\0' ;
   return word ;
}


void fileReading(char* filename){
   FILE* fDes = fopen(filename,"r") ;
   if(fDes < 0 || &fDes == NULL){
      perror("impossible de lire le fichier \n") ;
      fclose(fDes) ;
      exit(EXIT_FAILURE) ;
   }
      while(1){
         char* name = getWord(fDes) ;
         if(feof(fDes)){
            break ;
         }
         if(*name != '#' && *name != '\0'){
            char* widthc = getWord(fDes) ;
            char* heightc = getWord(fDes);
            char* arg1c = getWord(fDes) ;
            char* arg2c = getWord(fDes) ;     
            int width = atoi(widthc) ;  // ajouter sécurité en cas de mauvais argument
            int height = atoi(heightc);
            double arg1 = atof(arg1c) ;
            double arg2 = atof(arg2c) ;
            printf("nos valeurs :\n %s \n %d \n %d \n %f \n %f \n",name,width,height,arg1,arg2);
         }
         else{
            if(*name == '#'){
               newLine(fDes) ;
            }
         }
      }
}

int main(int argc, char *argv[])
{
    fileReading(argv[1]) ;
    return 0;
}
