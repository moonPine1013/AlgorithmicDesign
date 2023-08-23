/*
    Written by William Sutherland for 
    COMP20007 Assignment 1 2023 Semester 1
    Modified by Grady Fitzpatrick
    
    Implementation for module which contains map-related 
        data structures and functions.

    Functions in the task description to implement can
        be found here.
    
    Code implemented by <YOU>
*/

#include "map.h"
#include "stack.h"
#include "pq.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* Additional */
#include <limits.h>
#include <math.h>

#define MAX_MAP_POINTS 6
#define VISITED 1
#define UNVISITED 0
#define ON_MAP 1
#define NOT_ON_MAP 0
#define LO_BOUND_T 0
#define HI_BOUND_T 100
#define AIRPORT 100
#define INVALID_P -1
#define INI_VALUE 1
#define INI_COUNT 0
#define INI_WEIGHT 0

struct map *newMap(int height, int width) {
    struct map *m = (struct map *) malloc(sizeof(struct map));
    assert(m);
    m->height = height;
    m->width = width;

    // Note this means all values of map are 0
    int *points = (int *) calloc(height * width, sizeof(int));
    assert(points);
    m->points = (int **) malloc(width * sizeof(int *));
    assert(m->points);

    for (int i = 0; i < width; i++){
        /* Re-use sections of the memory we 
            allocated. */
        m->points[i] = points + i * height;
    }

    return m;
}

struct point *newPoint(int x, int y) {
    struct point *p = (struct point *) malloc(sizeof(struct point));
    assert(p);
    p->x = x;
    p->y = y;

    return p;
}

void freeMap(struct map *m) {
    /* We only allocate one pointer with the rest
    of the pointers in m->points pointing inside the
    allocated memory. */
    free(m->points[0]);
    free(m->points);
    free(m);
}

void printMap(struct map *m) {
    /* Print half of each row each time so we mirror the hexagonal layout. */
    int printRows = 2 * m->height;
    if(m->width < 2){
        /* If the width is less than 2, simply a vertical column so no need to print 
            last row as it will be empty. */
        printRows -= 1;
    }
    for (int i = 0; i < printRows; i++) {
        for (int j = i % 2; j < m->width; j += 2) {
            if (j == 1){
                /* For odd row, add a spacer in to place the first value after the 0th hex
                    in the printout. */
                printf("       ");
            }
            /* Default to even. Select every second column. */
            int yPos = i / 2;
            if(j % 2 != 0){
                /* If odd, numbering along height is reversed. */
                yPos = m->height - 1 - yPos;
            }
            int val = m->points[j][yPos];

            /* Print value appropriately. */
            if (val < 0){
                printf("S%.3d", val % 1000);
            } else if (val == 0){
                printf("  L  ");
            } else if (val == 100){
                printf("  A  ");
            } else {
                printf("T+%.3d ", val % 1000);
            }
            printf("          ");
        }

        printf("\n");
    }
}

/* IMPLEMENT PART A HERE */
/* Note that for the implementation in this question, you will submit
   an array that contains all the adjacent points and then add an additional
   point at the end of the array with coordinates -1, -1 to signify the
   termination of the array (similar to '\0' terminating a string) */
struct point *getAdjacentPoints(struct map *m, struct point *p) {
    struct point *ans = (struct point *) malloc(sizeof(struct point));
    assert(ans);
    ans->x = -1;
    ans->y = -1;
    
    /* Checking if the inputted point is on the map. */
    if (isOnMap(m, p)) {
        /* Allocate memory for the inputted point and its adjacent points in the array by malloc. */
        struct point *pointsArr = 
            (struct point *) malloc(MAX_MAP_POINTS * sizeof(struct point)); 
        assert(pointsArr);
        
        int numAdjPoints = 0;

        /* Update the adjacent points under the relation to the input.*/
        int adjX[MAX_MAP_POINTS] = {p->x-1, p->x-1, p->x, p->x, p->x+1, p->x+1};
        int adjY[MAX_MAP_POINTS] = {m->height-p->y-1, m->height-p->y, p->y-1,
                                    p->y+1, m->height-p->y-1, m->height-p->y};
        
        for(int i = 0; i < MAX_MAP_POINTS; i++){
            /* Check if the adjacent point is on map. */
            if(isOnMap(m, newPoint(adjX[i], adjY[i]))){
                pointsArr[numAdjPoints] = *newPoint(adjX[i], adjY[i]);
                numAdjPoints++;
            }
        }

        /* Reallocate memory to make the array the correct size */
        pointsArr = (struct point *) realloc(pointsArr,
                            numAdjPoints * sizeof(struct point));
        assert(pointsArr);
        /* Add a sentinel point with coordinates -1,-1 to signify the end of the array */
        pointsArr[numAdjPoints] = *ans;
        
        return pointsArr;

    } else {
        /* The inputted point is not on the map */
        return ans;
    }
}

/* Helper function*/
int isOnMap(struct map *m, struct point *p){
    /* point (x,y) is on map if x within [0,width) and y within [0,height)*/
    if((p->x >= 0 &&  p->x < m->width) 
            && (p->y >= 0 && p->y < m->height)){
        return ON_MAP;
    }
    return NOT_ON_MAP;
}


