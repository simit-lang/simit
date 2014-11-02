#include "mesh_loader.h"

#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "graph.h"

using namespace std;
using namespace simit;

typedef vector<array<double, 3>> vec3d;
typedef vector<array<double, 4>> vec4d;
typedef vector<array<unsigned, 3>> vec3i;

static void tokenize(string &input, string *cmd, vector<string> *args)
{
  args->clear();
  string in = input.substr(0, input.find('#'));
  replace(in.begin(), in.end(), '\t', ' ');
  char *p = strtok(&(in[0]), " ");
  if (p == NULL) {
    *cmd = "";
    return;
  }
  *cmd = string(p);
  p = strtok(NULL, " ");
  while (p != NULL) {
    args->push_back(string(p));
    p = strtok(NULL, " ");
  }
}

static int handle_v(vector<string> &tokens, vec4d *vlist) {
  vlist->push_back(array<double, 4>());
  int ret = 0;
  auto &v = vlist->back();
  v[3] = 1;
  for (int i = 0; i < 3; i++) {
    stringstream s(tokens[i]);
    s >> v[i];
    if (s.fail()) {
      ret = -1;
    }
  }
  if (ret) {
    vlist->pop_back();
  }
  return ret;
}

static int handle_f(vector<string> &tokens, vec3i *flist) {
  flist->push_back(array<unsigned, 3>());
  auto &f = flist->back();
  int type[3];
  int ret = 0;
  for (int i = 0; i < 3; i++) {
    size_t pos1 = tokens[i].find('/'), pos2 = tokens[i].rfind('/');
    if (pos1 == 0 || pos2 == tokens[i].npos - 1) {
      ret = -1;
    } else if (pos1 == tokens[i].npos) {  // 1
      type[i] = 0;
    } else if (pos1 == pos2) {  // 1/2
      type[i] = 1;
    } else if (pos1 + 1 == pos2) {  // 1//2
      type[i] = 2;
    } else {  // 1/2/3
      type[i] = 3;
    }
    string temp = tokens[i].substr(0, pos1);
    switch (type[i]) {
    case 0:
      temp += " 0 0 ";
      break;
    case 1:
      temp += " " + tokens[i].substr(pos1 + 1) + " 0 ";
      break;
    case 2:
      temp += " 0 " + tokens[i].substr(pos2 + 1) + " ";
      break;
    case 3:
      temp += " " + tokens[i].substr(pos1 + 1, pos2 - pos1 - 1) + " "
          + tokens[i].substr(pos2 + 1) + " ";
      break;
    default:
      throw;
    }
    stringstream stream = stringstream(temp);
    int dummy;
    stream >> f[i] >> dummy >> dummy;
    if (!stream) {
      ret = -1;
    }
  }
  if (type[0] != type[1] || type[1] != type[2]) {
    ret = -2;
  }
  if (ret) {
    flist->pop_back();
  }
  return ret;
}

#define ERROR(msg) \
{\
  res = -1;\
  continue;\
}

static int parse(istream &input, vec4d *vlist, vec3i *flist)
{
  int res = 0;
  string line;
  int lineNum = 0;
  vector<string> lines; // get lines so that extra info can be written
                        //back, currently unused;
  while(input) {
    lineNum++;
    getline(input, line);
    while (line.back() == '\\') {
      string nextline;
      lineNum++;
      getline(input, nextline);
      line.pop_back();
      line += nextline;
    }
    lines.push_back(line);
  }
  lines.pop_back();

  *vlist = vec4d(1);
  *flist = vec3i();
  for (size_t i = 0; i < lines.size(); ++i) {
    vector<string> tokens;
    string cmd;
    tokenize(lines[i], &cmd, &tokens);
    if (cmd == "") {
      continue;
    }if (cmd == "v") {
      if (tokens.size() < 3) {
        ERROR("Incorrect number of args, expect 3 or 4");
      }
      if (tokens.size() > 4) {
        res = -1;
      }
      int ret = handle_v(tokens, vlist);
      if (ret) {
        ERROR("Invalid args");
      }
    } else if (cmd == "f") {
      if (tokens.size() != 3) {
        ERROR("Incorrect number of args, expect 3");
      }
      int ret = handle_f(tokens, flist);
      if (ret) {
        ERROR("Invalid args");
      }
    } else {
      ERROR("Unsupported/Invalid command");
    }
  }
  return res;
}

static int makeSet(vec4d &vlist, vec3i &flist, simit::Set<> *points,
                    simit::Set<3> *faces) {

  FieldRef<int> id = points->addField<int>("id");
  FieldRef<double, 3> pos = points->addField<double, 3>("x");
  vector<ElementRef> elems;
  elems.reserve(vlist.size() + 1);
  elems.push_back(points->addElement());
  int id_num = 1;
  for (vec4d::iterator i = vlist.begin() + 1; i != vlist.end(); ++i, ++id_num) {
    elems.push_back(points->addElement());
    ElementRef p = elems.back();
    pos.set(p, {(*i)[0], (*i)[1], (*i)[2]});
    id.set(p, id_num);
  }
  int res = 0;
  for (auto &face : flist) {
    if (face[0] >= elems.size() || face[1] >= elems.size() || face[2] >= elems.size()) {
      res = -1;
      continue;
    }
    faces->addElement(elems[face[0]], elems[face[1]], elems[face[2]]);
  }
  return res;
}

int simit::loadMesh(istream &input, simit::Set<> *points,
                     simit::Set<3> *faces) {
  int res = 0;
  if (input) {
    vec4d vlist;
    vec3i flist;
    res += parse(input, &vlist, &flist);
    res += makeSet(vlist, flist, points, faces);
  } else {
    throw "Bad stream";
  }
  return res;
}

int simit::loadMesh(const string &path, Set<> *points, Set<3> *faces) {
  ifstream input(path);
  int res = 0;
  if (input) {
    res = loadMesh(input, points, faces);
  } else {
    throw "Unable to open " + path;
  }
  input.close();
  return res;
}

int simit::storeMesh(ostream &output, simit::Set<> &points,
                      simit::Set<3> &faces) {
  int res = 0;
  if (output) {
    FieldRef<int> id = points.getField<int>("id");
    FieldRef<double, 3> pos = points.getField<double, 3>("x");

    Set<>::ElementIterator it = points.begin();
    for (++it; it < points.end(); ++it) {
      auto point = *it;
      TensorRef<double, 3> x = pos.get(point);
      output << "v";
      for (int i = 0; i < 3; i++) {
        output << " " << x(i);
      }
      output<< "\n";
    }

    for (Set<3>::ElementIterator it = faces.begin(); it < faces.end(); it++) {
      output << "f";
      auto face = *it;
      for (int i = 0; i < 3; i++) {
        output << " " << id.get(faces.getEndpoint(face, i));
      }
      output << "\n";
    }
  } else {
    throw "Bad stream";
  }
  return res;
}

int simit::storeMesh(const string &path, simit::Set<> &points,
                      simit::Set<3> &faces) {
  ofstream output(path);
  int res = 0;
  if (output) {
    res = storeMesh(output, points, faces);
  } else {
    throw "Unable to open " + path;
  }
  output.close();
  return res;
}
