/*! \file main.cpp
        \brief A simple program to illustrate the use of Delaunay class.

	This program takes as input a set of points from a file and
	outputs the delaunay in geomview .off file format. It also
	prints the points and the faces of the delaunay triangulation
	using a iterator to the standard output.
 */


#include <vector>
#include <iostream>
#include <fstream>
#include <del_interface.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using std::ifstream;
using std::string;

using namespace tpp;

int main(int argc, char *argv[]) {

	if(argc != 2){
		cerr << "\nUsage: test input.dat\n";
		cerr << "input.dat should consist of 2d ascii values\n\n";
		exit(1);
	}

	ifstream inputFile(argv[1]);
	if(!inputFile){
		cerr << "Error: Could not open input file\n";
		exit(1);
	}

	Delaunay::Point tempP;
	vector< Delaunay::Point > v;

	string ofname = "del.off";
	cout <<"======================================\n";
	cout <<"      Input Points from file          \n";
	cout <<"======================================\n";
	do {
		inputFile >> tempP[0];
		inputFile >> tempP[1];
		if(!inputFile.good()) continue;
		v.push_back(tempP);
		cout << tempP << endl;
	} while (!inputFile.eof());
	cout <<"======================================\n";
	
	Delaunay delobject(v);
	delobject.Triangulate();
	cout << "Wrote del.off in geomview format\n";
	delobject.writeoff(ofname);


	cout << "Number of vertices in file      = " 
	     << delobject.nvertices() << endl;
	cout << "Number of edges in delaunay     = " 
	     << delobject.nedges() << endl;
	cout << "Number of triangles in delaunay = " 
	     << delobject.ntriangles() << endl;
	cout << "Hull size                       = " 
	     << delobject.hull_size() << endl;

	cout <<"======================================\n";
	cout <<"     Vertices from vertex iterator    \n";
	cout <<"======================================\n";
	for(Delaunay::vIterator vit = delobject.vbegin();
	    vit != delobject.vend();
	    ++ vit){
		cout << "ID : " << delobject.vertexId(vit) 
		     << "\t" << *vit << endl;
	}

	cout <<"======================================\n";	
	cout <<"     Faces from face iterator         \n";
	cout <<"======================================\n";
	for(Delaunay::fIterator fit  = delobject.fbegin(); 
	                        fit != delobject.fend(); 
	                      ++fit){
		cout << delobject.Org(fit)  << ", " 
			 << delobject.Dest(fit) << ", " 
			 << delobject.Apex(fit) << " \t: Area = " 
			 << delobject.area(fit) << endl;

		cout << "Adjacent vertices are: ";
		for(int i =0; i < 3; ++i)
			cout << delobject.Sym(fit,i) << "\t";
		cout << endl;
	}

	cout <<"======================================\n";	
	cout <<"     Testing point location           \n";
	cout <<"======================================\n";
	for(int i =0; i < delobject.nvertices(); ++i){
		Delaunay::fIterator fit = delobject.locate(i);
		cout << "Vertex " << i << " located in Triangle : " 
			 << delobject.Org(fit)  << ", " 
			 << delobject.Dest(fit) << ", " 
			 << delobject.Apex(fit) << " \t: Area = " 
			 << delobject.area(fit) << endl;
		
	}

	cout <<"======================================\n";	
	cout <<"     Testing vertex incidence         \n";
	cout <<"======================================\n";
	for (int j = 0 ; j < delobject.nvertices(); ++j){ 
		vector<int> vav;
		delobject.trianglesAroundVertex(j,vav);
		cout << "Triangles around vertex [" << j << "]:\n";
		for(unsigned int i =0; i < vav.size(); ++i){
			if( i % 3 == 0 ) cout << endl;
			cout << vav[i] << "\t";
		}
		cout << "\n---------------------------------\n";
	}

	inputFile.close();

	return 0;

}
