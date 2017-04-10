/*
 * constraint_container.cc
 *
 *  Created on: May 24, 2016
 *      Author: winklera
 */

#include <xpp/constraint_container.h>
#include <iostream>

namespace xpp {
namespace opt {

ConstraintContainer::ConstraintContainer (OptimizationVariablesContainer& subject)
{
  opt_variables_ = &subject;
}

ConstraintContainer::~ConstraintContainer ()
{
  // TODO Auto-generated destructor stub
}

void
ConstraintContainer::ClearConstraints ()
{
  constraints_.clear();
}

void
ConstraintContainer::AddConstraint (ConstraitPtrVec constraints)
{
  for (auto& c : constraints)
    constraints_.push_back(c);

  UpdateConstraints();
  RefreshBounds ();
}

ConstraintContainer::VectorXd
ConstraintContainer::EvaluateConstraints () const
{
  VectorXd g_all(bounds_.size());

  int c = 0;
  for (const auto& constraint : constraints_) {

    VectorXd g = constraint->GetConstraintValues();
    int c_new = g.rows();
    g_all.middleRows(c, c_new) = g;
    c += c_new;
  }
  return g_all;
}

ConstraintContainer::JacobianPtr
ConstraintContainer::GetJacobian () const
{
  int row = 0;
  for (const auto& constraint : constraints_) {

    int col = 0;
    for (const auto& var : opt_variables_->GetOptVarsVec()) {

      Jacobian jac = constraint->GetJacobianWithRespectTo(var->GetId());

      // insert the derivative in the correct position in the overall Jacobian
      for (int k=0; k<jac.outerSize(); ++k)
        for (Jacobian::InnerIterator it(jac,k); it; ++it)
          jacobian_->coeffRef(row+it.row(), col+it.col()) = it.value();


      col += var->GetVariables().rows();
    }

    row += constraint->GetNumberOfConstraints();
  }
  return jacobian_;
}

void
xpp::opt::ConstraintContainer::PrintStatus (double tol) const
{
  std::cout << "Constraint violation indices for tol=" << tol << ":\n";
  for (const auto& constraint : constraints_) {
    constraint->PrintStatus(tol);
  }
}

void
ConstraintContainer::UpdateConstraints ()
{
  for (auto& constraint : constraints_) {
    constraint->UpdateConstraintValues();
    constraint->UpdateJacobians();
  }
}

void
ConstraintContainer::RefreshBounds ()
{
  bounds_.clear();
  for (const auto& constraint : constraints_) {
    VecBound b = constraint->GetBounds();
    bounds_.insert(bounds_.end(), b.begin(), b.end());
  }

  int n_constraints = bounds_.size();
  // zmp_ this is the only time the subject is neccessary
  int n_variables   = opt_variables_->GetOptimizationVariableCount();
  jacobian_ = std::make_shared<Jacobian>(n_constraints, n_variables);
}

VecBound
ConstraintContainer::GetBounds () const
{
  return bounds_;
}

} /* namespace opt */
} /* namespace xpp */
