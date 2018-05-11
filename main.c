#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include "libfractal/fractal.h"
#include "queue.h"

struct fractal* toCompute = NULL ;
int ligne ;
int colonne ;
int readFinish = 0 ; 
int numberOfFile ;


char** filenames ; // enregistrer le nom des fichiers
char* fileOutName ;

pthread_mutex_t mutexRead; //mutex pour proteger la structure lors de la lecture des fichiers
pthread_mutex_t bestAverage; 
pthread_mutex_t fractalProt ;
pthread_mutex_t computeValue ;
queue* listeFractal ;
queue* bestAverageList;

sem_t empty; // semaphore pour la lecture des fichiers
sem_t full; // pour faire attendre le consommateur
int readstdin = 0 ;
int drawAll = 0; // 1 if true, 0 if draw just the best average
static double maxAverage = 0; // Remember the maximum computed average

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
   char* word = malloc(sizeof(char)*64) ;
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
   *(word + i - 1) = '\0' ;
   return word ;
}


void* fileReading(void* pos){
   int * num ;
   num = (int*) pos ;
   char * str = filenames[*num] ;
   FILE* fDes = fopen(str,"r") ;
   if(fDes == NULL){
      perror("impossible de lire le fichier \n") ;
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
            struct fractal* fract = fractal_new(name, width, height, arg1, arg2) ;
            sem_wait(&empty);
            pthread_mutex_lock(&mutexRead) ;
            if(checkName(listeFractal,name) == 0){
            int err = enqueue(fract,listeFractal) ;
            if(err != 0){
               exit(EXIT_FAILURE);
            }
            }
            pthread_mutex_unlock(&mutexRead);
            sem_post(&full);
            
            //printf("nos valeurs :\n %s \n %d \n %d \n %f \n %f \n",name,width,height,arg1,arg2);
            free(heightc) ;
            free(widthc) ;
            free(arg1c) ;
            free(arg2c) ;
            
         }
         else{
            if(*name == '#'){
               newLine(fDes) ;
            }
         }
      }
fclose(fDes) ;
readFinish++ ;
pthread_exit(NULL) ;
}

void* stdinReading(){
         char* name = getWord(stdin) ;
         if(*name != '#' && *name != '\0'){
            char* widthc = getWord(stdin) ;
            char* heightc = getWord(stdin);
            char* arg1c = getWord(stdin) ;
            char* arg2c = getWord(stdin) ;     
            int width = atoi(widthc) ;
            int height = atoi(heightc);
            double arg1 = atof(arg1c) ;
            double arg2 = atof(arg2c) ;
            struct fractal* fract = fractal_new(name, width, height, arg1, arg2) ;
            sem_wait(&empty);
            pthread_mutex_lock(&mutexRead) ;
            if(checkName(listeFractal,name) == 0){
            int err = enqueue(fract,listeFractal) ;
            if(err != 0){
               perror("Could not enqueue fractal \n");
               exit(EXIT_FAILURE);
            }
          
            }
            pthread_mutex_unlock(&mutexRead) ;
            sem_post(&full);
            //printf("nos valeurs :\n %s \n %d \n %d \n %f \n %f \n",name,width,height,arg1,arg2);
            free(heightc) ;
            free(widthc) ;
            free(arg1c) ;
            free(arg2c); 
            
         }
         else{
            if(*name == '#'){
               newLine(stdin) ;
            }
         }
fclose(stdin) ;
readFinish++ ;
pthread_exit(NULL) ;
}

// Consumer
void *computeValueThreadFunction(void *n){
int count = 0 ;
    while(1){
        count++ ;
        sem_wait(&full);
        pthread_mutex_lock(&fractalProt);
        // Section critique
        struct fractal *toComputeFractal = dequeue(listeFractal);
        pthread_mutex_unlock(&fractalProt);
        sem_post(&empty);
        // Problem allocating memory
        if(toComputeFractal == NULL){
            fractal_free(toComputeFractal);
            break;
        }
        // Compute the average of the fractal
        double* average = fractalAverage(toComputeFractal);
        

        if(drawAll == 0){ // We draw only the best average fractal
            //Section critique
            pthread_mutex_lock(&bestAverage);
            if(*average > maxAverage){
                free(bestAverageList);
                maxAverage = *average;
                bestAverageList = initQueue();
                enqueue(toComputeFractal,bestAverageList);
            }
            else if(*average == maxAverage){
                enqueue(toComputeFractal, bestAverageList);
            }
            else{
                fractal_free(toComputeFractal); // Nothing to do we simply throw the fractal
            }
            pthread_mutex_unlock(&bestAverage);
	    }
        else{ // We draw every fractal
            int sizeOfResult = 65;
	    char *name = (char*)malloc(sizeof(char)*(sizeOfResult+5));
	    strncpy(name,fractal_get_name(toComputeFractal),65);
	    write_bitmap_sdl(toComputeFractal, strcat(name,".bmp"));
	    free(name);
        }
        free(average) ;
    }
    pthread_exit(NULL) ;
}





