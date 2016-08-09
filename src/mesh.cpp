#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include "mesh.h"

using namespace simit;
using namespace std;

typedef array<double,3> Vector3d;
typedef array<int,3> Vector3i;
typedef map<int,int> IntMap;
int openIfstream(ifstream & in, const char * filename);
int openOfstream(ofstream & out, const char * filename);

//helper function used by saveHexObj
int findFace(const IntMap & m, int ei, const MeshVol & vol);
int findTetFace(const IntMap & m, int ei, const MeshVol & vol);

int HexFaces[6][4]={
    {0,1,3,2},{4,5,7,6},
    {0,4,5,1},{2,3,7,6},
    {0,2,6,4},{1,5,7,3}};
    
int TetFaces[4][3]={
    {0,2,1},{0,3,2},
    {0,1,3},{1,2,3}};

int openIfstream(ifstream & in, const char * filename)
{
  in.open(filename);
  if(!in.good()){
    std::cerr << "Cannot read " << filename << std::endl;
    return -1;
  }
  return 0;
}

int openOfstream(ofstream & out, const char * filename)
{
  out.open(filename);
  if(!out.good()){
    std::cerr << "Cannot write to " << filename << std::endl;
    return -1;
  }
  return 0;
}

int Mesh::load(const char * filename)
{
  ifstream in;
  int status = openIfstream(in,filename);
  if(status<0){
    return status;
  }
  status = load(in);
  in.close();
  return status;
}

