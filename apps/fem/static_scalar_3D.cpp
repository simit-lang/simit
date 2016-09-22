#include "graph.h"
#include "program.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace simit;

/************************************************************************//**
 * \brief Make a uniform mesh of n[0] x n[1] x n[2] gridpoints
 * \param n        [in]  The number of points in x-,y- and z-direction
 * \param node     [out] Simit set to hold the nodes (3 fields: x,u and edge)
 * \param tetrahed [out] Simit set to hold the tetrahedrals (connects 4 nodes)
 ***************************************************************************/
template<typename three_int>
void make_mesh(three_int n, Set& node, Set& tetrahed) {
    FieldRef<double, 3> x    = node.addField<double,3>("x"   ); // (x,y)-coordinate
    FieldRef<double>    u    = node.addField<double>(  "u"   ); // solution u
    FieldRef<bool>      edge = node.addField<bool>(    "edge"); // boundary node
    FieldRef<int>       ind  = node.addField<int>(     "i");    // global index
    vector<vector<vector<ElementRef> > > ref_node(n[0], vector<vector<ElementRef> >(n[1], vector<ElementRef>(n[2])));

    // create a uniform mesh over the unit cube [0,1]x[0,1]x[0,1]
    int counter = 0;
    for(int k=0; k<n[2]; k++) {
        for(int j=0; j<n[1]; j++) {
            for(int i=0; i<n[0]; i++) {
                ref_node[i][j][k]    = node.add();
                x(ref_node[i][j][k]) = { 1.0*i/(n[0]-1),
                                         1.0*j/(n[1]-1),
                                         1.0*k/(n[2]-1) };
                edge(ref_node[i][j][k]) = (i==0 || i==n[0]-1 ||
                                           j==0 || j==n[1]-1 ||
                                           k==0 || k==n[2]-1    );
                ind(ref_node[i][j][k]) = counter++;
            }
        }
    }

    // connect gridpoints by making tetrahedrals of them
    // easiest would be to use 8-node hexahedral elements, but lets do tets 
    int h2t[][4][3] = {{{0,0,0},{1,0,0},{0,1,0},{1,0,1}},
                       {{0,0,0},{1,0,1},{0,1,1},{0,0,1}},
                       {{0,0,0},{0,1,0},{0,1,1},{1,0,1}},
                       {{1,0,0},{0,1,0},{1,1,0},{1,1,1}},
                       {{1,0,1},{1,0,0},{1,1,1},{0,1,0}}, // hex-to-tet transformation
                       {{0,1,0},{1,1,1},{0,1,1},{1,0,1}}};// 6 tets in one hex
    for(int k=0; k<n[2]-1; k++) {
        for(int j=0; j<n[1]-1; j++) {
            for(int i=0; i<n[0]-1; i++) {
                ElementRef corner[4];
                for(int tet=0; tet<6; tet++) {
                    for(int m=0; m<4; m++) {
                        int *o = h2t[tet][m]; // index offset
                        corner[m] = ref_node[ i+o[0] ][ j+o[1] ][ k+o[2] ];
                    }
                    tetrahed.add(corner[0], corner[1], corner[2], corner[3]);
                }
            }
        }
    }
}

/************************************************************************//**
 * \brief Dump results as a surface plot in an ASCII-vtk file
 * \param filename    [in] VTK-filename. Use paraview to view this later
 * \param node        [in] Simit set to hold the nodes
 * \param tetrahedron [in] Simit set to hold the elements (connects 4 nodes)
 ***************************************************************************/
void plot_results(string filename, Set& node, Set& tetrahedron) {
    // full documentation of the VTK file format can be found at
    // http://www.vtk.org/wp-content/uploads/2015/04/file-formats.pdf
    
    // fetch fields
    FieldRef<double>   u = node.getField<double>(  "u");
    FieldRef<int>      i = node.getField<int>(     "i");
    FieldRef<double,3> x = node.getField<double,3>("x");

    // write header
    ofstream out(filename);
    out << "# vtk DataFile Version 3.0" << endl;
    out << "simit results " << endl;
    out << "ASCII " << endl;
    out << "DATASET UNSTRUCTURED_GRID " << endl << endl;

    // write node coordinates 
    out << "POINTS " << node.getSize() << " float" << endl;
    for(auto n : node)
        out << x(n)(0) << " " << x(n)(1) << " " << x(n)(2) << endl;
    out << endl;

    // write element connections
    out << "CELLS " << tetrahedron.getSize() << " " << tetrahedron.getSize()*5 << endl;
    for(auto t : tetrahedron) {
        out << "4 ";
        for(int j=0; j<4; j++) {
            ElementRef pt = tetrahedron.getEndpoint(t, j);
            out << i(pt) << " ";
        }
        out << endl;
    }
    out << endl;

    // tag all elements as tetrahedrals
    out << "CELL_TYPES " << tetrahedron.getSize() << endl;
    for(auto t : tetrahedron)
        out << "10" << endl; // VTK_TETRA element type
    out << endl;

    // write FEM solution
    out << "POINT_DATA " << node.getSize() << endl;
    out << endl;
    out << "SCALARS solution float" << endl;
    out << "LOOKUP_TABLE default" << endl;
    for(auto n : node)
        out << u(n) << endl;
    out << endl;

}




/************************************************************************//**
 * \brief Main function. Read simit program and run on unit cube
 ***************************************************************************/
int main(int argc, char **argv) {
    // initalize simit to use CPU for computation
    init("cpu", sizeof(double));

    if(argc != 2) { // request simit program as commandline argument
        cerr << "File usage:" << endl;
        cerr << argv[0] << " <simit-program>" << endl;
        exit(1);
    }
    string program_file = argv[1];

    // make the mesh (8x8x8 gridpoints)
    Set node;
    Set tet(node,node,node,node);
	int n[] = {8,8,8};
    make_mesh(n, node, tet);

    // read the simit program and compile it
    Program program;
    program.loadFile(program_file);
    Function func = program.compile("main");
    func.bind("verts", &node);
    func.bind("tets",  &tet);

    // run the program
    func.runSafe();

    // write results to file
    plot_results("results.vtk", node, tet);
    cout << "Results written to \"results.vtk\"" << endl;
} 
