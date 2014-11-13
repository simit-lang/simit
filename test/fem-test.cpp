#include "gtest/gtest.h"

#include <cmath>
#include <iostream>

#include "graph.h"
#include "program.h"
#include "function.h"
#include "errors.h"

using namespace std;
using namespace simit;

#define VX(ii,jj) ( (jj)*(sx+1) + (ii))

TEST(Program, fem2d) {
  Program program;
  std::string programText = R"(
element Trig
  u : float;
  l : float;
  m : float;
end

element Vert
  X : tensor[2](float);
  x : tensor[2](float);
  v : tensor[2](float);

  print : tensor[2](float);
end

extern verts : set{Vert};
extern trigs : set{Trig}(verts, verts);

func distribute_masses(t : Trig, v : (Vert*3)) ->
    (M : tensor[verts](tensor[2](float)))
  % Eigen match
%  thirdm = (1.0/3.0)*t.m;
%  thirdmvec = [0.0, 0.0];
%  thirdmvec(0) = thirdm;
%  thirdmvec(1) = thirdm;
%  M(v(0)) = thirdmvec;
%  M(v(1)) = thirdmvec;
%  M(v(2)) = thirdmvec;

  % Matlab match
  me = 0.01;
  mevec = [0.0, 0.0];
  mevec(0) = me/3.0;
  mevec(1) = me/3.0;

  M(v(0)) = mevec;
  M(v(1)) = mevec;
  M(v(2)) = mevec;
end

func compute_force(t : Trig, v : (Vert*3)) ->
    (f : tensor[verts](tensor[2](float)))
  % Note to match eigen code instead of matlab replace these with t.m and t.l
  mu = 10.0;
  lambda = 100.0;

  eye2 = [1.0, 0.0; 0.0, 1.0];
  e0 = v(1).x - v(0).x;
  e1 = v(2).x - v(0).x;
  e2 = v(2).x - v(1).x;
  E0 = v(1).X - v(0).X;
  E1 = v(2).X - v(0).X;
  F = [0.0, 0.0; 0.0, 0.0];
  F(0,0) = e0(0);
  F(0,1) = e0(1);
  F(1,0) = e1(0);
  F(1,1) = e1(1);

  m = [0.0, 0.0; 0.0, 0.0];
  m(0,0) = E0(0);
  m(0,1) = E0(1);
  m(1,0) = E1(0);
  m(1,1) = E1(1);

  det2m = m(0,0) * m(1,1) - m(0,1) * m(1,0);

  inv2m = [0.0, 0.0; 0.0, 0.0];

  d = 1.0/det2m;
  inv2m(0,0) =  m(1,1);
  inv2m(0,1) = -m(0,1);
  inv2m(1,0) = -m(1,0);
  inv2m(1,1) =  m(0,0);
  inv2m = d * inv2m;

  F = F * inv2m;
  FtF = F'*F;
  E = 0.5 * (FtF-eye2);
  trace2E = E(0,0) + E(1,1);
  P = mu * E + lambda * trace2E * eye2;
  J = F(0,0) * F(1,1) - F(0,1) * F(1,0);
  stress = (P*F')./J;

  n = [0.0, 0.0];
  n(0) = e0(1);
  n(1) = -e0(0);
  fe0 = stress*n;

  n(0) = e2(1);
  n(1) = -e2(0);
  fe1 = stress*n;

  n(0) = -e1(1);
  n(1) = e1(0);
  fe2 = stress*n;

  f(v(1)) = -0.5*(fe0 + fe1);
  f(v(0)) = -0.5*(fe0 + fe2);
  f(v(2)) = -0.5*(fe1 + fe2);
end

proc main
  h = 0.0002;
  verts.x  = verts.x + h*verts.v;

  Mm = map distribute_masses to trigs with verts reduce +;
  fm = map compute_force to trigs with verts reduce +;

  M = --Mm;
  verts.v = fm ./ M;

  % Eigen match
%  verts.print = --M;
%  v = 0.99*v + h*f./M;
%  verts.v = v;

  % Matlab match
end
  )";

  simit::Set<> verts;
  simit::Set<3> trigs(verts,verts,verts);
  simit::FieldRef<double,2> X = verts.addField<double,2>("X");
  simit::FieldRef<double,2> x = verts.addField<double,2>("x");
  simit::FieldRef<double,2> v = verts.addField<double,2>("v");
  simit::FieldRef<double,2> print = verts.addField<double,2>("print");

  simit::FieldRef<double> u = trigs.addField<double>("u");
  simit::FieldRef<double> l = trigs.addField<double>("l");
  simit::FieldRef<double> m = trigs.addField<double>("m");

  double uval = 1000, lval = 10000, mval = 10;
  int sx = 1;
  int sy = 1;

  //create nodes
  vector<simit::ElementRef> vertRefs;
  for(int jj =0; jj<sy+1; jj++){
    for(int ii=0; ii<sx+1; ii++){
      vertRefs.push_back(verts.addElement());
      simit::ElementRef p = vertRefs.back();
      X.set(p, {(double)ii, (double)jj});
      x.set(p, {1.1*(double)ii, 1.1*(double)jj});
      v.set(p, {0.0, 0.0} );
    }
  }

  for(int jj =0; jj<sy; jj++){
    for(int ii=0; ii<sx; ii++){
      simit::ElementRef t = trigs.addElement(vertRefs[VX(ii,   jj)],
                                               vertRefs[VX(ii+1, jj)],
                                               vertRefs[VX(ii  , jj+1)]);
      u.set(t, uval);
      l.set(t, lval);
      m.set(t, mval);
      t = trigs.addElement(vertRefs[VX(ii+1, jj+1)],
                             vertRefs[VX(ii  , jj+1)],
                             vertRefs[VX(ii+1, jj  )]);
      u.set(t, uval);
      l.set(t, lval);
      m.set(t, mval);
      
    }
  }

  std::cout << "Initial position" << std::endl;
  for (auto &vert : verts) {
    std::cout << v.get(vert) << std::endl;
  }
  std::cout << std::endl;

  // Compile program and bind arguments
  int errorCode = program.loadString(programText);
  if (errorCode) FAIL() << program.getDiagnostics().getMessage();

  std::unique_ptr<Function> func = program.compile("main");
  if (!func) FAIL() << program.getDiagnostics().getMessage();

  func->bind("verts", &verts);
  func->bind("trigs", &trigs);
  func->runSafe();

  std::cout << "Final position" << std::endl;
  for (auto &vert : verts) {
    std::cout << v.get(vert) << std::endl;
  }
}