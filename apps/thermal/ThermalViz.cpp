#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "graph.h"
#include "program.h"
#include "mesh.h"

using namespace simit;

#ifdef VIZ_MESH

#ifdef __cplusplus
  extern "C" {
#endif
#include "silo.h"
#ifdef __cplusplus
  }
#endif

// Function prototypes
static void DumpDomainToVisit(DBfile *db, int iter, double time, int Xsize, int Ysize, Set *quads, Set *points);


/**********************************************************************/
void DumpToVisit(std::string ZoneName,int iter, double time, int Xsize, int Ysize, Set *quads, Set *points)
{
  char subdirName[32];
  char basename[32];
  DBfile *db;

  sprintf(basename, "%s_plot_c%d", ZoneName.c_str(), iter);
  sprintf(subdirName, "data_%d", 0);

  db = (DBfile*)DBCreate(basename, DB_CLOBBER, DB_LOCAL, NULL, DB_HDF5X);

  if (db) {
     DumpDomainToVisit(db, iter, time, Xsize, Ysize, quads, points);
  }
  else {
     printf("Error writing out viz file \n");
  }

}


/**********************************************************************/

static void
DumpDomainToVisit(DBfile *db, int iter, double time, int Xsize, int Ysize, Set *quads, Set *points)
{
   int ok = 0;

   /* Create an option list that will give some hints to VisIt for
    * printing out the cycle and time in the annotations */
   DBoptlist *optlist;

   const char* coordnames[2] = {"X", "Y"};
   float *coords[2] ;
   coords[0] = new float[points->getSize()] ;
   coords[1] = new float[points->getSize()] ;
   int dims[2];

   /* do mesh */
   int ni=0;
   FieldRef<double,2> xy = points->getField<double,2>("xy");
   for (auto point = points->begin(); point != points->end(); ++point) {
      coords[0][ni] = float(xy.get(*point)(0)) ;
      coords[1][ni] = float(xy.get(*point)(1)) ;
      ni++;
   }
   dims[0]=Xsize;
   dims[1]=Ysize;

   /* Write out the mesh connectivity in fully unstructured format */
   int shapetype[1] = {DB_ZONETYPE_QUAD};
   int shapesize[1] = {4};
   int shapecnt[1] = {quads->getSize()};
   int *conn = new int[quads->getSize()*4] ;
   int ci = 0 ;int qi = 0;
   for (auto quad = quads->begin(); quad != quads->end(); ++quad) {
     for (auto point = quads->getEndpoints(*quad).begin(); point != quads->getEndpoints(*quad).end(); ++point) {
         conn[ci] =  point->getIdent() ;
         ci++;
      }
     qi++;
   }
   ok += DBPutZonelist2(db, "connectivity", quads->getSize(), 2, conn, quads->getSize()*4,
                        0,0,0, shapetype, shapesize, shapecnt, 1, NULL);
   optlist = DBMakeOptlist(2);
   ok += DBAddOption(optlist, DBOPT_DTIME, &time);
   ok += DBAddOption(optlist, DBOPT_CYCLE, &iter);

   /* Write out the mesh coordinates associated with the mesh */
   ok += DBPutUcdmesh (db,"Mesh",2,NULL,coords,points->getSize(),
		   	   	       points->getSize(),"connectivity",NULL,DB_FLOAT,optlist);

   delete [] conn ;
   ok += DBFreeOptlist(optlist);
   delete [] coords[1] ;
   delete [] coords[0] ;

   /* Write out Temperature */

   FieldRef<double> T = quads->getField<double>("T");
   float *Tq = new float[quads->getSize()] ;
   ni=0;
   for (auto quad = quads->begin(); quad != quads->end(); ++quad) {
      Tq[ni] = float(T.get(*quad)) ;
      ni++;
   }
   ok += DBPutUcdvar1(db, "T", "Mesh", Tq, quads->getSize(), NULL,
		   	   	   	  0, DB_FLOAT, DB_ZONECENT, NULL);
   delete [] Tq ;

   if (ok != 0) {
      printf("Error writing out viz file \n");
   }
}

#else

void DumpToVisit()
{
	printf("Must enable -DVIZ_MESH at compile time to call DumpToVisit \n");
}

#endif

