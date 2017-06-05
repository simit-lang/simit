#include "Thermal.h"

Thermal::Thermal(std::string paramFile, std::string CGNSFileName_L0,
                 std::string CGNSFileName_L1, std::string zoneName,
                 int index_zone)
{
  //1- Construct the parameter manager
  PM.readParameters(paramFile);
  PM.set(TPM::CGNSFileName_1,CGNSFileName_L1);
  PM.set(TPM::CGNSFileName_0,CGNSFileName_L0);
  std::string filenames[2];
  filenames[0]=CGNSFileName_L0;
  filenames[1]=CGNSFileName_L1;
  std::cout << "---- " << zoneName << " PARAMETERS LIST : " << std::endl << PM;

  std::array<std::vector<ElementRef>,2> pointRefs;
  // The fields of the points set
  std::vector<FieldRef<double,2>> xy_MG;

  // The fields of the quads set
  std::vector<FieldRef<double>> T_MG;
  std::vector<FieldRef<double>> K_MG;
  std::vector<FieldRef<double>> rho_MG;
  std::vector<FieldRef<double>> cv_MG;
  std::vector<FieldRef<double,2>> dxy_MG;
  std::vector<FieldRef<double>> gamma_MG;

  // The fields of the faces set
  std::vector<FieldRef<int>> dir_MG;
  std::vector<FieldRef<double>> Khalf_MG;
  std::vector<FieldRef<double>> dxyhalf_MG;

  // The fields of the boundary conditions sets
  std::vector<FieldRef<double>> qwinl_MG;
  std::vector<FieldRef<double>> qwoutl_MG;
  std::vector<FieldRef<double>> qwinr_MG;
  std::vector<FieldRef<double>> qwoutr_MG;
  std::vector<FieldRef<double>> qwinu_MG;
  std::vector<FieldRef<double>> qwoutu_MG;
  std::vector<FieldRef<double>> qwinb_MG;
  std::vector<FieldRef<double>> qwoutb_MG;
  std::vector<FieldRef<double>> Twinl_MG;
  std::vector<FieldRef<double>> Twoutl_MG;
  std::vector<FieldRef<double>> Twinr_MG;
  std::vector<FieldRef<double>> Twoutr_MG;
  std::vector<FieldRef<double>> Twinu_MG;
  std::vector<FieldRef<double>> Twoutu_MG;
  std::vector<FieldRef<double>> Twinb_MG;
  std::vector<FieldRef<double>> Twoutb_MG;

  //3- Create a graph and initialize it with mesh data
  for(int i=0; i<2; i++) {
    points_MG[i] = new Set;
    quads_MG[i] = new Set(*points_MG[i],*points_MG[i],*points_MG[i],*points_MG[i]);
    faces_MG[i] = new Set(*quads_MG[i],*quads_MG[i]);
    bcleft_MG[i] = new Set(*quads_MG[i],*quads_MG[i]);
    bcright_MG[i] = new Set(*quads_MG[i],*quads_MG[i]);
    bcup_MG[i] = new Set(*quads_MG[i],*quads_MG[i]);
    bcbottom_MG[i] = new Set(*quads_MG[i],*quads_MG[i]);

    // The fields of the points set
    FieldRef<double,2> xy_loc  = points_MG[i]->addField<double,2>("xy");
    xy_MG.push_back(xy_loc);

    // The fields of the quads set
    FieldRef<double> T_loc  = quads_MG[i]->addField<double>("T");
    T_MG.push_back(T_loc);
    FieldRef<double> K_loc  = quads_MG[i]->addField<double>("K");
    K_MG.push_back(K_loc);
    FieldRef<double> rho_loc  = quads_MG[i]->addField<double>("rho");
    rho_MG.push_back(rho_loc);
    FieldRef<double> cv_loc  = quads_MG[i]->addField<double>("cv");
    cv_MG.push_back(cv_loc);
    FieldRef<double,2> dxy_loc  = quads_MG[i]->addField<double,2>("dxy");
    dxy_MG.push_back(dxy_loc);
    FieldRef<double> gamma_loc  = quads_MG[i]->addField<double>("gamma");
    gamma_MG.push_back(gamma_loc);

    // The fields of the faces set
    FieldRef<int> dir_loc  = faces_MG[i]->addField<int>("dir");
    dir_MG.push_back(dir_loc);
    FieldRef<double> Khalf_loc  = faces_MG[i]->addField<double>("Khalf");
    Khalf_MG.push_back(Khalf_loc);
    FieldRef<double> dxyhalf_loc  = faces_MG[i]->addField<double>("dxyhalf");
    dxyhalf_MG.push_back(dxyhalf_loc);

    // The fields of the boundary conditions sets
    FieldRef<double> qwinl_loc  = bcleft_MG[i]->addField<double>("qwin");
    qwinl_MG.push_back(qwinl_loc);
    FieldRef<double> qwoutl_loc  = bcleft_MG[i]->addField<double>("qwout");
    qwoutl_MG.push_back(qwoutl_loc);
    FieldRef<double> qwinr_loc  = bcright_MG[i]->addField<double>("qwin");
    qwinr_MG.push_back(qwinr_loc);
    FieldRef<double> qwoutr_loc  = bcright_MG[i]->addField<double>("qwout");
    qwoutr_MG.push_back(qwoutr_loc);
    FieldRef<double> qwinu_loc  = bcup_MG[i]->addField<double>("qwin");
    qwinu_MG.push_back(qwinu_loc);
    FieldRef<double> qwoutu_loc  = bcup_MG[i]->addField<double>("qwout");
    qwoutu_MG.push_back(qwoutu_loc);
    FieldRef<double> qwinb_loc  = bcbottom_MG[i]->addField<double>("qwin");
    qwinb_MG.push_back(qwinb_loc);
    FieldRef<double> qwoutb_loc  = bcbottom_MG[i]->addField<double>("qwout");
    qwoutb_MG.push_back(qwoutb_loc);
    FieldRef<double> Twinl_loc  = bcleft_MG[i]->addField<double>("Twin");
    Twinl_MG.push_back(Twinl_loc);
    FieldRef<double> Twoutl_loc  = bcleft_MG[i]->addField<double>("Twout");
    Twoutl_MG.push_back(Twoutl_loc);
    FieldRef<double> Twinr_loc  = bcright_MG[i]->addField<double>("Twin");
    Twinr_MG.push_back(Twinr_loc);
    FieldRef<double> Twoutr_loc  = bcright_MG[i]->addField<double>("Twout");
    Twoutr_MG.push_back(Twoutr_loc);
    FieldRef<double> Twinu_loc  = bcup_MG[i]->addField<double>("Twin");
    Twinu_MG.push_back(Twinu_loc);
    FieldRef<double> Twoutu_loc  = bcup_MG[i]->addField<double>("Twout");
    Twoutu_MG.push_back(Twoutu_loc);
    FieldRef<double> Twinb_loc  = bcbottom_MG[i]->addField<double>("Twin");
    Twinb_MG.push_back(Twinb_loc);
    FieldRef<double> Twoutb_loc  = bcbottom_MG[i]->addField<double>("Twout");
    Twoutb_MG.push_back(Twoutb_loc);

    //2- Load CGNS mesh data
    int index_file, index_base;
    cgsize_t isize[3][3];
    cgsize_t irmin[3],irmax[3];
    std::string fieldName;
    if (cg_open(filenames[i].c_str(),CG_MODE_READ,&index_file))
      cg_error_exit();
    //  we know there is only one base (real working code would check!)
    index_base=1;
    cg_zone_read(index_file,index_base,index_zone,(char *)zoneName.c_str(),*isize);
    //  lower range index
    irmin[0]=1; irmin[1]=1; irmin[2]=1;
    //  upper range index of vertices
    irmax[0]=isize[0][0];irmax[1]=isize[0][1];irmax[2]=isize[0][2];
    Xsize[i]=irmax[2]; Ysize[i]=irmax[1];Zsize[i]=irmax[2];
    //  read grid coordinates
    double x[irmax[2]][irmax[1]][irmax[0]];
    double y[irmax[2]][irmax[1]][irmax[0]];
    double z[irmax[2]][irmax[1]][irmax[0]];
    fieldName="CoordinateX";
    if (cg_coord_read(index_file,index_base,index_zone,(char *)fieldName.c_str(),RealDouble,irmin,irmax,x))
      cg_error_exit();
    fieldName="CoordinateY";
    if (cg_coord_read(index_file,index_base,index_zone,(char *)fieldName.c_str(),RealDouble,irmin,irmax,y))
      cg_error_exit();
    fieldName="CoordinateZ";
    if (cg_coord_read(index_file,index_base,index_zone,(char *)fieldName.c_str(),RealDouble,irmin,irmax,z))
      cg_error_exit();
    //   close CGNS file
    // ONLY in 2D for now so zmax=1
    for (int zdir=0; zdir<1; ++zdir) {
      for (int ydir=0; ydir<Ysize[i]; ++ydir) {
        for (int xdir=0; xdir<Xsize[i]; ++xdir) {
          ElementRef point = points_MG[i]->add();
          pointRefs[i].push_back(point);
          xy_MG[i].set(point, {x[xdir][ydir][zdir],y[xdir][ydir][zdir]});
        }
      }
    }
    cg_close(index_file);


    for (int zdir=0; zdir<1; ++zdir) {
      for (int ydir=0; ydir<Ysize[i]-1; ++ydir) {
        for (int xdir=0; xdir<Xsize[i]-1; ++xdir) {
          ElementRef quad = quads_MG[i]->add(pointRefs[i][ydir*Xsize[i]+xdir],
                                             pointRefs[i][ydir*Xsize[i]+xdir+1],
                                             pointRefs[i][(ydir+1)*Xsize[i]+xdir+1],
                                             pointRefs[i][(ydir+1)*Xsize[i]+xdir]);

          quadsRefs_MG[i].push_back(quad);
          T_MG[i].set(quad,PM.get(TPM::T_init));
          if (xdir<Xsize[i]/2)
            K_MG[i].set(quad,PM.get(TPM::K));
          else
            K_MG[i].set(quad,PM.get(TPM::K)*2);
          rho_MG[i].set(quad,PM.get(TPM::rho));
          cv_MG[i].set(quad,PM.get(TPM::cv));
        }
      }
    }

    for (int zdir=0; zdir<1; ++zdir) {
      for (int ydir=0; ydir<Ysize[i]-1; ++ydir) {
        for (int xdir=0; xdir<Xsize[i]-2; ++xdir) {
          ElementRef faceh = faces_MG[i]->add(quadsRefs_MG[i][ydir*(Xsize[i]-1)+xdir],
                                              quadsRefs_MG[i][ydir*(Xsize[i]-1)+xdir+1]);
          dir_MG[i].set(faceh,0);
        }
      }
    }
    for (int zdir=0; zdir<1; ++zdir) {
      for (int ydir=0; ydir<Ysize[i]-2; ++ydir) {
        for (int xdir=0; xdir<Xsize[i]-1; ++xdir) {
          ElementRef facev = faces_MG[i]->add(quadsRefs_MG[i][ydir*(Xsize[i]-1)+xdir],
                                              quadsRefs_MG[i][ydir*(Xsize[i]-1)+xdir+Xsize[i]-1]);
          dir_MG[i].set(facev,1);
        }
      }
    }
    for (int zdir=0; zdir<1; ++zdir) {
      for (int ydir=0; ydir<Ysize[i]-1; ++ydir) {
        ElementRef bcl = bcleft_MG[i]->add(quadsRefs_MG[i][ydir*(Xsize[i]-1)],
                                           quadsRefs_MG[i][ydir*(Xsize[i]-1)+1]); //,
        //			 quadsRefs[ydir*(Xsize[i]-1)+2]);
        ElementRef bcr = bcright_MG[i]->add(quadsRefs_MG[i][ydir*(Xsize[i]-1)+Xsize[i]-2],
                                            quadsRefs_MG[i][ydir*(Xsize[i]-1)+Xsize[i]-3]); //,
        //			 quadsRefs[ydir*(Xsize[i]-1)+Xsize[i]-4]);
        if ((ydir<Ysize[i]/4) || (3*ydir>Ysize[i]/4)) {
          qwinr_MG[i].set(bcr,PM.get(TPM::qwr)*2);
          qwinl_MG[i].set(bcl,PM.get(TPM::qwl));
        }
        else {
          qwinr_MG[i].set(bcr,PM.get(TPM::qwr));
          qwinl_MG[i].set(bcl,PM.get(TPM::qwl)*2);
        }
      }
      for (int xdir=0; xdir<Xsize[i]-1; ++xdir) {
        ElementRef bcu = bcup_MG[i]->add(quadsRefs_MG[i][(Ysize[i]-2)*(Xsize[i]-1)+xdir],
                                         quadsRefs_MG[i][(Ysize[i]-3)*(Xsize[i]-1)+xdir]); //,
        //			   quadsRefs[(Ysize[i]-4)*(Xsize[i]-1)+xdir]);
        qwinu_MG[i].set(bcu,PM.get(TPM::qwu));
        ElementRef bcb = bcbottom_MG[i]->add(quadsRefs_MG[i][xdir],
                                             quadsRefs_MG[i][xdir+Xsize[i]-1]); //,
        //			   quadsRefs[xdir+2*(Xsize[i]-1)]);
        if ((xdir<Xsize[i]/4) || (3*xdir>Xsize[i]/4)) {
          qwinb_MG[i].set(bcb,PM.get(TPM::qwb));
          qwinu_MG[i].set(bcu,PM.get(TPM::qwu)*2);
        }
        else {
          qwinb_MG[i].set(bcb,PM.get(TPM::qwb)*2);
          qwinu_MG[i].set(bcu,PM.get(TPM::qwu));
        }
      }
    }
  }

  links = new Set(*quads_MG[0],*quads_MG[1],
                  *quads_MG[1],*quads_MG[1],*quads_MG[1],*quads_MG[1],
                  *quads_MG[1],*quads_MG[1],*quads_MG[1],*quads_MG[1]);
  std::vector<ElementRef> linksRefs;

  for (int zdir=0; zdir<1; ++zdir) {
    for (int ydir=0; ydir<Ysize[0]-1; ++ydir) {
      for (int xdir=0; xdir<Xsize[0]-1; ++xdir) {
        //	int n = 2*(ydir*(Xsize[0]-1)+xdir)+ydir*(Xsize[1]-1);
        int n = (2*ydir+1)*(Xsize[1]-1)+2*xdir+1;
        //	std::cout << n << std::endl;
        ElementRef link = links->add(quadsRefs_MG[0][ydir*(Xsize[0]-1)+xdir],
                                     quadsRefs_MG[1][n],
                                     quadsRefs_MG[1][n-Xsize[1]+1],
                                     quadsRefs_MG[1][n-1],
                                     quadsRefs_MG[1][n+1],
                                     quadsRefs_MG[1][n+Xsize[1]-1],
                                     quadsRefs_MG[1][n-Xsize[1]],
                                     quadsRefs_MG[1][n-Xsize[1]+2],
                                     quadsRefs_MG[1][n+Xsize[1]-2],
                                     quadsRefs_MG[1][n+Xsize[1]]);
        //	std::cout << n << " " << n-Xsize[1]+1 << " " << n-1 << " " << n+1 << " " << n+Xsize[1]-1 << std::endl;
        //	std::cout << n-Xsize[1] << " " << n-Xsize[1]+2 << " " << n+Xsize[1]-2 << " " << n+Xsize[1] << std::endl;
        linksRefs.push_back(link);
      }
    }
  }

  //4- Compile Simit program and bind arguments
  simit::init("cpu", sizeof(double));
  Program program;
  program.loadFile(PM.get(TPM::SimitFileName));
  dt(0)=0.0;				// dt timestep
  cfl(0)=PM.get(TPM::cfl);		// cfl
  coupling_direction(0)=PM.get(TPM::coupling_direction);
  solver_type(0)=PM.get(TPM::solver_type);
  solver_itermax(0)=PM.get(TPM::solver_itermax);
  solver_tolerance(0)=PM.get(TPM::solver_tolerance);
  smoother_iter(0)=PM.get(TPM::smoother_iter);
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
  simFunc->bind("points_MG1", points_MG[1]);
  simFunc->bind("quads_MG1", quads_MG[1]);
  simFunc->bind("faces_MG1", faces_MG[1]);
  simFunc->bind("bcleft_MG1", bcleft_MG[1]);
  simFunc->bind("bcright_MG1", bcright_MG[1]);
  simFunc->bind("bcup_MG1", bcup_MG[1]);
  simFunc->bind("bcbottom_MG1", bcbottom_MG[1]);
  simFunc->bind("points_MG0", points_MG[0]);
  simFunc->bind("quads_MG0", quads_MG[0]);
  simFunc->bind("faces_MG0", faces_MG[0]);
  simFunc->bind("bcleft_MG0", bcleft_MG[0]);
  simFunc->bind("bcright_MG0", bcright_MG[0]);
  simFunc->bind("bcup_MG0", bcup_MG[0]);
  simFunc->bind("bcbottom_MG0", bcbottom_MG[0]);
  simFunc->bind("dt", &dt);
  simFunc->bind("cfl", &cfl);
  simFunc->bind("coupling_direction", &coupling_direction);
  simFunc->bind("solver_type", &solver_type);
  simFunc->bind("solver_itermax", &solver_itermax);
  simFunc->bind("solver_tolerance", &solver_tolerance);
  simFunc->bind("smoother_iter", &smoother_iter);
  simFunc->bind("bc_types", &bc_types);
  simFunc->bind("links", links);
  simFunc->init();
}

