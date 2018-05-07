#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "fractal.h"

int main()
{
    void* fileReading(char* filename){
       fDes = open(filename,O_RDONLY) ;
       if(fDes < 0){
          perror("impssible de lire le fichier") ;
          close(fDes) ;
          exit(EXIT_FAILURE) ;
       }
       while(!feof(fDes)){
          
       }
    }
    return 0;

}
