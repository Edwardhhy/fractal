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

char** filenames ; // enregistrer le nom des fichiers
char* fileOutName ;

pthread_mutex_t mutexRead; //mutex pour proteger la structure lors de la lecture des fichiers
pthread_mutex_t bestAverage; 
queue* listeFractal ;
queue* bestAverageList;

sem_t empty; // semaphore pour la lecture des fichiers
sem_t full; // pour faire attendre le consommateur
int readstdin = 0 ;
int drawAll; // 1 if true, 0 if draw just the best average
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
            struct fractal* fract = fractal_new(name, width, height, arg1, arg2) ;
            sem_wait(&empty);
            pthread_mutex_lock(&mutexRead) ;
            int err = enqueue(fract,listeFractal) ;
            if(err != 0){
               printf("%s\n", "Could not enqueue fractal");
               exit(EXIT_FAILURE);
            }
            pthread_mutex_unlock(&mutexRead);
            sem_post(&full);
            
            printf("nos valeurs :\n %s \n %d \n %d \n %f \n %f \n",name,width,height,arg1,arg2);
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
printf("On quitte la thread %d \n",*num);
pthread_exit(NULL) ;
}

void* stdinReading(){
   printf("debut fileReading stdin \n"); 
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
            int err = enqueue(fract,listeFractal) ;
            if(err != 0){
               printf("%s\n", "Could not enqueue fractal");
               exit(EXIT_FAILURE);
            }
            pthread_mutex_unlock(&mutexRead) ;
            sem_post(&full);
            printf("nos valeurs :\n %s \n %d \n %d \n %f \n %f \n",name,width,height,arg1,arg2);
            
         }
         else{
            if(*name == '#'){
               newLine(stdin) ;
            }
         }
fclose(stdin) ;

pthread_exit(NULL) ;
}

// Consumer
void *computeValueThreadFunction(void *n){
    while(1){
        sem_wait(&full);
        pthread_mutex_lock(&listeFractal);
        // Section critique
        struct fractal *toComputeFractal = dequeue(listeFractal);
        pthread_mutex_unlock(&listeFractal);
        sem_post(&empty);
        // Problem allocating memory
        if(toComputeFractal == NULL){
            free(toComputeFractal);
            break;
        }
        // Compute the average of the fractal
        double average = fractalAverage(toComputeFractal);

        if(drawAll == 0){ // We draw only the best average fractal
            //Section critique
            pthread_mutex_lock(&bestAverage);
            if(average > maxAverage){
                free(bestAverageList);
                maxAverage = average;
                bestAverageList = initQueue;
                enqueue(toComputeFractal,bestAverageList);
            }
            else if(average = maxAverage){
                enqueue(toComputeFractal, bestAverageList);
            }
            pthread_mutex_unlock(&bestAverage);
            else{
                fractal_free(fractal); // Nothing to do we simply throw the fractal
            }
	    }
        else{ // We draw every fractal
        int sizeOfResult = 65;
	    char *name = (char*)malloc(sizeof(char)*(sizeOfResult+5));
	    strncpy(name,fractal_get_name(toComputeFractal),65);
	    write_bitmap_sdl(toComputeFractal, strcat(name,".bmp"));
	    free(name);
        }
    }
}

int main(int argc, char *argv[])
{
    int maxThreads;

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
        printf("%s\n", "Could not initiate mutex");
        exit(EXIT_FAILURE);
     }
    err = pthread_mutex_init(&bestAverage, NULL);
    if(err!=0){
        printf("%s\n", "Could not initiate mutex");
        exit(EXIT_FAILURE);
     }
    sem_init(&empty, 0, maxThreads);
    sem_init(&full, 0, 0);
    
    int count  = 0;
    for(int i = 1; i < argc; i++){
    printf("On est rentré : %s \n",argv[i]) ;
       if(strcmp(argv[i],"-d") == 0){
          // dessiner tout les fichiers a la fin
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
    int value[argc-count] ;
    
    printf("avant la boucle for  \n") ;
    for(int i =count + 1; i < argc -1 ; i++){
       printf("i vaut %d \n",i);
       if(strcmp(argv[i],"-") == 0){
          //Lire l'entree standard
          printf("Lecture entrée standart demandé \n");
          readstdin = 1 ;
       }
       else{
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
    }
	
    if(readstdin == 1){
       err=pthread_create(&threadRead[0],NULL,&stdinReading, NULL);
       printf("Thread stdin créé \n") ;
       if(err!=0) {
	  printf("%s\n", "Could not create thread");
	  exit(EXIT_FAILURE);
	}
    }

    // Creating threads computation
    for(int j=0; j<maxThreads; j++){
        int error = pthread_create(&(threadCompute[j]), NULL, &computeValueThreadFunction, NULL);
        if(error != 0){
            printf("error");
        }
    }
    
    fileOutName = argv[argc - 1] ;
    printf("fichier de sortie : %s \n",fileOutName) ;
			
    printf("avant join \n") ;
    for(int i =0; i < argc - count -1 ; i++){
    printf("vraiment avant join %d \n",i) ;
    err = pthread_join(threadRead[i],NULL) ;
       if(err != 0){
          printf("%s\n", "Could not join thread");
			exit(EXIT_FAILURE);
       }
    }
	
    // Joining threads
    for (int i = 0; i < maxThreads; i++) {
        pthread_join(threadCompute[i], NULL);
    }
	
    // Drawing the fractals
    if(drawAll == 0){
        while(bestAverageList != NULL){
            struct fractal *toDrawFractal = dequeue(bestAverageList);
            int sizeOfResult = strlen(argv[argc-1]);
            char *name = (char*)malloc(sizeof(char)*(sizeOfResult+5));
            strncpy(name,argv[argc-1],sizeOfResult+1);
            write_bitmap_sdl(toDrawFractal,strcat(name, ".bmp"));
                free(name);
        }
    }
	
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
