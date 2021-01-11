/*Forest Fire*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "neillsimplescreen.h"

#define G 250
#define L 10 * G
#define NUMROWS 30
#define NUMCOLS 80
#define MAXGEN 1000
#define SCREENWAIT 0.1

enum state {removed, living, flaming};
typedef enum state state;

enum bool {false, true};
typedef enum bool bool;

struct tree{
   state status;
   int row;
   int col;
};
typedef struct tree tree;

struct forest{
   tree tree[NUMROWS][NUMCOLS];
   int rows;
   int cols;
   char strVersion[NUMROWS][NUMCOLS + 1];
};
typedef struct forest forest;

/*Creates a new forest:
1. Living -> flaming if shouldIgnite is true.
2. Flaming -> removed trees.
3. Removed -> living if shouldGrow is true.*/
forest newGeneration(forest f);

/*Initializes a forest and populates a grid rows x col
with trees of a status removed.*/
forest createForest(int rows, int cols);

/*Creates an array of strings representing forest rows.*/
void stringifyForest(forest *f);

/*Initializes a tree an sets its coordinates*/
tree createTree(int row, int col, state s);

/*Returns true if a tree should ignite any neighoring trees
have status flaming or if isLightning is true*/
bool shouldIgnite(forest f, tree t);

/*Returns true if lightning should strike (1 in L chance)*/
bool isLightning();

/*Returns true if a tree should grow (1 in G chance)*/
bool shouldGrow();

/*Gets a color based on char c:
'@' -> green, '*' -> red, default -> white. */
neillcol getCharColor(char c);

/*Generates a min value for looping around a
tree's neighbors to prevent going out of bounds*/
int minNeighborLoopVal(int val, int min);

/*Generates a max value for looping around a
tree's neighbors to prevent going out of bounds*/
int maxNeighborLoopVal(int val, int max);

/*Generates a random numbers betweeen 1 and rate*/
int generateRandom(int rate);

/*Runs through functions to verify correct behavior*/
void test();

int main(void){
   int gen, i, j;
   forest f;
   neillcol color;
   srand(time(NULL));
   test();
   f = createForest(NUMROWS, NUMCOLS);
   neillclrscrn();
   for (gen = 0; gen <  MAXGEN; gen++){
      neillcursorhome();
      for (i = 0; i < NUMROWS; i++){
         /*don't print the null char*/
         for (j = 0; j < NUMCOLS; j++){
            color = getCharColor(f.strVersion[i][j]);
            neillfgcol(color);
            printf("%c",f.strVersion[i][j]);
         }
      }
      neillbusywait(SCREENWAIT);
      f = newGeneration(f);
   }
   neillfgcol(white);
   return 0;
}

void test(){
   int i, j, g, h, counter, fireCount;
   forest f;
   tree *t1, *t2;
   bool pass = false;

   for (i = 0; i < 100000; i++){
      assert(generateRandom(L) <= L);
      assert(generateRandom(G) <= G);
      assert(generateRandom(L) >= 0);
      assert(generateRandom(G) >= 0);
   }
   for (i = 0; i < 10000; i++){
      if (shouldGrow(generateRandom(G)) == true){
         pass = true;
      }
   }
   assert(pass == true);
   pass = false;
   for (i = 0; i < 1000000; i++){
      if (isLightning(generateRandom(L)) == true){
         pass = true;
      }
   }
   assert(pass == true);

   /*forest and tree population testing*/
   f = createForest(3, 3);
   t1 = &f.tree[1][1];
   t2 = &f.tree[0][0];
   assert(t1->row == 1);
   assert(t1->col == 1);
   assert(t2->row == 0);
   assert(t2->col ==0);

   assert(minNeighborLoopVal(0, 0) == 0);
   assert(minNeighborLoopVal(1, 0) == 0);
   assert(minNeighborLoopVal(2, 0) == 1);
   assert(maxNeighborLoopVal(3, 3) == 2);
   assert(maxNeighborLoopVal(2, 3) == 2);
   assert(maxNeighborLoopVal(1, 3) == 2);

   /*confirms should ignite works*/
   t2->status = flaming;
   t1->status = living;
   assert(shouldIgnite(f, *t2) == false);
   assert(minNeighborLoopVal(1, 0) == 0);
   assert(maxNeighborLoopVal(1, f.rows) == 2);
   assert(maxNeighborLoopVal(1, f.cols) == 2);
   assert(shouldIgnite(f, *t1) == true);
   assert(shouldIgnite(f, f.tree[0][2]) == false);

   stringifyForest(&f);
   assert(strcmp("*  ", f.strVersion[0]) == 0);
   assert(strcmp(" @ ", f.strVersion[1]) == 0);
   assert(strcmp("   ", f.strVersion[2]) == 0);

   f = newGeneration(f);
   assert(f.tree[0][0].status == removed);
   assert(f.strVersion[0][0] == ' ');
   assert(f.tree[1][1].status == flaming);
   assert(f.strVersion[1][1] == '*');
   assert(getCharColor('@') == green);
   assert(getCharColor('t') == white);
   assert(getCharColor(f.strVersion[1][1]) == red);

   /*test if new trees generate*/
   counter = 0;
   for (g = 0; g < 1000; g++){
      f = newGeneration(f);
      for (i = 0; i < f.rows; i++){
         for (j = 0; j < f.cols; j++){
            if (f.tree[i][j].status == living){
               counter++;
            }
         }
      }
   }
   assert(counter > 10);

   /*test if lightning creates fire*/
   f = createForest(3, 3);
   counter = 0;
   for (g = 0; g < 10000; g++){
      f = newGeneration(f);
      for (i = 0; i < f.rows; i++){
         for (j = 0; j < f.cols; j++){
            if (f.tree[i][j].status == flaming){
               counter++;
               f = createForest(3, 3);
            }
         }
      }
   }
   assert(counter > 1);

   /*test fire spreads*/
   f = createForest(3, 3);
   for (h = 0; h < 10; h++){
      fireCount = 0;
      for (g = 0; g < 10000; g++){
         f = newGeneration(f);
         for (i =0; i < f.rows; i++){
            for (j = 0; j < f.cols; j++){
               if (f.tree[i][j].status == flaming){
                  fireCount++;
               }
            }
         }
      }
      /*always (almost) greater than lightning alone*/
      assert(fireCount > counter);
   }
}