void Thermal::setBC_qw(Set *InterfaceIN, int i){
  Set *InterfaceOUT;
  switch (coupling_direction(0)) {
    case 0 : InterfaceOUT = bcleft_MG[i];  	break;
    case 1 : InterfaceOUT = bcright_MG[i]; 	break;
    case 2 : InterfaceOUT = bcbottom_MG[i];	break;
    case 3 : InterfaceOUT = bcup_MG[i]; 		break;
  }
  //	double Tw;
  FieldRef<double> qwin  = InterfaceOUT->getField<double>("qwin");
  FieldRef<double> qwout = InterfaceIN->getField<double>("qwout");
  for (std::pair<simit::Set::ElementIterator, simit::Set::ElementIterator>
      bc(InterfaceOUT->begin(),InterfaceIN->begin());
      bc.first != InterfaceOUT->end(); ++bc.first,++bc.second) {
    qwin.set((ElementRef)*bc.first,-qwout.get((ElementRef)*bc.second));
    //	Tw=InterfaceIN->getField<double>("Twout").get((ElementRef)*bc.second);
  }
  //	std::cout << " TW " << Tw << std::endl;
}

void Thermal::setBC_Tw(Set *InterfaceIN, int i){
  Set *InterfaceOUT;
  switch (coupling_direction(0)) {
    case 0 : InterfaceOUT = bcleft_MG[i];  	break;
    case 1 : InterfaceOUT = bcright_MG[i]; 	break;
    case 2 : InterfaceOUT = bcbottom_MG[i];	break;
    case 3 : InterfaceOUT = bcup_MG[i]; 		break;
  }
  FieldRef<double> Twin  = InterfaceOUT->getField<double>("Twin");
  FieldRef<double> Twout = InterfaceIN->getField<double>("Twout");
  //	double Tw;
  for (std::pair<simit::Set::ElementIterator, simit::Set::ElementIterator>
      bc(InterfaceOUT->begin(),InterfaceIN->begin());
      bc.first != InterfaceOUT->end(); ++bc.first,++bc.second) {
    Twin.set((ElementRef)*bc.first,Twout.get((ElementRef)*bc.second));
    //		Tw=Twout.get((ElementRef)*bc.second);
  }
  //	std::cout << " TW " << Tw << std::endl;
}

bool Thermal::compareTw(Set *InterfaceIN, double tolerance, int i){
  Set *InterfaceOUT;
  switch (coupling_direction(0)) {
    case 0 : InterfaceOUT = bcleft_MG[i];  	break;
    case 1 : InterfaceOUT = bcright_MG[i]; 	break;
    case 2 : InterfaceOUT = bcbottom_MG[i];	break;
    case 3 : InterfaceOUT = bcup_MG[i]; 		break;
  }
  FieldRef<double> Twin  = InterfaceOUT->getField<double>("Twin");
  FieldRef<double> Twout = InterfaceIN->getField<double>("Twout");
  bool converged=true;
  for (std::pair<simit::Set::ElementIterator, simit::Set::ElementIterator>
      bc(InterfaceOUT->begin(),InterfaceIN->begin());
      bc.first != InterfaceOUT->end(); ++bc.first,++bc.second) {
    if (abs( Twin.get((ElementRef)*bc.first)-Twout.get((ElementRef)*bc.second) )
        /Twin.get((ElementRef)*bc.first) > tolerance) {
      converged=false;
      break;
    }
  }
  return converged;
}

