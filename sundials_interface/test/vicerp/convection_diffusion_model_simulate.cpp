/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A minimalistic computer algebra system with automatic differentiation 
 *    and framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl et al., K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include <fstream>
#include <ctime>
#include "convection_diffusion_model.hpp"


// Just simulate a scenario
void simulate(ostream& resfile, vector<double>& t_prof){

  // Create a single-stage optimal control problem
  OCP ocp;

  // Time variable
  Matrix t = ocp.getTime();

  // Specify the time horizon
  ocp.setInitialTime(t_sunrise);
  ocp.setFinalTime(t_next_sunrise);

  // differential states
  ocp.addState(T,Tdot);

  // Solar irradiation
  Matrix phiR = (t<t_sunset) * t*(12-t)/36; // quadratic function

  // Mass flow to boiler
  Matrix phiB = 0.8 * (( t >= t_startup) && ( t < t_shutdown));

  // Differential states
  Matrix eta = 1 - ((0.80 - phiB)*(0.80 - phiB))/((0.80 - 0.25)*(0.80 - 0.25)); // quadratic function with max at 80 %
  
  // Mass flow to storage
  Matrix u = phiR - phiB;

  // Differential equation
  ocp.addEquation(Tdot, - u * if_else(u>=0, dT_dz_pos, dT_dz_neg) + D*d2T_dz2);

  // Initial temperature in the storage
  Matrix T0 = T_u * ones(NZ,1);
  ocp.addInitialCondition(T, T0);

  // The desired output times
  Matrix t_out = linspace(0,10,nt_out);

  // numerically
  vector<double> t_out_num(nt_out);
  for(int i=0; i<nt_out; ++i)
    t_out_num[i] = 10*i/double(nt_out-1);

  // Project to these points
  Matrix T_proj(nz_out,NZ);
  for(int i=0; i<nz_out; ++i)
    T_proj(i,(NZ*i)/nz_out) = 1; // integer division!

  // Set output function
  int oind_T = ocp.addOutput(t_out, T_proj*T);
  int oind_u = ocp.addOutput(t_out, u);

  // Eliminate dependent variables from the functions
  eliminateDependent(ocp);

  // Allocate an integrator
  CvodesIntegrator integrator(ocp);

  // Set the linear solver
  //   integrator.setOption("linear_solver","dense");
  integrator->setOption("linear_solver","band");
  //  integrator->setOption("linear_solver","sparse");

  // Use exact jacobian
  integrator->setOption("exact_jacobian",false);

  // Upper and lower band-widths (only relevant when using a band linear solver)
   integrator->setOption("mupper",1);
   integrator->setOption("mlower",1);

  // set tolerances 
  integrator->setOption("reltol",1e-6);
  integrator->setOption("abstol",1e-8);

  // Initialize the integrator
  integrator->init();
  cout << "initialized" << endl;

  // Integrate once for visualization
  integrator->evaluate();
  cout << "solved" << endl;

  // Save results to file
  resfile << "t_out " << t_out_num << endl;
  resfile << "T " << integrator->getOutput(oind_T) << endl;
  resfile << "u " << integrator->getOutput(oind_u) << endl;
  resfile << "nz_out " << nz_out << endl;
  resfile << "nt_out " << nt_out << endl;

  vector<double> z_out(nz_out);
  for(int i=0; i<nz_out; ++i)
    z_out[i] = i/double(nz_out-1);

  resfile << "z_out " << z_out << endl;

  // Save to output
  t_prof = integrator->getOutput(oind_T);
}