/* IMPLEMENT PART B HERE */
int mapValue(struct map *m) {
    /* Use array to update if the point has been visited. */
    int **visitArr = (int **) malloc(m->width * sizeof(int *));
    assert(visitArr);
    for(int i = 0; i < m->width; i++){
        visitArr[i] = (int *) calloc(m->height, sizeof(int));
        assert(visitArr[i]);
        for(int j = 0; j < m->height; j++){
        /* Initialise as unvisited. */
        visitArr[i][j]=0;
        }
    }

    int totalVal = 0;
    /* Check every point on the map. */
    for(int i = 0; i < m->width; i++){
        for(int j = 0; j < m->height; j++){
            struct point *pCheck = newPoint(i, j);
            if(isOnMap(m, pCheck)){
                /* Compute the value only if the point is unvisited and it is not a sea. */
                if(visitArr[pCheck->x][pCheck->y] != VISITED 
                        && m->points[pCheck->x][pCheck->y] >= LO_BOUND_T){
                    /* Initialise the value and count for the starting point. */
                    int value = INI_VALUE;
                    int count = INI_COUNT;
                    /* Use DFS to traverse every formed island. */
                    findIsland(m, pCheck, &value, &count, visitArr);
                    /* Update the value of points that are on the same island. */
                    value = count * value;
                    totalVal += value;
                }

            }
            free(pCheck);
        }
    }
    for(int i = 0; i < m->width; i++){
        free(visitArr[i]);
    }
    free(visitArr);
    return totalVal;
}

/* Helper function */
/* Implement DFS graph-traversal algorithm */
struct point *findIsland(struct map *m, struct point *p,
        int *value, int *count, int **visited){
    /* Mark the input point as visited */
    visited[p->x][p->y] = VISITED;

    /* Check if this point has treasure */
    if(m->points[p->x][p->y] > 0 && m->points[p->x][p->y] != AIRPORT){
        *value *= m->points[p->x][p->y];
        (*count)++;
    }
    /* Check if the adjacent points of the inputted point have treasure */
        struct point *adjPoints = getAdjacentPoints(m, p);
        /* Use a temporary pointer to iterate through points. */
        struct point *adjPoint = adjPoints;
        while (adjPoint->x != INVALID_P) {
            /* Compute the value only if the adjacent point is unvisited and it is not a sea. */
            if(visited[adjPoint->x][adjPoint->y] != VISITED 
                    && m->points[adjPoint->x][adjPoint->y] >= LO_BOUND_T){
                findIsland(m, adjPoint, value, count, visited);
            }
        adjPoint = adjPoint + 1;
        }
        if(adjPoints){
            free(adjPoints);
        }
    return NULL;
}


/* IMPLEMENT PART D HERE */
int minTime(struct map *m, struct point *start, struct point *end){
    /* Make a priority queue to store all the points. */
    struct pq *getPq = createPQ();
    /* Make 2D an array to update the distance from the point to the start point. */
    int **weight = (int **) malloc(m->width * sizeof(int*));
    assert(weight);
    /* Make an 2D array to check if the point has been visited. */
    int **visited = (int **) malloc(m->width * sizeof(int*));
    assert(visited);
    /* Complete the array initialisation. */
    for(int i = 0; i < m->width; i++){
        weight[i] = malloc(m->height * sizeof(int));
        assert(weight[i]);
        visited[i] = malloc(m->height * sizeof(int));
        assert(visited[i]);
        for(int j = 0; j < m->height; j++){
            weight[i][j] = INT_MAX;
            visited[i][j] = UNVISITED;
        }
    }
    /* Initialise the weight of starting point. */
    weight[start->x][start->y] = INI_WEIGHT;

    /* Update each point into the priority queue. */
    for(int i = 0; i < m->width; i++){
        for(int j = 0; j < m->height; j++){
            struct point *storeP = newPoint(i, j);
            insert(getPq, storeP, -weight[i][j]);
        }
    }


    /* Implement dijkstra algorithm to find shortest path. */
    while(!isEmpty(getPq)){
        /* Pull the current point with shortest path out from the priority queue. */
        struct point *currentP = pull(getPq);
        /* Mark the current point as visited. */
        visited[currentP->x][currentP->y] = VISITED;

        /* Check if the point is the airport. */
        if(m->points[currentP->x][currentP->y] == AIRPORT){
            /* Loop through other airports to the current airport point. */
            for(int i = 0; i < m->width; i++){
                for(int j = 0; j < m->height; j++){
                    struct point *otherAirport = newPoint(i, j);
                    if(otherAirport->x != currentP->x){
                        /* Compare the weight for travelling and update the shorter path. */
                        if(weight[otherAirport->x][otherAirport->y]  
                        > weight[currentP->x][currentP->y] 
                        + flyDist(m, currentP, otherAirport)){
                            weight[otherAirport->x][otherAirport->y] 
                            = weight[currentP->x][currentP->y] 
                            + flyDist(m, currentP, otherAirport);
                            /* Update the priority queue. */
                            insert(getPq, otherAirport, 
                            -weight[otherAirport->x][otherAirport->y]);
                        }   
                    }
                }
            }
        }    
                      
        /* Loop through its adjacent points. */
        struct point *adjPoints = getAdjacentPoints(m,currentP);
        /* Use a temporary pointer to iterate through points. */
        struct point *adjPoint = adjPoints;
        while (adjPoint->x != INVALID_P){
            /* Compare the new weight and the old weight. */
                if(weight[adjPoint->x][adjPoint->y] 
                > (weight[currentP->x][currentP->y] 
                + timeTaken(m,adjPoint))){
                    /* Update the weight array. */
                    weight[adjPoint->x][adjPoint->y] 
                    = weight[currentP->x][currentP->y] 
                    + timeTaken(m,currentP);
                    /* Insert into the priority queue. */
                    insert(getPq, adjPoint, 
                    -weight[adjPoint->x][adjPoint->y]);
                }                        
            adjPoint = adjPoint + 1;
        }            
        
    }
    free(getPq);
    return weight[end->x][end->y];
}


