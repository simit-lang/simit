#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "mesh.h"

using namespace simit;
using namespace std;

typedef array<double,3> Vector3d;
typedef array<int,3> Vector3i;

int Mesh::load(const char * filename)
{
  ifstream in(filename);
  if(!in.good()){
    std::cout<<"Could not open "<<filename<<"\n";
    return -1;
  }
  int status = load(in);
  in.close();
  return status;
}

int Mesh::load(istream & in)
{
  string line;
  string vTok("v");
  string fTok("f");
  char bslash='/',space=' ';
  string tok;
  while(1) {
    getline(in,line);
    if(in.eof()) {
      break;
    }
    if(line == "#end"){
      break;
    }
    if(line.size()<3) {
      continue;
    }
    if(line.at(0)=='#') {
      continue;
    }
    stringstream ss(line);
    ss>>tok;
    if(tok==vTok) {
      Vector3d vec;
      ss>>vec[0]>>vec[1]>>vec[2];
      v.push_back(vec);
    } else if(tok==fTok) {
      bool hasTexture = false;
      if (line.find(bslash) != string::npos) {
        replace(line.begin(), line.end(), bslash, space);
        hasTexture = true;
      }
      stringstream facess(line);
      facess>>tok;
      vector<int> vidx;
      int x;
      while(facess>>x){
        vidx.push_back(x);
        if(hasTexture){
          facess>>x;
        }
      }
      for(unsigned ii = 0;ii<vidx.size()-2;ii++){
        Vector3i trig;
        trig[0] = vidx[0]-1;
        for (int jj = 1; jj < 3; jj++) {
          trig[jj] = vidx[ii+jj]-1;
        }
        t.push_back(trig);
      }
    }
  }
  return 0;
}

int Mesh::save(const char * filename)
{
  ofstream out(filename);
  if(!out.good()){
    std::cout<<"Could not open "<<filename<<"\n";
    return -1;
  }
  int status = save(out);
  out.close();
  return status;  
}

int Mesh::save(ostream & out)
{
  string vTok("v");
  string fTok("f");
  string tok;
  for(size_t ii=0;ii<v.size();ii++){
    out<<vTok<<" "<<v[ii][0]<<" "<<v[ii][1]<<" "<<v[ii][2]<<"\n";
  }
  for(size_t ii=0;ii<t.size();ii++){
    out<<fTok<<" "<<t[ii][0]+1<<" "<<
    t[ii][1]+1<<" "<<t[ii][2]+1<<"\n";
  }
  out<<"#end\n";
  return 0;
}

int MeshVol::load(const char * filename)
{
  ifstream in(filename);
  if(!in.good()){
    std::cout<<"Could not open "<<filename<<"\n";
    return -1;
  }
  int status = load(in);
  in.close();
  return status;
}

int MeshVol::load(istream & in)
{
  string token;
  in>>token;
  int num;
  in>>num;
  v.resize(num);
  in>>token;
  in>>num;
  e.resize(num);
  for(unsigned int ii = 0;ii<v.size();ii++){
    in>>v[ii][0]>>v[ii][1]>>v[ii][2];
  }
  for(unsigned int ii = 0;ii<v.size();ii++){
    in>>num;
    e[ii].resize(num);
    for(unsigned int jj = 0; jj<e[ii].size(); jj++){
      in>>e[ii][jj];
    }
  }
  return 0;
}

int MeshVol::save(const char * filename)
{
  ofstream out(filename);
  if(!out.good()){
    std::cout<<"Could not open "<<filename<<"\n";
    return -1;
  }
  int status = save(out);
  out.close();
  return status;
}

int MeshVol::save(ostream & out)
{
  out<<"#vertices "<<v.size()<<"\n";
  out<<"#elements "<<e.size()<<"\n";
  for(unsigned int ii = 0;ii<v.size();ii++){
    out<<v[ii][0]<<" "<<v[ii][1]<<" "<<v[ii][2]<<"\n";
  }
  
  for(unsigned int ii = 0;ii<e.size();ii++){
    out<<e[ii].size();
    for(unsigned int jj = 0;jj<e[ii].size();jj++){
      out<<" "<<e[ii][jj];
    }
    out<<"\n";
  }
  
  return 0;
}
