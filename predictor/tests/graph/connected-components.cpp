// Author: Nilay Vaish <nilay@cs.wisc.edu>


#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "m5op.h"

using namespace std;

void ReadInputFile(char *file_name, int& n, int& m,
                   vector<vector<int>>& Neighbors)
{
  FILE *fp = fopen(file_name, "r");

  // Read the number of nodes and edges in the graph.
  fscanf(fp, "%d %d\n", &n, &m);
  Neighbors.resize(n+1);

  // Loop for reading the edges.  Each line contains information about one
  // vertex.  Note that vertices are numubered starting from 1.
  for (int i = 1; i <= n; ++i)
  {
    char *line = nullptr;
    size_t line_size = 0;
    int bytes = getline(&line, &line_size, fp);

    // Tokenize the string obtained.
    char * pch = strtok(line," \n");
    while (pch != nullptr)
    {
      int v = atoi(pch);
      Neighbors[i].push_back(v);
      //printf ("%d ", v);
      pch = strtok (nullptr, " \n");
    }
    //printf("\n");
    free(line);
  }
}

void CountConnectedComponents(int n, const vector<int>& CCid)
{
  // Count the number of connected components.
  vector<bool> Flag(n+1, false);
  for (int i = 1; i <= n; ++i) Flag[CCid[i]] = true;

  int count = 0;
  for (int i = 1; i <= n; ++i) if (Flag[i]) count += 1;
  printf("Number of Connected Components: %d\n", count);
}

void ConnectedComponents1(const vector<vector<int>>& Neighbors,
                          int n, vector<int>& CCid)
{
  for (int i = 1; i <= n; ++i) CCid[i] = i;
  int change = 1;

  while (change)
  {
    change = 0;
    for (int v = 1; v <= n; ++v)
    {
      int cv = CCid[v];
      for (auto u: Neighbors[v])
      {
        int cu = CCid[u];
        if (cu < cv)
        {
          CCid[v] = cu;
          change = 1;
        }
      }
    }
  }
}

void compare(int a, int& b)
{
	asm("cmp %1,%0 ; cmovnae %1,%0"
		:"=r" (b)
		: "r" (a), "r" (b));
}

void ConnectedComponents2(const vector<vector<int>>& Neighbors,
                          int n, vector<int>& CCid)
{
  for (int i = 1; i <= n; ++i) CCid[i] = i;
  int change = 1;

  while (change)
  {
    change = 0;
    for (int v = 1; v <= n; ++v)
    {
      int& cv = CCid[v];
      int cv_init = cv;
      for (auto u: Neighbors[v])
      {
        int cu = CCid[u];
		compare(cu, cv);
      }

      change = change | (cv ^ cv_init);
    }
  }
}

int main(int argc, char **argv)
{
  int n, m; vector<vector<int> > Neighbors;

  bool with_gem5 = false;
  bool with_cmov = false;

  if (strcmp(argv[1], "--with-gem5") == 0)
    with_gem5 = true;
  if (strcmp(argv[2], "--with-cmov") == 0)
    with_cmov = true;

  // Read the graph provided in the input file.
  ReadInputFile(argv[3], n, m, Neighbors);

  // Vector for holding the component ids.
  vector<int> CCid(n+1);
  // Find the connected components.
  if (with_gem5)
    m5_dumpreset_stats(0, 0);

  if (with_cmov)
    ConnectedComponents2(Neighbors, n, CCid);
  else
    ConnectedComponents1(Neighbors, n, CCid);

  if (with_gem5)
    m5_dumpreset_stats(0, 0);
  // Count the number of connected components.
  CountConnectedComponents(n, CCid);
  return 0;
}