/* Helper function */
/* Calculate the time taken to move to another adjacent point */
int timeTaken(struct map *m, struct point *p){
    /* Find the value of the point. */
    int value = m->points[p->x][p->y];
    if(value < 0 ){
        /* From sea to an adjacent point. */
        return 2 + ceil(value * value / 1000.0);
    }else{
        /* From land to an adjacent point. */
        return 5;
    }
}

/* Calculate the time taken from one airport to another. */
int flyDist(struct map *m, struct point *airP_1, struct point *airP_2){
    int dist = pow(abs(airP_1->x - airP_2->x ), 2) - 85;
    return dist < 15 ? 15 : dist;
}


/* IMPLEMENT PART E HERE */
int minTimeDry(struct map *m, struct point *start, 
    struct point *end, struct point *airports, int numAirports){
    /* Make a priority queue to store all the points. */
    struct pq *getPq = createPQ();
    /* Make 2D an array to update the distance from the point to the start point. */
    int **weight = (int **) malloc(m->width * sizeof(int*));
    assert(weight);
    /* Make an 2D array to check if the point has been visited. */
    int **visited = (int **) malloc(m->width * sizeof(int*));
    assert(visited);
    /* Complete the array initialisation. */
    for(int i = 0; i < m->width; i++){
        weight[i] = malloc(m->height * sizeof(int));
        assert(weight[i]);
        visited[i] = malloc(m->height * sizeof(int));
        assert(visited[i]);
        for(int j = 0; j < m->height; j++){
            weight[i][j] = INT_MAX;
            visited[i][j] = UNVISITED;
        }
    }
    /* Initialise the weight of starting point. */
    weight[start->x][start->y] = INI_WEIGHT;

    /* Update each point into the priority queue. */
    for(int i = 0; i < m->width; i++){
        for(int j = 0; j < m->height; j++){
            struct point *storeP = newPoint(i, j);
            insert(getPq, storeP, -weight[i][j]);
        }
    }
    
    /* Implement dijkstra algorithm to find shortest path. */
    while(!isEmpty(getPq)){
        /* Pull the current point with shortest path out from the priority queue. */
        struct point *currentP = pull(getPq);
        /* Mark the current point as visited. */
        visited[currentP->x][currentP->y] = VISITED;

        /* Check if the point is the airport. */
        if(m->points[currentP->x][currentP->y] == AIRPORT){
            /* Loop through other airports from the existing airport array. */
            for(int i = numAirports; i > 1; i--){
                struct point *otherAirport = &airports[i];
                if(otherAirport->x != currentP->x){
                    /* Compare the weight for travelling and update the shorter path. */
                    if(weight[otherAirport->x][otherAirport->y] 
                    > weight[currentP->x][currentP->y] 
                    + flyDist(m, currentP, otherAirport)){
                            weight[otherAirport->x][otherAirport->y] 
                            = weight[currentP->x][currentP->y] 
                            + flyDist(m, currentP, otherAirport);
                            /* Insert into the priority queue. */
                            insert(getPq, otherAirport,
                                -weight[otherAirport->x][otherAirport->y]);
                    }   
                }

            }
        }    
            
        
                                
        /* Loop through its adjacent points. */
        struct point *adjPoints = getAdjacentPoints(m,currentP);
        /* Use a temporary pointer to iterate through points. */
        struct point *adjPoint = adjPoints;
        while (adjPoint->x != INVALID_P){
            
            /* Compare the new weight and the old weight. */
                if(weight[adjPoint->x][adjPoint->y] 
                > (weight[currentP->x][currentP->y] 
                + timeTaken(m,adjPoint))){
                    /* Update the weight array. */
                    weight[adjPoint->x][adjPoint->y] 
                    = weight[currentP->x][currentP->y] 
                    + timeTaken(m,currentP);
                    /* Insert into the priority queue. */
                    insert(getPq, adjPoint, 
                    -weight[adjPoint->x][adjPoint->y]);
                }            
            
            adjPoint = adjPoint + 1;
        }
        
        if(adjPoints){
            free(adjPoints);
        }
    }      

    free(getPq);
    return weight[end->x][end->y];
}
