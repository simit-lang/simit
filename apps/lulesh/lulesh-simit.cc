/*
  This is a Version 2.0 MPI + OpenMP implementation of LULESH

                 Copyright (c) 2010-2013.
      Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory.
                  LLNL-CODE-461231
                All rights reserved.

This file is part of LULESH, Version 2.0.
Please also read this link -- http://www.opensource.org/licenses/index.php

//////////////
DIFFERENCES BETWEEN THIS VERSION (2.x) AND EARLIER VERSIONS:
* Addition of regions to make work more representative of multi-material codes
* Default size of each domain is 30^3 (27000 elem) instead of 45^3. This is
  more representative of our actual working set sizes
* Single source distribution supports pure serial, pure OpenMP, MPI-only, 
  and MPI+OpenMP
* Addition of ability to visualize the mesh using VisIt 
  https://wci.llnl.gov/codes/visit/download.html
* Various command line options (see ./lulesh2.0 -h)
 -q              : quiet mode - suppress stdout
 -i <iterations> : number of cycles to run
 -s <size>       : length of cube mesh along side
 -r <numregions> : Number of distinct regions (def: 11)
 -b <balance>    : Load balance between regions of a domain (def: 1)
 -c <cost>       : Extra cost of more expensive regions (def: 1)
 -f <filepieces> : Number of file parts for viz output (def: np/9)
 -p              : Print out progress
 -v              : Output viz file (requires compiling with -DVIZ_MESH
 -h              : This message

 printf("Usage: %s [opts]\n", execname);
      printf(" where [opts] is one or more of:\n");
      printf(" -q              : quiet mode - suppress all stdout\n");
      printf(" -i <iterations> : number of cycles to run\n");
      printf(" -s <size>       : length of cube mesh along side\n");
      printf(" -r <numregions> : Number of distinct regions (def: 11)\n");
      printf(" -b <balance>    : Load balance between regions of a domain (def: 1)\n");
      printf(" -c <cost>       : Extra cost of more expensive regions (def: 1)\n");
      printf(" -f <numfiles>   : Number of files to split viz dump into (def: (np+10)/9)\n");
      printf(" -p              : Print out progress\n");
      printf(" -v              : Output viz file (requires compiling with -DVIZ_MESH\n");
      printf(" -h              : This message\n");
      printf("\n\n");

*Notable changes in LULESH 2.0

* Split functionality into different files
lulesh.cc - where most (all?) of the timed functionality lies
lulesh-comm.cc - MPI functionality
lulesh-init.cc - Setup code
lulesh-viz.cc  - Support for visualization option
lulesh-util.cc - Non-timed functions
*
* The concept of "regions" was added, although every region is the same ideal
*    gas material, and the same sedov blast wave problem is still the only
*    problem its hardcoded to solve.
* Regions allow two things important to making this proxy app more representative:
*   Four of the LULESH routines are now performed on a region-by-region basis,
*     making the memory access patterns non-unit stride
*   Artificial load imbalances can be easily introduced that could impact
*     parallelization strategies.  
* The load balance flag changes region assignment.  Region number is raised to
*   the power entered for assignment probability.  Most likely regions changes
*   with MPI process id.
* The cost flag raises the cost of ~45% of the regions to evaluate EOS by the
*   entered multiple. The cost of 5% is 10x the entered multiple.
* MPI and OpenMP were added, and coalesced into a single version of the source
*   that can support serial builds, MPI-only, OpenMP-only, and MPI+OpenMP
* Added support to write plot files using "poor mans parallel I/O" when linked
*   with the silo library, which in turn can be read by VisIt.
* Enabled variable timestep calculation by default (courant condition), which
*   results in an additional reduction.
* Default domain (mesh) size reduced from 45^3 to 30^3
* Command line options to allow numerous test cases without needing to recompile
* Performance optimizations and code cleanup beyond LULESH 1.0
* Added a "Figure of Merit" calculation (elements solved per microsecond) and
*   output in support of using LULESH 2.0 for the 2017 CORAL procurement
*
* Possible Differences in Final Release (other changes possible)
*
* High Level mesh structure to allow data structure transformations
* Different default parameters
* Minor code performance changes and cleanup

TODO in future versions
* Add reader for (truly) unstructured meshes, probably serial only
* CMake based build system

//////////////

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

   * Redistributions of source code must retain the above copyright
     notice, this list of conditions and the disclaimer below.

   * Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the disclaimer (as noted below)
     in the documentation and/or other materials provided with the
     distribution.

   * Neither the name of the LLNS/LLNL nor the names of its contributors
     may be used to endorse or promote products derived from this software
     without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC,
THE U.S. DEPARTMENT OF ENERGY OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Additional BSD Notice

1. This notice is required to be provided under our contract with the U.S.
   Department of Energy (DOE). This work was produced at Lawrence Livermore
   National Laboratory under Contract No. DE-AC52-07NA27344 with the DOE.

2. Neither the United States Government nor Lawrence Livermore National
   Security, LLC nor any of their employees, makes any warranty, express
   or implied, or assumes any liability or responsibility for the accuracy,
   completeness, or usefulness of any information, apparatus, product, or
   process disclosed, or represents that its use would not infringe
   privately-owned rights.

3. Also, reference herein to any specific commercial products, process, or
   services by trade name, trademark, manufacturer or otherwise does not
   necessarily constitute or imply its endorsement, recommendation, or
   favoring by the United States Government or Lawrence Livermore National
   Security, LLC. The views and opinions of authors expressed herein do not
   necessarily state or reflect those of the United States Government or
   Lawrence Livermore National Security, LLC, and shall not be used for
   advertising or product endorsement purposes.

*/

