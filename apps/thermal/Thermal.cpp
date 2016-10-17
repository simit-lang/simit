#include "Thermal.h"

Thermal::Thermal(std::string paramFile, std::string zoneName, int index_zone)
{
	//1- Construct the parameter manager
	PM.readParameters(paramFile);
	std::cout << "---- " << zoneName << " PARAMETERS LIST : " << std::endl << PM;

	//2- Load CGNS mesh data
	int index_file, index_base;
	cgsize_t isize[3][3];
	cgsize_t irmin[3],irmax[3];
	std::string fieldName;
	if (cg_open(PM.get(TPM::CGNSFileName).c_str(),CG_MODE_READ,&index_file)) cg_error_exit();
	//  we know there is only one base (real working code would check!)
	index_base=1;
	cg_zone_read(index_file,index_base,index_zone,(char *)zoneName.c_str(),*isize);
	//  lower range index
	irmin[0]=1; irmin[1]=1; irmin[2]=1;
	//  upper range index of vertices
	irmax[0]=isize[0][0];irmax[1]=isize[0][1];irmax[2]=isize[0][2];
	//  read grid coordinates
	double x[irmax[2]][irmax[1]][irmax[0]]; //[irmax[0]][irmax[1]][irmax[2]];
	double y[irmax[2]][irmax[1]][irmax[0]]; //[irmax[0]][irmax[1]][irmax[2]];
	double z[irmax[2]][irmax[1]][irmax[0]]; //[irmax[0]][irmax[1]][irmax[2]];
	fieldName="CoordinateX";
	if (cg_coord_read(index_file,index_base,index_zone,(char *)fieldName.c_str(),RealDouble,irmin,irmax,x)) cg_error_exit();
	fieldName="CoordinateY";
	if (cg_coord_read(index_file,index_base,index_zone,(char *)fieldName.c_str(),RealDouble,irmin,irmax,y)) cg_error_exit();
	fieldName="CoordinateZ";
	if (cg_coord_read(index_file,index_base,index_zone,(char *)fieldName.c_str(),RealDouble,irmin,irmax,z)) cg_error_exit();
	//   close CGNS file
	cg_close(index_file);

	//3- Create a graph and initialize it with mesh data
	points = new Set;
	quads = new Set(*points,*points,*points,*points);
	faces = new Set(*quads,*quads);
	bcleft = new Set(*quads);
	bcright = new Set(*quads);
	bcup = new Set(*quads);
	bcbottom = new Set(*quads);
	std::vector<ElementRef> pointRefs;
	std::vector<ElementRef> quadsRefs;

	// The fields of the points set
	FieldRef<double,2> xy    = points->addField<double,2>("xy");

	// The fields of the quads set
	FieldRef<double> T  = quads->addField<double>("T");
	FieldRef<double> K  = quads->addField<double>("K");
	FieldRef<double> rho  = quads->addField<double>("rho");
	FieldRef<double> cv  = quads->addField<double>("cv");
	FieldRef<double,2> dxy  = quads->addField<double,2>("dxy");
	FieldRef<double> gamma  = quads->addField<double>("gamma");

	// The fields of the faces set
	FieldRef<int> dir  = faces->addField<int>("dir");
	FieldRef<double> Khalf  = faces->addField<double>("Khalf");
	FieldRef<double> dxyhalf  = faces->addField<double>("dxyhalf");

	// The fields of the boundary conditions sets
	FieldRef<double> qwl  = bcleft->addField<double>("qw");
	FieldRef<double> qwr  = bcright->addField<double>("qw");
	FieldRef<double> qwu  = bcup->addField<double>("qw");
	FieldRef<double> qwb  = bcbottom->addField<double>("qw");


	// ONLY in 2D for now so zmax=1
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<irmax[1]; ++ydir) {
			for (int xdir=0; xdir<irmax[2]; ++xdir) {
				ElementRef point = points->add();
				pointRefs.push_back(point);
				xy.set(point, {x[xdir][ydir][zdir],y[xdir][ydir][zdir]});
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<irmax[1]-1; ++ydir) {
			for (int xdir=0; xdir<irmax[2]-1; ++xdir) {
				ElementRef quad = quads->add(pointRefs[ydir*irmax[2]+xdir],
						pointRefs[ydir*irmax[2]+xdir+1],
						pointRefs[(ydir+1)*irmax[2]+xdir+1],
						pointRefs[(ydir+1)*irmax[2]+xdir]);
				quadsRefs.push_back(quad);
				T.set(quad,PM.get(TPM::T_init));
				K.set(quad,PM.get(TPM::K));
				rho.set(quad,PM.get(TPM::rho));
				cv.set(quad,PM.get(TPM::cv));
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<irmax[1]-1; ++ydir) {
			for (int xdir=0; xdir<irmax[2]-2; ++xdir) {
				ElementRef faceh = faces->add(quadsRefs[ydir*(irmax[2]-1)+xdir],
						quadsRefs[ydir*(irmax[2]-1)+xdir+1]);
				dir.set(faceh,0);
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<irmax[1]-2; ++ydir) {
			for (int xdir=0; xdir<irmax[2]-1; ++xdir) {
				ElementRef facev = faces->add(quadsRefs[ydir*(irmax[2]-1)+xdir],
						quadsRefs[ydir*(irmax[2]-1)+xdir+irmax[2]-1]);
				dir.set(facev,1);
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<irmax[1]-1; ++ydir) {
			ElementRef bcl = bcleft->add(quadsRefs[ydir*(irmax[2]-1)]);
			qwl.set(bcl,PM.get(TPM::qwl));
			ElementRef bcr = bcright->add(quadsRefs[ydir*(irmax[2]-1)+irmax[2]-2]);
			qwr.set(bcr,PM.get(TPM::qwr));
		}
		for (int xdir=0; xdir<irmax[2]-1; ++xdir) {
			ElementRef bcu = bcup->add(quadsRefs[(irmax[1]-2)*(irmax[2]-1)+xdir]);
			qwu.set(bcu,PM.get(TPM::qwu));
			ElementRef bcb = bcbottom->add(quadsRefs[xdir]);
			if ((xdir<irmax[2]/4) || (xdir>(irmax[2]-irmax[2]/4)))
				qwb.set(bcb,PM.get(TPM::qwb)/2);
			else
				qwb.set(bcb,PM.get(TPM::qwb));
		}
	}

	//4- Compile Simit program and bind arguments
	simit::init("cpu", sizeof(double));
	Program program;
	program.loadFile(PM.get(TPM::SimitFileName));
	dt(0)=0.0;						// dt timestep
	cfl(0)=PM.get(TPM::cfl);		// cfl

	solve_thermal = program.compile("solve_thermal");
	solve_thermal.bind("points", points);
	solve_thermal.bind("quads", quads);
	solve_thermal.bind("faces", faces);
	solve_thermal.bind("bcleft", bcleft);
	solve_thermal.bind("bcright", bcright);
	solve_thermal.bind("bcup", bcup);
	solve_thermal.bind("bcbottom", bcbottom);
	solve_thermal.bind("dt", &dt);
	solve_thermal.bind("cfl", &cfl);

	solve_thermal.init();

	compute_dt = program.compile("compute_dt");
	compute_dt.bind("points", points);
	compute_dt.bind("quads", quads);
	compute_dt.bind("faces", faces);
	compute_dt.bind("bcleft", bcleft);
	compute_dt.bind("bcright", bcright);
	compute_dt.bind("bcup", bcup);
	compute_dt.bind("bcbottom", bcbottom);
	compute_dt.bind("dt", &dt);
	compute_dt.bind("cfl", &cfl);

	compute_dt.init();
}

Thermal::~Thermal() {
	// TODO Auto-generated destructor stub
}

