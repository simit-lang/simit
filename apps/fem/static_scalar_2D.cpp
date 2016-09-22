#include "graph.h"
#include "program.h"
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;
using namespace simit;

/************************************************************************//**
 * \brief Make a uniform mesh of n x m gridpoints
 * \param n        [in]  The number of points in x-direction
 * \param m        [in]  The number of points in x-direction
 * \param node     [out] Simit set to hold the nodes (3 fields: x,u and edge)
 * \param line     [out] Simit set to hold the lines (connects two nodes)
 * \param triangle [out] Simit set to hold the elements (connects 3 nodes)
 ***************************************************************************/
void make_mesh(int n, int m, Set& node, Set& line, Set& triangle) {
    FieldRef<double, 2> x    = node.addField<double,2>("x"   ); // (x,y)-coordinate
    FieldRef<double>    u    = node.addField<double>(  "u"   ); // solution u
    FieldRef<bool>      edge = node.addField<bool>(    "edge"); // boundary node
    vector<vector<ElementRef> > ref_node(n, vector<ElementRef>(m));

    // create a uniform mesh of n x m gridpoints over the unit square [0,1]x[0,1]
    for(int j=0; j<m; j++) {
        for(int i=0; i<n; i++) {
            ref_node[i][j]    = node.add();
            x(ref_node[i][j]) = { 1.0*i/(n-1), 1.0*j/(m-1) };
        }
    }

    // connect gridpoints by making triangles of them
    // 
    //  (i,j+1)       (i+1, j+1)
    //       +-------+
    //       |      /|
    //       |    ,' |
    //       |   /   |
    //       | ,'    |
    //       +-------+ 
    //  (i,j)       (i+1, j)
    //
    for(int j=0; j<m-1; j++) {
        for(int i=0; i<n-1; i++) {
            ElementRef node1 = ref_node[ i ][ j ];
            ElementRef node2 = ref_node[i+1][ j ];
            ElementRef node3 = ref_node[i+1][j+1];
            ElementRef element = triangle.add(node1, node2, node3);

            node1 = ref_node[i+1][j+1];
            node2 = ref_node[ i ][j+1];
            node3 = ref_node[ i ][ j ];
            element = triangle.add(node1, node2, node3);
        }
    }

    // identify boundary nodes by creating clockwise ordered line pieces between them
    //
    //      |         |        |
    //   ---+---------+--------+------>
    //    (i,j)    (i+1,j)   (i+2,j)
    //
    for(int i=0; i<n-1; i++) 
        line.add(ref_node[i][0], ref_node[i+1][0]);      // south edge
    for(int i=0; i<n-1; i++) 
        line.add(ref_node[i+1][m-1], ref_node[i][m-1]);  // north edge
    for(int j=0; j<m-1; j++) 
        line.add(ref_node[0][j+1], ref_node[0][j]);      // west edge
    for(int j=0; j<m-1; j++) 
        line.add(ref_node[n-1][j], ref_node[n-1][j+1]);  // east edge
}

/************************************************************************//**
 * \brief Dump results as a surface plot in an ASCII-stl file
 * \param filename [in] STL-filename. Use for instance meshlab to view this later
 * \param node     [in] Simit set to hold the nodes
 * \param triangle [in] Simit set to hold the elements (connects 3 nodes)
 ***************************************************************************/
void plot_results(string filename, Set& node, Set& triangle) {
    ofstream out(filename);
    out << "solid simit" << endl;
    FieldRef<double>   u_ref = node.getField<double>("u");
    FieldRef<double,2> x_ref = node.getField<double,2>("x");
    for(auto t : triangle) {
        out << "facet normal 0 0 1" << endl;
        out << "outer loop" << endl;
        for(int i=0; i<3; i++) {
            ElementRef pt = triangle.getEndpoint(t, i);
            double u = u_ref(pt);
            double x = x_ref(pt)(0);
            double y = x_ref(pt)(1);
            out << "vertex " << x << " " << y << " " << u << endl;
        }
        out << "endloop" << endl;
        out << "endfacet" << endl;
    }
    out << "endsolid simit";
    out.close();
}




/************************************************************************//**
 * \brief Main function. Read simit program and run on unit square
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

    // make the mesh (15x15 gridpoints)
    Set node;
    Set line(node,node);
    Set tri(node,node,node);
    make_mesh(15,15, node, line, tri);

    // read the simit program and compile it
    Program program;
    program.loadFile(program_file);
    Function func = program.compile("main");
    func.bind("verts", &node);
    func.bind("line",  &line);
    func.bind("tri",   &tri);

    // run the program
    func.runSafe();

    // write results to file
    plot_results("results.stl", node, tri);
    cout << "Results written to \"results.stl\"" << endl;
} 