int Mesh::load(std::string filename) {
  return load(filename.c_str());
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
    std::cerr << "Could not open " << filename << std::endl;
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
  if(saveColor){
    for(size_t ii=0;ii<v.size();ii++){
      out<<vTok<<" "<<v[ii][0]<<" "<<v[ii][1]<<" "<<v[ii][2]<<
      " "<<vcolor[ii][0]<<" "<<vcolor[ii][1]<<" "<<vcolor[ii][2]<<"\n";
    }
  }else{
    for(size_t ii=0;ii<v.size();ii++){
      out<<vTok<<" "<<v[ii][0]<<" "<<v[ii][1]<<" "<<v[ii][2]<<"\n";
    }
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
  ifstream in;
  int status = openIfstream(in,filename);
  if(status<0){
    return status;
  }
  status = load(in);
  in.close();
  return status;
}

int MeshVol::load(std::string filename) {
  return load(filename.c_str());
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
  for(unsigned int ii = 0;ii<e.size();ii++){
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
    std::cerr << "Could not open " << filename << std::endl;
    return -1;
  }
  int status = save(out);
  out.close();
  return status;
}

int MeshVol::save(std::string filename) {
  return save(filename.c_str());
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

int MeshVol::loadTet(const char * nodeFile, const char * eleFile)
{
  ifstream nodeIn, eleIn;
  int status = openIfstream(nodeIn, nodeFile);
  if(status<0){
    return status;
  }
  status = openIfstream(eleIn, eleFile);
  if(status<0){
    return status;
  }
  status = loadTet(nodeIn, eleIn);
  nodeIn.close();
  eleIn.close();
  return status;
}

int MeshVol::loadTet(std::string nodeFile, std::string eleFile) {
  return loadTet(nodeFile.c_str(), eleFile.c_str());
}

int MeshVol::loadTet(istream & nodeIn, istream & eleIn)
{
  
  //load vertices
  int intVal;
  nodeIn>>intVal;
  v.resize(intVal);
  string line;
  unsigned int cnt = 0;
  //discard rest of the first line.
  getline(nodeIn,line);
  while(1) {
    getline(nodeIn,line);
    if(nodeIn.eof()) {
      break;
    }
    //skip empty lines
    if(line.size()<3) {
      continue;
    }
    //skip comments
    if(line.at(0)=='#') {
      continue;
    }
    stringstream ss(line);
    
    ss>>intVal;
    for(int ii = 0;ii<3;ii++){
      ss>>v[cnt][ii];
    }
    cnt ++ ;
    if(cnt>=v.size()){
      break;
    }
  }
  
  //load elements
  eleIn>>intVal;
  e.resize(intVal);
  
  int nV=0;
  eleIn>>nV;
  cnt = 0;
  getline(eleIn,line);
  while(1) {
    getline(eleIn,line);
    if(eleIn.eof()) {
      break;
    }
    if(line.size()<3) {
      continue;
    }
    if(line.at(0)=='#') {
      continue;
    }
    stringstream ss(line);
    ss>>intVal;
    
    e[cnt].resize(nV);
    for(int ii = 0;ii<nV;ii++){
      ss>>e[cnt][ii];
    }
    
    cnt ++ ;
    if(cnt>=e.size()){
      break;
    }
  }
  return 0;
}

int MeshVol::loadTetEdge(const char * edgeFile)
{
  ifstream edgeIn;
  int status = openIfstream(edgeIn, edgeFile);
  if(status<0){
    return status;
  }
  status = loadTetEdge(edgeIn);
  edgeIn.close();
  return status;
}

int MeshVol::loadTetEdge(std::string edgeFile) {
  return loadTetEdge(edgeFile.c_str());
}

int MeshVol::loadTetEdge(istream & edgeIn)
{
  //load vertices
  int intVal;
  edgeIn>>intVal;
  edges.resize(intVal);
  string line;
  unsigned int cnt = 0;
  //discard rest of the first line.
  getline(edgeIn,line);
  while(1) {
    getline(edgeIn,line);
    if(edgeIn.eof()) {
      break;
    }
    //skip empty lines
    if(line.size()<3) {
      continue;
    }
    //skip comments
    if(line.at(0)=='#') {
      continue;
    }
    stringstream ss(line);
    
    ss>>intVal;
    for(int ii = 0;ii<2;ii++){
      ss>>edges[cnt][ii];
    }
    cnt ++ ;
    if(cnt>=edges.size()){
      break;
    }
  }
  return 0;
}

void MeshVol::elementNeighbors(vector<vector<int> > & eleNeighbor)
{
  eleNeighbor.resize(v.size());
  for(unsigned int ii = 0;ii<e.size();ii++){
    for(unsigned int jj = 0;jj<e[ii].size();jj++){
      eleNeighbor[e[ii][jj]].push_back(ii);
    }
  }
}

int findFace(const IntMap & m, int ei, const MeshVol & vol){
  for(int fi = 0;fi<6;fi++){
    int cnt = 0;
    for(int fv = 0;fv<4;fv++){
      if(m.at(vol.e[ei][HexFaces[fi][fv]])==2){
        cnt++;
      }
    }
    if(cnt==4){
      return fi;
    }
  }
  return 0;
}

void MeshVol::updateSurfVert()
{
  for(unsigned int ii = 0;ii<v.size();ii++){
    if(vidx[ii]<0){
      continue;
    }
    surf.v[vidx[ii]] = v[ii];
  }
}

int MeshVol::saveHexObj(const char * filename)
{
  if(surf.v.size()==0){
    makeHexSurf();
  }
  updateSurfVert();
  return surf.save(filename);
}

int MeshVol::saveHexObj(std::string filename) {
  return saveHexObj(filename.c_str());
}

int saveTetObj(std::string filename);

void MeshVol::makeHexSurf()
{
  //is a face exterior
  exterior.resize(e.size());
  for(unsigned int ii = 0;ii<exterior.size();ii++){
    unsigned int nV = e[ii].size();
    exterior[ii].resize(nV,true);
  }
  
  //mark exterior faces
  vector<vector<int > > eleNeighbor;
  elementNeighbors(eleNeighbor);
  for(unsigned int ii =0 ;ii<v.size();ii++){
    for(unsigned int nj = 0; nj<eleNeighbor[ii].size()-1;nj++){
      int jj = eleNeighbor[ii][nj];
      for(unsigned int nk = nj+1; nk<eleNeighbor[ii].size();nk++){
        int kk = eleNeighbor[ii][nk];
        IntMap im;
        for(unsigned int vv = 0;vv<e[jj].size();vv++){
          im[e[jj][vv]] = 1;
        }
        for(unsigned int vv = 0;vv<e[kk].size();vv++){
          int key = e[kk][vv];
          auto entry = im.find(key);
          if(entry!=im.end()){
            im[key] = 2;
          }else{
            im[key] = 1;
          }
        }
        if(im.size()==12){
          //share a face
          int fi = findFace(im, jj, *this);
          exterior[jj][fi] = false;
          fi = findFace(im, kk, *this);
          exterior[kk][fi] = false;
        }
      }
    }
  }
  //save exterior vertices and exterior faces
  vidx.resize(v.size(),-1);
  int vCnt = 0;
  //save vertices
  for(unsigned int ii = 0; ii<e.size();ii++){
    for(int fi = 0;fi<6;fi++){
      if(!exterior[ii][fi]){
        continue;
      }
      for(int fv = 0;fv<4;fv++){
        int vi = e[ii][HexFaces[fi][fv]];
        if(vidx[vi]<0){
          vidx[vi] = vCnt;
          surf.v.push_back(v[vi]);
          vCnt++;
        }
      }
    }
  }
  //save faces
  for(unsigned int ii = 0; ii<e.size();ii++){
    for(int fi = 0;fi<6;fi++){
      if(!exterior[ii][fi]){
        continue;
      }
      int trigs[2][3] = {{0,1,2},{2,3,0}};
      for(int ti = 0;ti<2;ti++){
        std::array< int,3> newTrig;
        for(int tv = 0;tv<3;tv++){
          int vi = e[ii][HexFaces[fi][trigs[ti][tv]]];
          newTrig[tv] = vidx[vi];
        }
        surf.t.push_back(newTrig);
      }
    }
  }
}

int findTetFace(const IntMap & m, int ei, const MeshVol & vol)
{
  for(int fi = 0;fi<4;fi++){
    int cnt = 0;
    for(int fv = 0;fv<3;fv++){
      if(m.at(vol.e[ei][TetFaces[fi][fv]])==2){
        cnt++;
      }
    }
    if(cnt==3){
      return fi;
    }
  }
  return 0;
}

int MeshVol::saveTetObj(const char * filename)
{
  if(surf.v.size()==0){
    makeTetSurf();
  }
  updateSurfVert();
  return surf.save(filename);
}

int MeshVol::saveTetObj(std::string filename) {
  return saveTetObj(filename.c_str());
}

void MeshVol::makeTetSurf()
{
  exterior.resize(e.size());
  for(unsigned int ii = 0;ii<exterior.size();ii++){
    unsigned int nV = e[ii].size();
    exterior[ii].resize(nV, true);
  }
  
  //mark exterior faces
  vector<vector<int > > eleNeighbor;
  elementNeighbors(eleNeighbor);
  for(unsigned int ii =0 ;ii<v.size();ii++){
    for(unsigned int nj = 0; nj<eleNeighbor[ii].size()-1;nj++){
      int jj = eleNeighbor[ii][nj];
      for(unsigned int nk = nj+1; nk<eleNeighbor[ii].size();nk++){
        int kk = eleNeighbor[ii][nk];
        IntMap im;
        for(unsigned int vv = 0;vv<e[jj].size();vv++){
          im[e[jj][vv]] = 1;
        }
        for(unsigned int vv = 0;vv<e[kk].size();vv++){
          int key = e[kk][vv];
          auto entry = im.find(key);
          if(entry!=im.end()){
            im[key] = 2;
          }else{
            im[key] = 1;
          }
        }
        if(im.size()==5){
          //share a face
          int fi = findTetFace(im, jj, *this);
          exterior[jj][fi] = false;
          fi = findTetFace(im, kk, *this);
          exterior[kk][fi] = false;
        }
      }
    }
  }
  //save exterior vertices and exterior faces
  vidx.resize(v.size(),-1);
  int vCnt = 0;
  //save vertices
  for(unsigned int ii = 0; ii<e.size();ii++){
    for(int fi = 0;fi<4;fi++){
      if(!exterior[ii][fi]){
        continue;
      }
      for(int fv = 0;fv<3;fv++){
        int vi = e[ii][TetFaces[fi][fv]];
        if(vidx[vi]<0){
          vidx[vi] = vCnt;
          surf.v.push_back(v[vi]);
          vCnt++;
        }
      }
    }
  }
  //save faces
  for(unsigned int ii = 0; ii<e.size();ii++){
    for(int fi = 0;fi<4;fi++){
      if(!exterior[ii][fi]){
        continue;
      }
      array<int,3>newTrig;
      for(int fv = 0; fv<3; fv++){
        int vi = e[ii][TetFaces[fi][fv]];
        vi = vidx[vi];
        newTrig[fv] = vi;
      }
      surf.t.push_back(newTrig);
    }
  }
}