#include <climits>
#include <vector>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>

#include "lulesh.h"

// using simit namespace
#include "graph.h"
#include "program.h"
#include "mesh.h"
#include <cmath>
using namespace simit;


/******************************************/

int main(int argc, char *argv[])
{
  Domain *locDom ;
   Int_t numRanks ;
   Int_t myRank ;
   struct cmdLineOpts opts;

   numRanks = 1;
   myRank = 0;

   /* Set defaults that can be overridden by command line opts */
   opts.its = 9999999;
   opts.nx  = 30;
   opts.numReg = 11;
   opts.numFiles = (int)(numRanks+10)/9;
   opts.showProg = 0;
   opts.quiet = 0;
   opts.viz = 0;
   opts.balance = 1;
   opts.cost = 1;

   ParseCommandLineOptions(argc, argv, myRank, &opts);

   if ((myRank == 0) && (opts.quiet == 0)) {
      printf("Running problem size %d^3 per domain until completion\n", opts.nx);
      printf("Num processors: %d\n", numRanks);
      printf("Total number of elements: %lld\n\n", (long long int)(numRanks*opts.nx*opts.nx*opts.nx));
      printf("To run other sizes, use -s <integer>.\n");
      printf("To run a fixed number of iterations, use -i <integer>.\n");
      printf("Not Available in the SIMIT version : To run a more or less balanced region set, use -b <integer>.\n");
      printf("To change the relative costs of regions, use -c <integer>.\n");
      printf("To print out progress, use -p\n");
      printf("To write an output file for VisIt, use -v\n");
      printf("See help (-h) for more options\n\n");
   }

   // Initialize Simit
   simit::init("cpu", sizeof(double));

   // Set up the mesh and decompose. Assumes regular cubes for now
   Int_t col, row, plane, side;
   InitMeshDecomp(numRanks, myRank, &col, &row, &plane, &side);

   // Build the main data structure and initialize it
   locDom = new Domain(numRanks, col, row, plane, opts.nx,
                       side, opts.numReg, opts.balance, opts.cost) ;

   // Create a graph and initialize it with Domain data
   Set nodes;
   Set elems(nodes, nodes, nodes, nodes, nodes, nodes, nodes, nodes);
   Set connects(elems,elems,elems,elems,elems,elems,elems);
   Set symmX(nodes);
   Set symmY(nodes);
   Set symmZ(nodes);

   // The fields of the nodes set
   FieldRef<double,3> coord 	= nodes.addField<double,3>("coord");	// Nodal coordinates
   FieldRef<double,3> coord_local 	= nodes.addField<double,3>("coord_local");	// Nodal coordinates
   FieldRef<double,3> vel     	= nodes.addField<double,3>("vel");		// Nodal velocities
   FieldRef<double,3> a     	= nodes.addField<double,3>("a");		// Nodal accelerations
   FieldRef<double,3> f     	= nodes.addField<double,3>("f");		// Nodal forces
   FieldRef<double> nodalMass 	= nodes.addField<double>("nodalMass");	// Nodal mass
   FieldRef<int,3> symm     	= nodes.addField<int,3>("symm");		// Nodes on symmetry planes

   std::vector<ElementRef> nodeRefs;
   // initialize nodes values with Domain
   Index_t edgeElems = opts.nx;
   Index_t edgeNodes = edgeElems+1;  // In Lulesh same number of elems and nodes in each dimension
   Index_t nidx = 0 ;

   for (Index_t plane=0; plane<edgeNodes; ++plane) {
     for (Index_t row=0; row<edgeNodes; ++row) {
       for (Index_t col=0; col<edgeNodes; ++col) {
    	     ElementRef node = nodes.add();
    	     nodeRefs.push_back(node);
    	     coord.set(node, {locDom->x(nidx),locDom->y(nidx),locDom->z(nidx)});
    	     vel.set(node, {locDom->xd(nidx),locDom->yd(nidx),locDom->zd(nidx)});
    	     a.set(node, {locDom->xdd(nidx),locDom->ydd(nidx),locDom->zdd(nidx)});
    	     f.set(node, {locDom->fx(nidx),locDom->fy(nidx),locDom->fz(nidx)});
    	     nodalMass.set(node, locDom->nodalMass(nidx));
    	     ++nidx ;
       }
     }
   }

   // The fields of the elems set
   FieldRef<int,3,6> elemBC 	= elems.addField<int,3,6>("elemBC");	/* symmetry/free-surface flags for each elem face */
   FieldRef<double,3> dxyz     	= elems.addField<double,3>("dxyz");		/* principal strains -- temporary */
   FieldRef<double,3> delvel  	= elems.addField<double,3>("delvel");	/* velocity gradient -- temporary */
   FieldRef<double,3> delx  	= elems.addField<double,3>("delx");		/* coordinate gradient -- temporary */
   FieldRef<double> e     		= elems.addField<double>("e");   		/* energy */
   FieldRef<double> p     		= elems.addField<double>("p");   		/* pressure */
   FieldRef<double> q     		= elems.addField<double>("q");   		/* q */
   FieldRef<double> ql    		= elems.addField<double>("ql");  		/* linear term for q */
   FieldRef<double> qq    		= elems.addField<double>("qq");   		/* quadratic term for q */
   FieldRef<double> v     		= elems.addField<double>("v");   		/* relative volume */
   FieldRef<double> volo     	= elems.addField<double>("volo");   	/* reference volume */
   FieldRef<double> delv     	= elems.addField<double>("delv");   	/* m_vnew - m_v */
   FieldRef<double> vdov     	= elems.addField<double>("vdov");    	/* volume derivative over volume */
   FieldRef<double> arealg     	= elems.addField<double>("arealg");   	/* characteristic length of an element */
   FieldRef<double> ss     		= elems.addField<double>("ss");        	/* "sound speed" */
   FieldRef<double> elemMass    = elems.addField<double>("elemMass");   /* mass */

   // initialize elems values with Domain
   std::vector<ElementRef> elemRefs;
   nidx = 0 ;
   for (Index_t plane=0; plane<edgeElems; ++plane) {
     for (Index_t row=0; row<edgeElems; ++row) {
       for (Index_t col=0; col<edgeElems; ++col) {
    	     ElementRef elem = elems.add(nodeRefs[locDom->nodelist(nidx)[0]], nodeRefs[locDom->nodelist(nidx)[1]],
										 nodeRefs[locDom->nodelist(nidx)[2]], nodeRefs[locDom->nodelist(nidx)[3]],
										 nodeRefs[locDom->nodelist(nidx)[4]], nodeRefs[locDom->nodelist(nidx)[5]],
										 nodeRefs[locDom->nodelist(nidx)[6]], nodeRefs[locDom->nodelist(nidx)[7]]);
    	     elemRefs.push_back(elem);
    	     e.set(elem,{locDom->e(nidx)});
    	     p.set(elem,{locDom->p(nidx)});
    	     q.set(elem,{locDom->q(nidx)});
    	     ql.set(elem,{locDom->ql(nidx)});
    	     qq.set(elem,{locDom->qq(nidx)});
    	     v.set(elem,{locDom->v(nidx)});
    	     volo.set(elem,{locDom->volo(nidx)});
    	     delv.set(elem,{locDom->delv(nidx)});
    	     vdov.set(elem,{locDom->vdov(nidx)});
    	     arealg.set(elem,{locDom->arealg(nidx)});
    	     ss.set(elem,{locDom->ss(nidx)});
    	     elemMass.set(elem,{locDom->elemMass(nidx)});
    	     Int_t bcMask = locDom->elemBC(nidx) ;
    	     std::vector<int> maskxim(3,0);
    	     switch (bcMask & XI_M) {
    	     	 case XI_M_COMM: maskxim[0]=0;maskxim[1]=0;maskxim[2]=1; break;
    	         case 0:         break;
    	         case XI_M_SYMM: maskxim[0]=0;maskxim[1]=1;maskxim[2]=0; break;
    	         case XI_M_FREE: maskxim[0]=1;maskxim[1]=0;maskxim[2]=0; break ;
    	     }
    	     std::vector<int> maskxip(3,0);
    	      switch (bcMask & XI_P) {
    	         case XI_P_COMM: maskxip[0]=0;maskxip[1]=0;maskxip[2]=1; break;
    	         case 0:         break ;
    	         case XI_P_SYMM: maskxip[0]=0;maskxip[1]=1;maskxip[2]=0; break ;
    	         case XI_P_FREE: maskxip[0]=1;maskxip[1]=0;maskxip[2]=0; break ;
    	      }
     	     std::vector<int> masketam(3,0);
    	      switch (bcMask & ETA_M) {
    	         case ETA_M_COMM: masketam[0]=0;masketam[1]=0;masketam[2]=1; break;
    	         case 0:          break ;
    	         case ETA_M_SYMM: masketam[0]=0;masketam[1]=1;masketam[2]=0; break ;
    	         case ETA_M_FREE: masketam[0]=1;masketam[1]=0;masketam[2]=0; break ;
    	      }
     	     std::vector<int> masketap(3,0);
    	      switch (bcMask & ETA_P) {
    	         case ETA_P_COMM: masketap[0]=0;masketap[1]=0;masketap[2]=1; break;
    	         case 0:          break ;
    	         case ETA_P_SYMM: masketap[0]=0;masketap[1]=1;masketap[2]=0; break ;
    	         case ETA_P_FREE: masketap[0]=1;masketap[1]=0;masketap[2]=0; break ;
    	      }
     	     std::vector<int> maskzetam(3,0);
    	      switch (bcMask & ZETA_M) {
    	         case ZETA_M_COMM: maskzetam[0]=0;maskzetam[1]=0;maskzetam[2]=1; break;
    	         case 0:           break ;
    	         case ZETA_M_SYMM: maskzetam[0]=0;maskzetam[1]=1;maskzetam[2]=0; break ;
    	         case ZETA_M_FREE: maskzetam[0]=1;maskzetam[1]=0;maskzetam[2]=0; break ;
    	      }
     	     std::vector<int> maskzetap(3,0);
    	      switch (bcMask & ZETA_P) {
    	         case ZETA_P_COMM: maskzetap[0]=0;maskzetap[1]=0;maskzetap[2]=1; break;
    	         case 0:           break ;
    	         case ZETA_P_SYMM: maskzetap[0]=0;maskzetap[1]=1;maskzetap[2]=0; break ;
    	         case ZETA_P_FREE: maskzetap[0]=1;maskzetap[1]=0;maskzetap[2]=0; break ;
    	      }
    	      elemBC.set(elem,{maskxim[2],maskxip[2],masketam[2],masketap[2],maskzetam[2],maskzetap[2],
    	    		  	  	   maskxim[1],maskxip[1],masketam[1],masketap[1],maskzetam[1],maskzetap[1],
							   maskxim[0],maskxip[0],masketam[0],masketap[0],maskzetam[0],maskzetap[0]});
    	      ++nidx ;
       }
     }
   }

   /* element connectivity across each face */
   nidx=0;
   std::vector<ElementRef> connectRefs;
   for (Index_t plane=0; plane<edgeElems; ++plane) {
     for (Index_t row=0; row<edgeElems; ++row) {
       for (Index_t col=0; col<edgeElems; ++col) {
    	   ElementRef connect = connects.add(elemRefs[nidx],
    			   	   	   	    elemRefs[locDom->lxim(nidx)],elemRefs[locDom->lxip(nidx)],
								elemRefs[locDom->letam(nidx)],elemRefs[locDom->letap(nidx)],
								elemRefs[locDom->lzetam(nidx)],elemRefs[locDom->lzetap(nidx)]);
    	   connectRefs.push_back(connect);
    	   ++nidx;
       }
     }
   }
   std::vector<ElementRef> symmXRefs;
   std::vector<ElementRef> symmYRefs;
   std::vector<ElementRef> symmZRefs;
   for (int bc =0; bc<edgeNodes*edgeNodes; ++bc){
	   if (!locDom->symmXempty() != 0) {
		   ElementRef BoundNode = symmX.add(nodeRefs[locDom->symmX(bc)]);
		   symmXRefs.push_back(BoundNode);
	   }
	   if (!locDom->symmYempty() != 0) {
		   ElementRef BoundNode = symmY.add(nodeRefs[locDom->symmY(bc)]);
		   symmYRefs.push_back(BoundNode);
	   }
	   if (!locDom->symmZempty() != 0) {
		   ElementRef BoundNode = symmZ.add(nodeRefs[locDom->symmZ(bc)]);
		   symmZRefs.push_back(BoundNode);
	   }
   }

	simit::Tensor<int,2> cycle, iterMax, showProg;
	cycle(0)= locDom->cycle();
	iterMax(0)=opts.its;
	showProg(0)=opts.showProg;
	simit::Tensor<double,2> time, deltatime, stoptime, dtfixed,
							dtcourant, dthydro, deltatimemultlb,
							deltatimemultub, dtmax,
							hgcoef, u_cut, qstop, monoq_limiter_mult,
							monoq_max_slope, qlc_monoq, qqc_monoq,
							eosvmin, eosvmax, v_cut, qqc, dvovmax,
							rho0, ss4o3, e_cut, p_cut, q_cut, emin, pmin;
	time(0)=locDom->time();
	deltatime(0)=locDom->deltatime();
	stoptime(0)=locDom->stoptime();
	dtfixed(0)=locDom->dtfixed();
	dtcourant(0)=locDom->dtcourant();
	dthydro(0)=locDom->dthydro();
	deltatimemultlb(0)=locDom->deltatimemultlb();
	deltatimemultub(0)=locDom->deltatimemultub();
	dtmax(0)=locDom->dtmax();
	hgcoef(0)=locDom->hgcoef();
	u_cut(0)=locDom->u_cut();
	qstop(0)=locDom->qstop();
	monoq_limiter_mult(0)=locDom->monoq_limiter_mult();
	monoq_max_slope(0)=locDom->monoq_max_slope();
	qlc_monoq(0)=locDom->qlc_monoq();
	qqc_monoq(0)=locDom->qqc_monoq();
	eosvmin(0)=locDom->eosvmin();
	eosvmax(0)=locDom->eosvmax();
	v_cut(0)=locDom->v_cut();
	qqc(0)=locDom->qqc();
	dvovmax(0)=locDom->dvovmax();
	rho0(0)=locDom->refdens();
	ss4o3(0)=locDom->ss4o3();
	e_cut(0)=locDom->e_cut();
	p_cut(0)=locDom->p_cut();
	q_cut(0)=locDom->q_cut();
	emin(0)=locDom->emin();
	pmin(0)=locDom->pmin();

	std::string codefile = "../lulesh.sim";
   // Compile program and bind arguments
   Program program;
   program.loadFile(codefile);

   Function lulesh_sim = program.compile("lulesh_sim");
   lulesh_sim.bind("nodes",  &nodes);
   lulesh_sim.bind("elems", &elems);
   lulesh_sim.bind("connects", &connects);
   lulesh_sim.bind("symmX", &symmX);
   lulesh_sim.bind("symmY", &symmY);
   lulesh_sim.bind("symmZ", &symmZ);
   lulesh_sim.bind("cycle", &cycle);
   lulesh_sim.bind("time", &time);
   lulesh_sim.bind("deltatime", &deltatime);
   lulesh_sim.bind("stoptime", &stoptime);
   lulesh_sim.bind("iterMax", &iterMax);
   lulesh_sim.bind("showProg", &showProg);
   lulesh_sim.bind("dtfixed", &dtfixed);
   lulesh_sim.bind("dtcourant", &dtcourant);
   lulesh_sim.bind("dthydro", &dthydro);
   lulesh_sim.bind("deltatimemultlb", &deltatimemultlb);
   lulesh_sim.bind("deltatimemultub", &deltatimemultub);
   lulesh_sim.bind("dtmax", &dtmax);
   lulesh_sim.bind("hgcoef", &hgcoef);
   lulesh_sim.bind("u_cut", &u_cut);
   lulesh_sim.bind("qstop", &qstop);
   lulesh_sim.bind("monoq_limiter_mult", &monoq_limiter_mult);
   lulesh_sim.bind("monoq_max_slope", &monoq_max_slope);
   lulesh_sim.bind("qlc_monoq", &qlc_monoq);
   lulesh_sim.bind("qqc_monoq", &qqc_monoq);
   lulesh_sim.bind("eosvmin", &eosvmin);
   lulesh_sim.bind("eosvmax", &eosvmax);
   lulesh_sim.bind("v_cut", &v_cut);
   lulesh_sim.bind("qqc", &qqc);
   lulesh_sim.bind("dvovmax", &dvovmax);
   lulesh_sim.bind("rho0", &rho0);
   lulesh_sim.bind("ss4o3", &ss4o3);
   lulesh_sim.bind("e_cut", &e_cut);
   lulesh_sim.bind("p_cut", &p_cut);
   lulesh_sim.bind("q_cut", &q_cut);
   lulesh_sim.bind("emin", &emin);
   lulesh_sim.bind("pmin", &pmin);

   lulesh_sim.init();

   // BEGIN timestep to solution */
   timeval start;
   gettimeofday(&start, NULL) ;

   lulesh_sim.run();

   // Use reduced max elapsed time
   double elapsed_time;
   timeval end;
   gettimeofday(&end, NULL) ;
   elapsed_time = (double)(end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec))/1000000 ;
   double elapsed_timeG;
   elapsed_timeG = elapsed_time;

   // Write out final viz file */
   //if (opts.viz) {
   //   DumpToVisit(*locDom, opts.numFiles, myRank, numRanks) ;
   //}

   // Put Energy back in locDom to verify results
   nidx = 0 ;
   for (Index_t plane=0; plane<edgeElems; ++plane) {
     for (Index_t row=0; row<edgeElems; ++row) {
       for (Index_t col=0; col<edgeElems; ++col) {
    	     locDom->e(nidx)=e.get(elemRefs[nidx]);
    	     nidx++;
       }
     }
   }
   if ((myRank == 0) && (opts.quiet == 0)) {
      VerifyAndWriteFinalOutput(elapsed_timeG, *locDom, opts.nx, numRanks);
   }

   return 0 ;
}