int main(int argc, char *argv[])
{
    int maxThreads = 4;

    // Initialisation queues;
    listeFractal = initQueue();
    bestAverageList = initQueue();
    if(bestAverageList == NULL){
        perror("Error memory allocation");
        freeQueue(listeFractal);
        freeQueue(bestAverageList);
        return 1;
    }

    // Initialisation resources
    int err =pthread_mutex_init(&mutexRead, NULL);
    if(err!=0){
        perror("Could not initiate mutex \n");
        exit(EXIT_FAILURE);
     }
    err = pthread_mutex_init(&bestAverage, NULL);
    if(err!=0){
        perror("Could not initiate mutex \n");
        exit(EXIT_FAILURE);
     }
     err = pthread_mutex_init(&computeValue, NULL);
    if(err!=0){
        perror("Could not initiate mutex \n");
        exit(EXIT_FAILURE);
     }
     err = pthread_mutex_init(&fractalProt, NULL);
    if(err!=0){
        perror("Could not initiate mutex \n");
        exit(EXIT_FAILURE);
     }
     sem_init(&empty, 0, maxThreads);
     sem_init(&full, 0, 0);
    
    int count  = 1;
    for(int i = 1; i < argc; i++){
       if(strcmp(argv[i],"-d") == 0){
          // dessiner tout les fichiers a la fin
          drawAll = 1 ;
          count++ ;
       }
       else if(strcmp(argv[i],"--maxthreads") == 0){
          // nombre de threads de calcul max
          maxThreads = atoi(argv[i + 1]) ;
          i++ ;
          count++ ;
          count++ ;
       }
       else{
          break ;
	}
       
    }

    // Threads
    pthread_t threadRead[argc - count]; // thread de lecture
    pthread_t threadCompute[maxThreads]; // thread for calculations--
    

    filenames = malloc(sizeof(char)*64) ;
    int value[argc-count -1] ;
    numberOfFile = argc -count -1 ;
    

    int toPlace = 0 ;
    for(toPlace =0; toPlace < argc -count -1 ; toPlace++){
       if(strcmp(argv[toPlace+count],"-") == 0){
          //Lire l'entree standard
          readstdin = 1 ;
          printf("Entrez une fractal");
       }
       else{
       value[toPlace] = toPlace ; // car on ne peut passer i dans thread_create puisqu'il change sans arret
       filenames[toPlace] = malloc(sizeof(char)*64) ;
       filenames[toPlace] = argv[toPlace+count] ;
       err=pthread_create(&threadRead[toPlace],NULL,&fileReading, (void*) &value[toPlace]);
       if(err!=0) {
	  perror("Could not create thread \n");
	  exit(EXIT_FAILURE);
	}
       }
    }
	
    if(readstdin == 1){
       err=pthread_create(&threadRead[toPlace-1],NULL,&stdinReading, NULL);
       if(err!=0) {
	  perror("Could not create read thread \n");
	  exit(EXIT_FAILURE);
	}
    }

    // Creating threads computation
    for(int j=0; j<maxThreads; j++){
        int error = pthread_create(&(threadCompute[j]), NULL, &computeValueThreadFunction, NULL);
        if(error != 0){
            perror("Could not create compute thread \n");
        }
    }
    
    fileOutName = argv[argc - 1] ;
			
    for(int i =0; i < argc - count -1 ; i++){
    err = pthread_join(threadRead[i],NULL) ;
       if(err != 0){
          perror("Could not join read thread \n");
			exit(EXIT_FAILURE);
       }
    }
    for(int k = 0 ; k < maxThreads ; k++){
       sem_post(&full) ;
    }

    // Joining threads
    for (int i = 0; i < maxThreads; i++) {
        pthread_join(threadCompute[i], NULL);
    }
	
    // Drawing the fractals
    char numberDraw = 'a' ;
    char* toAdd = malloc(sizeof(char)*2);
    *toAdd = numberDraw ;
    *(toAdd + 1) = '\0' ;
    if(drawAll == 0){
        while(!isQueueEmpty(bestAverageList)){
            struct fractal *toDrawFractal = dequeue(bestAverageList);
            if(toDrawFractal == NULL){
               break ;
            }
            int sizeOfResult = strlen(argv[argc-1]);
            char *name = (char*)malloc(sizeof(char)*(sizeOfResult+5));
            strncpy(name,argv[argc-1],sizeOfResult+1);
            if(numberDraw != 'a'){
               *toAdd = numberDraw ;
               *(toAdd + 1) = '\0' ;
               name = strcat(name,toAdd) ;
            }
            write_bitmap_sdl(toDrawFractal,strcat(name, ".bmp"));
            free(name);
            numberDraw++ ;
        }
    }
    free(toAdd) ;
	
    // Free memory
    freeQueue(listeFractal);
    freeQueue(bestAverageList);
	
    // Destroying resources
    pthread_mutex_destroy(&mutexRead);
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&bestAverage);
    return 0;
}
