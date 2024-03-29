/*Lance-Williams Algorithm for Agglomerative Clustering
  COMP2521 2019 T1
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "LanceWilliamsHAC.h"
#include "Graph.h"
#define numVertices numVerticies

//Struct to hold the index pairs with minimum distances
typedef struct indexpair{
  int index1;
  int index2;
}indexpair;

//Static functions
static double edgeWeightvalue(Graph g,int row,int col);
static indexpair getVerticeWithMinDist(int size,double **dist);
static Dendrogram *updateDendogram(int size,Dendrogram *dendRep,indexpair vp);
static double **updateDist(int size,int method,double **dist,indexpair ip);
static Dendrogram newDendogram(int vertex);
static double** newDistArray(Graph g);

/* 
 * Finds Dendrogram using Lance-Williams algorithm (as discussed in the specs) 
   for the given graph g and the specified method for agglomerative clustering.
   Method value 1 represents 'Single linkage', and 2 represents 'Complete linkage'.
   For this assignment, you only need to implement the above two methods. 
   The function returns 'Dendrogram' structure (binary tree) with the required information.
 * 
 */
Dendrogram LanceWilliamsHAC(Graph g, int method) {
  //Intialising a matrix of doubles to store the distance
  double **dist = newDistArray(g);

  //Initiliasing the Dendogram array of structs
  Dendrogram *dendRep = malloc(sizeof(DNode)*numVertices(g));
  for(int i = 0; i < numVertices(g) ;i++){
    dendRep[i] = newDendogram(i);
  }

  //Iterating number of times the clusters need to be merged 
  //since every iteration 2 cluster get merged, we have as many iterations as the vertices in graph
  for(int count = 0; count < numVertices(g) ;count++){
    indexpair ip;
    //getting the indexes in the dist matrix with minimum distance
    ip = getVerticeWithMinDist((numVertices(g)-count),dist);
    //updating the array of dendographs by mergeing 2 dendographs each iteration
    dendRep = updateDendogram((numVertices(g)-count),dendRep,ip);
    //updating the distances array
    dist = updateDist((numVertices(g)-count),method,dist,ip);   
  }
  return *dendRep;
}



//Function to derive the distance matrix
static double** newDistArray(Graph g){
  //mallocing a 2-D array of doubles
  double **dist = malloc(sizeof(double *)*numVertices(g));
  for(int i = 0;i<numVertices(g);i++){
    dist[i] = malloc(sizeof(double)*numVertices(g));
  }

  //Intitialising the distance array with the distances between vertices
  for(int row = 0;row<numVertices(g);row++){
    for(int col = 0;col<numVertices(g);col++){
      
      if(row == col){
        //Diagonal in the matrix
        dist[row][col] = 0;
      }else{
         //Alloting distances based on graph g
         //making sure that the distance between the vertices is not inf when there is a edge between them regardless of direction
        if(edgeWeightvalue(g,row,col) != 0){
           //Checking the minimum distance between 2 vertices 
            if( 1/edgeWeightvalue(g,row,col) > 1/edgeWeightvalue(g,col,row) && edgeWeightvalue(g,col,row) != 0){
                dist[row][col] = 1/edgeWeightvalue(g,col,row);
            }else{
               dist[row][col] = 1/edgeWeightvalue(g,row,col);
            } 
        }else{
            dist[row][col] = 1/edgeWeightvalue(g,col,row);
        }
      }
    }
  }
  return dist;
}


//Function to initialise the Dendogram struct 
static Dendrogram newDendogram(int vertex){
  Dendrogram newD = malloc(sizeof(DNode));
  newD->vertex = vertex;
  newD->left = NULL;
  newD->right = NULL;
  return newD;
}

//Function to calculate edge weight between 2 vertices
static double edgeWeightvalue(Graph g,int row,int col){
  for(AdjList temp = outIncident(g,row);temp != NULL;temp = temp->next){
    if(temp->w == col){
      return temp->weight;  
    }
  }
  return 0;
}