forest newGeneration(forest f){
   forest newF;
   bool ignite;
   int i, j;
   newF = createForest(f.rows, f.cols);
   for (i = 0; i < newF.rows; i++){
      for (j = 0; j < newF.cols; j++){
         if (f.tree[i][j].status == removed
            && shouldGrow() == true){
            newF.tree[i][j].status = living;
         }
         ignite = shouldIgnite(f, f.tree[i][j]);
         if (f.tree[i][j].status == living
            && ignite == true){
            newF.tree[i][j].status = flaming;
         }
         if (f.tree[i][j].status == living
            && ignite == false){
            newF.tree[i][j].status = living;
         }
         if (f.tree[i][j].status == flaming){
            newF.tree[i][j].status = removed;
         }
      }
   }
   stringifyForest(&newF);
   return newF;
}

forest createForest(int rows, int columns){
   int i, j;
   forest f;
   f.rows = rows;
   f.cols = columns;
   for (i = 0; i < f.rows; i++){
      for (j = 0; j < f.cols; j++){
         f.tree[i][j] = createTree(i, j, removed);
      }
   }
   stringifyForest(&f);
   return f;
}

void stringifyForest(forest *f){
   int i, j;
   for (i = 0; i < f->rows; i++){
      for (j = 0; j < f->cols; j++){
         if(f->tree[i][j].status == removed){
            f->strVersion[i][j] = ' ';
         }
         if(f->tree[i][j].status == living){
            f->strVersion[i][j] = '@';
         }
         if(f->tree[i][j].status == flaming){
            f->strVersion[i][j] = '*';
         }
      }
      f->strVersion[i][j] = '\0';
   }
}

tree createTree(int row, int col, state s){
   tree t;
   t.row = row;
   t.col = col;
   t.status = s;
   return t;
}

bool shouldIgnite(forest f, tree t){
   int minRow, maxRow, minCol, maxCol;
   int i, j;
   if(t.status == flaming || t.status == removed){
      return false;
   }
   if (isLightning() == true){
      return true;
   }
   minRow = minNeighborLoopVal(t.row, 0);
   maxRow = maxNeighborLoopVal(t.row, f.rows);
   minCol = minNeighborLoopVal(t.col, 0);
   maxCol = maxNeighborLoopVal(t.col, f.cols);
   for(i = minRow; i <= maxRow; i++){
      for (j = minCol; j <= maxCol; j++){
         if (f.tree[i][j].status == flaming){
            return true;
         }
      }
   }
   return false;
}

bool isLightning(){
   if (generateRandom(L) == 0){
      return true;
   }
   return false;
}

bool shouldGrow(){
   if (generateRandom(G) == 0){
      return true;
   }
   return false;
}

neillcol getCharColor(char c){
   neillcol color;
   switch(c){
      case '@':
         color = green;
         return color;
      case '*':
         color = red;
         return color;
      default:
         color = white;
         return color;
   }
}

int minNeighborLoopVal(int n, int min){
   if (n - 1 <= min){
      return min;
   }
   return n - 1;
}

int maxNeighborLoopVal(int n, int max){
   if (n + 1 >= max){
      return max - 1;
   }
   return n + 1;
}

int generateRandom(int rate){
   int n;
   n = rand() % (rate);
   return n;
}
