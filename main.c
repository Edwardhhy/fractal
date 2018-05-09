#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
//#include "fractal.h"
//#include "queue.h"

char** filenames ; // enregistrer le nom des fichiers

pthread_mutex_t mutexRead; //mutex pour proteger la structure lors de la lecture des fichiers

sem_t semRead; // semaphore pour la lecture des fichiers

int err ;
int readstdin = 0 ;

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


void* fileReading(void* pos){
   int * num ;
   num = (int*) pos ;
   printf("%d \n",*num);
   char * str = filenames[*num] ;
   printf("debut fileReading %s \n",str);
   FILE* fDes = fopen(str,"r") ;
   if(fDes == NULL){
      perror("impossible de lire le fichier \n") ;
exit(0) ;
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
            int width = atoi(widthc) ;
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
pthread_exit(NULL) ;
}

void* stdinReading(){
   printf("debut fileReading stdin \n"); 
      while(1){
         char* name = getWord(stdin) ;
         if(feof(stdin)){
            break ;
         }
         if(*name != '#' && *name != '\0'){
            char* widthc = getWord(stdin) ;
            char* heightc = getWord(stdin);
            char* arg1c = getWord(stdin) ;
            char* arg2c = getWord(stdin) ;     
            int width = atoi(widthc) ;
            int height = atoi(heightc);
            double arg1 = atof(arg1c) ;
            double arg2 = atof(arg2c) ;
            printf("nos valeurs :\n %s \n %d \n %d \n %f \n %f \n",name,width,height,arg1,arg2);
            
         }
         else{
            if(*name == '#'){
               newLine(stdin) ;
            }
         }
      }
pthread_exit(NULL) ;
}

int main(int argc, char *argv[])
{
    int count  = 0;
    for(int i = 1; i < argc; i++){
    printf("On est rentré : %s \n",argv[i]) ;
       if(strcmp(argv[i],"-d") == 0){
          // dessiner tout les fichiers a la fin
          count++ ;
       }
       else if(strcmp(argv[i],"2") == 0){
          // nombre de threads de calcul max
          count++ ;
       }
       else if(strcmp(argv[i],"-") == 0){
          //Lire l'entree standard
          printf("Lecture entrée standart demandé \n");
          readstdin = 1 ;
       }
       else{
          break ;
	}
       
    }
    pthread_t threadRead[argc - count] ; //thread de lecture
    filenames = malloc(sizeof(char)*64) ;
    int value[argc-count] ;
    
    if(readstdin == 1){
       err=pthread_create(&threadRead[0],NULL,&stdinReading, NULL);
       printf("Thread stdin créé \n") ;
       if(err!=0) {
	  printf("%s\n", "Could not create thread");
	  exit(EXIT_FAILURE);
	}
    }
    
    printf("avant la boucle for  \n") ;
    for(int i =count + 1; i < argc ; i++){
       printf("i vaut %d \n",i);
       if(i == count + 1 && readstdin == 1){
          i++ ;
          printf("On augmente i \n") ;
       }
       value[i - count-1] = i - count -1 ; // car on ne peut passer i dans thread_create puisqu'il change sans arret
       filenames[i - count - 1] = malloc(sizeof(char)*64) ;
       filenames[i - count - 1] = argv[i] ;
       err=pthread_create(&threadRead[i-count-1],NULL,&fileReading, (void*) &value[i - count - 1]);
       printf("Thread créé \n") ;
       if(err!=0) {
	  printf("%s\n", "Could not create thread");
	  exit(EXIT_FAILURE);
	}
    }
    
			
    printf("avant join \n") ;
    for(int i =0; i < argc - count -1 ; i++){
    printf("vraiment avant join %d \n",i) ;
    err = pthread_join(threadRead[i],NULL) ;
       if(err != 0){
          printf("%s\n", "Could not join thread");
			exit(EXIT_FAILURE);
       }
    }
    return 0;
}
