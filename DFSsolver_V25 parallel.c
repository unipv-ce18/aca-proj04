#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <omp.h>

#define RESET   "\033[0m"
#define RED     "\033[31m"

#define MAXMAZENUM 500

/*
 * This code has been written by Leto II Atreides on Youtube. All credits go to him.
 * I just typed the code from the video and uploaded it to GitHub.
 */


int nMaze;
int flag;
int width, height;
int vettFlag[1000]; /// NOT GOOD, CHANGE IT


int** initMaze(int width, int height){
    int **rawMaze;
    rawMaze = malloc(height * sizeof(int*));
    for (int i=0; i<width; i++){
            rawMaze[i] = malloc(width * sizeof(int));
    }
return rawMaze;
}


// Display the Raw maze as maze.
void showMaze3(int **rawMaze, int width, int height) {
   for(int y = 0; y < height; y++) {
      for(int x = 0; x < width; x++) {
         switch(rawMaze[y][x]) {
         case 1:  printf("[]");  break;
         case 2:  printf(RED "<>" RESET);  break;
         default: printf("  ");  break;
         }
      }
      printf("\n");
   }
   printf("\n");
}



void showRawMaze2(int **rawMaze, int width, int height) {
    for(int a=0; a<height; a++){
            for(int b=0; b<width; b++){
                printf("%d ",rawMaze[a][b]);
            }
            printf("\n");
    }
    printf("\n");
 }


void loadMazeMatrix3(const char *path, int **rawMaze, int width, int height){
    FILE *fd = fopen(path, "r");

    // Check for error
    if( fd==NULL ) {
        perror("Errore in apertura del file");
        exit(1);
    }

    // Maze matrix read element by element
    for(int t=0;t<height;t++){
        for(int j=0;j<width;j++){
            fscanf(fd,"%d", &rawMaze[t][j]);
        }
    }


    fclose(fd); //Close File
}


int raw_dfs(int **rawMaze,int x, int y, int h){
    int count=0;

    if (rawMaze[x+1][y] == 5) {
        vettFlag[h]=0;
        #pragma omp flush(vettFlag)
        #pragma omp critical
        {
            rawMaze[x][y] = 2;
            #pragma omp flush(rawMaze)
        }
        return 1;  // condizioni di uscita
    }



    if (rawMaze[x][y]==0) {
            #pragma omp critical
            {
                rawMaze[x][y] = 2;
                count++;
                #pragma omp flush(rawMaze,count)
            }

                // TASK GIU
                #pragma omp task
                {
                    if(x+1<height && x+1>=0 && vettFlag[h]==1){
                            if(rawMaze[x+1][y]==0){
                                raw_dfs(rawMaze, x+1, y,h);

                            }
                    }
                }

                // TASK DESTRA
                #pragma omp task
                {
                    if(y+1<width && y+1>=0 && vettFlag[h]==1){
                            if(rawMaze[x][y+1]==0){
                                raw_dfs(rawMaze, x, y+1,h);

                            }
                        }
                }

                // TASK SINISTRA
                #pragma omp task
                {
                    if(y-1<width && y-1>=0 && vettFlag[h]==1){
                            if(rawMaze[x][y-1]==0){
                                raw_dfs(rawMaze, x, y-1,h);

                            }
                        }
                }


                 // TASK su
                 #pragma omp task
                {
                    if(x-1<height && x-1>=0 && vettFlag[h]==1){
                            if(rawMaze[x-1][y]==0){
                                raw_dfs(rawMaze, x - 1, y,h);

                            }
                        }
                }



    }

    if(count==0){
        return 0;
    }
	return 1; // se non trovo alcuna soluzione, ritorno 0
}



int isOdd(int dim){
    if ( dim % 2==0){
        dim +=1;
    }
return dim;
}


void solveRawMaze3(int **rawMaze, int width, int height, int h) {
    rawMaze[height - 1][width - 2] = 5;     /// END POINT


    // set start point
    int start_row = 0;
    int start_col = 1;
    flag=1;  //needed to stop task after reach the goal

    omp_set_num_threads(8);
    #pragma omp parallel
    {
        raw_dfs(rawMaze, start_row, start_col, h);
    }

    rawMaze[height - 1][width - 2] = 2;
}


char** listAllMaze(){
    char folder[] = "../data/";
    char** name = malloc(1000 * sizeof(char*));
    for (int i=0; i<MAXMAZENUM; i++){
            name[i] = malloc(40 * sizeof(char));
    }

    nMaze=0;
    DIR *dir = opendir("../data/");
    struct dirent *dent;

    if(dir!=NULL){
            while((dent=readdir(dir))!=NULL){
                if((strcmp(dent->d_name,".")==0 || strcmp(dent->d_name,"..")==0 || (*dent->d_name) == '.' )){
                }else{
                    char temp[100], temp2[100];
                    strcpy(temp, dent->d_name);
                    strcpy(temp2, folder);
                    strcat(temp2, temp);
                    strcpy(name[nMaze], temp2);
                    nMaze++;
                }
            }
    }else{
        closedir(dir);
        printf("EMPTY FOLDER!!!\n");
    }

    closedir(dir);
    return name;
}




int main(int argc, char **argv) {
    static int defaultWidth = 20;   //117 max
    static int defaultHeight = 20;


    if(argc == 2) {
        width = atoi(argv[1]);
		height = width;
    }else if(argc == 3) {
        width = atoi(argv[1]);
		height = atoi(argv[2]);
    }else{
			width=defaultWidth;
			height = defaultHeight;
			printf("No parameters added, default config will be used\n");
    }

    // Controll on width and height
    width = isOdd(width);
    height = isOdd(height);

    char **name;
    name = listAllMaze();
    printf("Will solve %d maze of %d x %d dimension.\n", nMaze, width, height);


    int **rawMaze[nMaze];
    for(int h=0; h<nMaze; h++){
            rawMaze[h] = initMaze(width, height); // pre-allocazione del maze
            loadMazeMatrix3(name[h], rawMaze[h], width, height); // Load all maze matrix
            vettFlag[h]=1;
    }


    omp_set_num_threads(8); /// NOT PUT THIS HERE
    #pragma omp parallel for
        for(int h=0; h<nMaze; h++){
            solveRawMaze3(rawMaze[h], width, height, h);
        }

    for(int h=0; h<nMaze; h++){
            printf("%s\n", name[h]);
            showMaze3(rawMaze[h], width, height);
            free(rawMaze[h]);
    }
	return 0;
}


