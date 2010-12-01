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

#include "evaluation.hpp"
#include "../stl_vector_tools.hpp"
#include <cassert>

using namespace std;

namespace CasADi{

// Constructor
Evaluation::Evaluation(const FX& fcn, const vector<MX>& dep, int oind_) : MXNode(dep), fcn_(fcn), oind(oind_) {
  sz.nrow = fcn_->output_[oind].size1();
  sz.ncol = fcn_->output_[oind].size2();
}

void Evaluation::print(ostream &stream) const{
  stream << fcn_ << "[" << dep() << "]";
}

void Evaluation::evaluate(int fsens_order, int asens_order){
  assert(fsens_order==0 || asens_order==0);
  
  // Pass the input to the function
  for(int i=0; i<ndep(); ++i)
    fcn_.input(i).set(dep(i)->val(0));

  // Give the forward seed to the function
  if(fsens_order>0) 
    for(int i=0; i<ndep(); ++i)
      fcn_.input(i).setF(dep(i)->val(1));
  
  // Evaluate
  fcn_.evaluate(fsens_order, asens_order);
  
  // Get the results
  fcn_.output(oind).get(&val(0)[0]);
  if(fsens_order>0)
    fcn_.output(oind).getF(val(1));

  // Adjoints
  if(asens_order>0){
    // Pass the adjoint seed to the function
    fcn_.output(oind).setA(val(1));
    
    // Evaluate
    fcn_->evaluate(0,1);

    // Get the results
    for(int i=0; i<ndep(); ++i)
      fcn_.input(i).getA(dep(i)->val(1));
  }
}

} // namespace CasADi