//Function to find indexes with minimum distances
static indexpair getVerticeWithMinDist(int size,double **dist){  

  indexpair VP;
  //setting a default value 
  VP.index1 = 0;
  VP.index2 = 1;
  double min = dist[0][1];
  //iterating through the matrix to find minimum
  for(int row = 0;row<size;row++)
  {
    for(int col = 0;col<size;col++)
    {
      //skipping diagnols
      if(row == col)
      {
        continue;
      }
      //updating when a minimum distance found
      if(dist[row][col] < min)
      {
        min = dist[row][col];
        VP.index1 = row;
        VP.index2 = col;
      }
    }
  }
  //ordering the index pair such that the first one is always lesser then second
  if(VP.index1 > VP.index2)
  {
    int t = VP.index1;
    VP.index1 = VP.index2;
    VP.index2 = t;
  }
  return VP;
}

//Function to update the structure of the Dendogram
static Dendrogram *updateDendogram(int size,Dendrogram *dendRep,indexpair vp){
  
  if(size == 1) {
    //Size of Dendogram is 1
    return dendRep;
  }
  //Creating a new dendogram array with size one less then the previous one
  Dendrogram *newDend = malloc(sizeof(DNode)*(size - 1));
  int j = 0;
  //Setting the last node of the dendogram array to be a root node for other clusters
  newDend[size - 2] = newDendogram(-1);
  //Calculating which pair of index appears first
  //iterating through the new dendogram array to update it with accordance to merged clusters
  for(int i = 0;i<(size-1);i++){
    //if j is equal to one of the vertices to be merged ,then adding the new node to heirarchy
    if(j == vp.index1){
      newDend[size-2]->left = dendRep[j];
      j++;
    }
    if(j == vp.index2){
      newDend[size-2]->right = dendRep[j];
      j++;
    }

    //Condition to check the newDend[size-2] always holds the new heirarchy
    if(i != (size-2)){
      newDend[i] = dendRep[j];
    }
    j++;
  }
  return newDend; 
}


//Function to update the dendogram after merging clusters
static double **updateDist(int size,int method,double **dist,indexpair ip){
//returning when only one vertice left
  if(size == 1){
    return 0;
  }
//Constants to be used in formulas based on methods
  double alpha1,alpha2,beta,gamma;
  if(method == 1){
    alpha1 = 0.5;
    alpha2 = 0.5;
    beta = 0;
    gamma = -0.5;
  }else if(method == 2){
    alpha1 = 0.5;
    alpha2 = 0.5;
    beta = 0;
    gamma = 0.5; 
  }
  //mallocing a new dist matrix
  double **newdist = malloc(sizeof(double *)*(size-1));
  for(int i = 0;i<size-1;i++){
    newdist[i] = malloc(sizeof(double)*size-1);
  }
  //updating distances for the merged clusters with rest of clusters
  int default_row = 0;
  for(int d_row = 0;d_row<size-1;d_row++){
    if(default_row == ip.index1 ){
      default_row++;
    }

    if(default_row == ip.index2){
      default_row++;
    }

    if(d_row != size-2){
      //Applying formula
      newdist[d_row][size-2] = alpha1*dist[default_row][ip.index1] + alpha2*dist[default_row][ip.index2] + gamma*fabs(dist[default_row][ip.index1]-dist[default_row][ip.index2]);
    }
    default_row++;
  }
  //making sure that the matrix is always complete
  for(int i = 0;i<size-1;i++){
    newdist[size-2][i] = newdist[i][size-2];
  }
  newdist[size - 2][size - 2] = 0;
  int o_row = 0;
  // filling the remaining vertices in the new dist by copying from the old dist
  for(int row = 0;row<size-1;row++){
    if(o_row ==ip.index1){
      o_row++;
    }
    if(o_row ==ip.index2){
      o_row++;
    }
    int o_col = 0;
    for(int col = 0;col<size-1;col++){
      if(row == col){
        o_col++;
        newdist[row][col] = 0;
        continue;
      }

      if(o_col == ip.index1){
        o_col++;
      }  

      if(o_col == ip.index2){
        o_col++;
      }
          //Updating when a minimum distance found
      if(row != size-2 && col != size-2){
        newdist[row][col] = dist[o_row][o_col];
      }
      o_col++;
    }
    o_row++;
  }
  //freeing the old dist
  for(int i = 0;i<size;i++)
  {
    free(dist[i]);
  }
  free(dist);
  return newdist ;
}
//function to free the dendogram
void freeDendrogram(Dendrogram d) {
  if(d->left != NULL)
  {
    freeDendrogram(d->left);
  }
  else if(d->right != NULL)
  {
    freeDendrogram(d->right);
  }
  free(d);
  return;
}