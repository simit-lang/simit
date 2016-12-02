#include "Thermal.h"

Thermal::Thermal(std::string paramFile, std::string CGNSFileName, std::string zoneName, int index_zone)
{
	//1- Construct the parameter manager
	PM.readParameters(paramFile);
	PM.set(TPM::CGNSFileName,CGNSFileName);
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
	Xsize=irmax[2]; Ysize=irmax[1];Zsize=irmax[2];
	//  read grid coordinates
	double x[irmax[2]][irmax[1]][irmax[0]];
	double y[irmax[2]][irmax[1]][irmax[0]];
	double z[irmax[2]][irmax[1]][irmax[0]];
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
	bcleft = new Set(*quads,*quads,*quads);
	bcright = new Set(*quads,*quads,*quads);
	bcup = new Set(*quads,*quads,*quads);
	bcbottom = new Set(*quads,*quads,*quads);
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
	FieldRef<double> qwinl  = bcleft->addField<double>("qwin");
	FieldRef<double> qwoutl  = bcleft->addField<double>("qwout");
	FieldRef<double> qwinr  = bcright->addField<double>("qwin");
	FieldRef<double> qwoutr  = bcright->addField<double>("qwout");
	FieldRef<double> qwinu  = bcup->addField<double>("qwin");
	FieldRef<double> qwoutu  = bcup->addField<double>("qwout");
	FieldRef<double> qwinb  = bcbottom->addField<double>("qwin");
	FieldRef<double> qwoutb  = bcbottom->addField<double>("qwout");
	FieldRef<double> Twinl  = bcleft->addField<double>("Twin");
	FieldRef<double> Twoutl  = bcleft->addField<double>("Twout");
	FieldRef<double> Twinr  = bcright->addField<double>("Twin");
	FieldRef<double> Twoutr  = bcright->addField<double>("Twout");
	FieldRef<double> Twinu  = bcup->addField<double>("Twin");
	FieldRef<double> Twoutu  = bcup->addField<double>("Twout");
	FieldRef<double> Twinb  = bcbottom->addField<double>("Twin");
	FieldRef<double> Twoutb  = bcbottom->addField<double>("Twout");

	// ONLY in 2D for now so zmax=1
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<Ysize; ++ydir) {
			for (int xdir=0; xdir<Xsize; ++xdir) {
				ElementRef point = points->add();
				pointRefs.push_back(point);
				xy.set(point, {x[xdir][ydir][zdir],y[xdir][ydir][zdir]});
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<Ysize-1; ++ydir) {
			for (int xdir=0; xdir<Xsize-1; ++xdir) {
				ElementRef quad = quads->add(pointRefs[ydir*Xsize+xdir],
											 pointRefs[ydir*Xsize+xdir+1],
											 pointRefs[(ydir+1)*Xsize+xdir+1],
											 pointRefs[(ydir+1)*Xsize+xdir]);
				quadsRefs.push_back(quad);
				T.set(quad,PM.get(TPM::T_init));
				K.set(quad,PM.get(TPM::K));
				rho.set(quad,PM.get(TPM::rho));
				cv.set(quad,PM.get(TPM::cv));
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<Ysize-1; ++ydir) {
			for (int xdir=0; xdir<Xsize-2; ++xdir) {
				ElementRef faceh = faces->add(quadsRefs[ydir*(Xsize-1)+xdir],
											  quadsRefs[ydir*(Xsize-1)+xdir+1]);
				dir.set(faceh,0);
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<Ysize-2; ++ydir) {
			for (int xdir=0; xdir<Xsize-1; ++xdir) {
				ElementRef facev = faces->add(quadsRefs[ydir*(Xsize-1)+xdir],
											  quadsRefs[ydir*(Xsize-1)+xdir+Xsize-1]);
				dir.set(facev,1);
			}
		}
	}
	for (int zdir=0; zdir<1; ++zdir) {
		for (int ydir=0; ydir<Ysize-1; ++ydir) {
			ElementRef bcl = bcleft->add(quadsRefs[ydir*(Xsize-1)],
										 quadsRefs[ydir*(Xsize-1)+1],
										 quadsRefs[ydir*(Xsize-1)+2]);
			qwinl.set(bcl,PM.get(TPM::qwl));
			ElementRef bcr = bcright->add(quadsRefs[ydir*(Xsize-1)+Xsize-2],
										  quadsRefs[ydir*(Xsize-1)+Xsize-3],
										  quadsRefs[ydir*(Xsize-1)+Xsize-4]);
			qwinr.set(bcr,PM.get(TPM::qwr));
		}
		for (int xdir=0; xdir<Xsize-1; ++xdir) {
			ElementRef bcu = bcup->add(quadsRefs[(Ysize-2)*(Xsize-1)+xdir],
									   quadsRefs[(Ysize-3)*(Xsize-1)+xdir],
									   quadsRefs[(Ysize-4)*(Xsize-1)+xdir]);
			qwinu.set(bcu,PM.get(TPM::qwu));
			ElementRef bcb = bcbottom->add(quadsRefs[xdir],
										   quadsRefs[xdir+Xsize-1],
										   quadsRefs[xdir+2*(Xsize-1)]);
			qwinb.set(bcb,PM.get(TPM::qwb));
		}
	}

	//4- Compile Simit program and bind arguments
	simit::init("cpu", sizeof(double));
	Program program;
	program.loadFile(PM.get(TPM::SimitFileName));
	dt(0)=0.0;						// dt timestep
	cfl(0)=PM.get(TPM::cfl);		// cfl
	coupling_direction(0)=PM.get(TPM::coupling_direction);
	solver_type(0)=PM.get(TPM::solver_type);
	solver_itermax(0)=PM.get(TPM::solver_itermax);
	bc_types={0,0,0,0};

	solve_thermal = program.compile("solve_thermal");
	bindSimitFunc(&solve_thermal);

	compute_dt = program.compile("compute_dt");
	bindSimitFunc(&compute_dt);

	flux_interface = program.compile("flux_interface");
	bindSimitFunc(&flux_interface);

	temperature_interface = program.compile("temperature_interface");
	bindSimitFunc(&temperature_interface);
}

Thermal::~Thermal() {
	// TODO Auto-generated destructor stub
}

void Thermal::bindSimitFunc(Function *simFunc){
	simFunc->bind("points", points);
	simFunc->bind("quads", quads);
	simFunc->bind("faces", faces);
	simFunc->bind("bcleft", bcleft);
	simFunc->bind("bcright", bcright);
	simFunc->bind("bcup", bcup);
	simFunc->bind("bcbottom", bcbottom);
	simFunc->bind("dt", &dt);
	simFunc->bind("cfl", &cfl);
	simFunc->bind("coupling_direction", &coupling_direction);
	simFunc->bind("solver_type", &solver_type);
	simFunc->bind("solver_itermax", &solver_itermax);
	simFunc->bind("bc_types", &bc_types);

	simFunc->init();
}

void Thermal::setBC_qw(Set *InterfaceIN){
	Set *InterfaceOUT;
	switch (coupling_direction(0)) {
		case 0 : InterfaceOUT = bcleft;  	break;
		case 1 : InterfaceOUT = bcright; 	break;
		case 2 : InterfaceOUT = bcbottom;	break;
		case 3 : InterfaceOUT = bcup; 		break;
	}
//	double Tw;
	FieldRef<double> qwin  = InterfaceOUT->getField<double>("qwin");
	FieldRef<double> qwout = InterfaceIN->getField<double>("qwout");
	for (std::pair<simit::Set::ElementIterator, simit::Set::ElementIterator> bc(InterfaceOUT->begin(),InterfaceIN->begin()); bc.first != InterfaceOUT->end(); ++bc.first,++bc.second) {
		qwin.set((ElementRef)*bc.first,-qwout.get((ElementRef)*bc.second));
//		Tw=InterfaceIN->getField<double>("Twout").get((ElementRef)*bc.second);
	}
//	std::cout << " TW " << Tw << std::endl;
}

void Thermal::setBC_Tw(Set *InterfaceIN){
	Set *InterfaceOUT;
	switch (coupling_direction(0)) {
		case 0 : InterfaceOUT = bcleft;  	break;
		case 1 : InterfaceOUT = bcright; 	break;
		case 2 : InterfaceOUT = bcbottom;	break;
		case 3 : InterfaceOUT = bcup; 		break;
	}
	FieldRef<double> Twin  = InterfaceOUT->getField<double>("Twin");
	FieldRef<double> Twout = InterfaceIN->getField<double>("Twout");
//	double Tw;
	for (std::pair<simit::Set::ElementIterator, simit::Set::ElementIterator> bc(InterfaceOUT->begin(),InterfaceIN->begin()); bc.first != InterfaceOUT->end(); ++bc.first,++bc.second) {
		Twin.set((ElementRef)*bc.first,Twout.get((ElementRef)*bc.second));
//		Tw=Twout.get((ElementRef)*bc.second);
	}
//	std::cout << " TW " << Tw << std::endl;
}

bool Thermal::compareTw(Set *InterfaceIN, double tolerance){
	Set *InterfaceOUT;
	switch (coupling_direction(0)) {
		case 0 : InterfaceOUT = bcleft;  	break;
		case 1 : InterfaceOUT = bcright; 	break;
		case 2 : InterfaceOUT = bcbottom;	break;
		case 3 : InterfaceOUT = bcup; 		break;
	}
	FieldRef<double> Twin  = InterfaceOUT->getField<double>("Twin");
	FieldRef<double> Twout = InterfaceIN->getField<double>("Twout");
	bool converged=true;
	for (std::pair<simit::Set::ElementIterator, simit::Set::ElementIterator> bc(InterfaceOUT->begin(),InterfaceIN->begin()); bc.first != InterfaceOUT->end(); ++bc.first,++bc.second) {
		if (abs( Twin.get((ElementRef)*bc.first)-Twout.get((ElementRef)*bc.second) )/Twin.get((ElementRef)*bc.first) > tolerance) {
			converged=false;
			break;
		}
	}
	return converged;
}

