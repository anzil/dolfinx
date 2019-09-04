// Copyright (C) 2007-2014 Anders Logg
//
// This file is part of DOLFIN (https://www.fenicsproject.org)
//
// SPDX-License-Identifier:    LGPL-3.0-or-later

#pragma once

#include "FormCoefficients.h"
#include "FormIntegrals.h"
#include <functional>
#include <map>
#include <memory>
#include <petscsys.h>
#include <string>
#include <vector>

// Forward declaration
struct ufc_form;

namespace dolfin
{

namespace fem
{
class CoordinateMapping;
}

namespace function
{
class Constant;
class FunctionSpace;
} // namespace function

namespace mesh
{
class Mesh;
template <typename T>
class MeshFunction;
} // namespace mesh

namespace fem
{

/// Base class for UFC code generated by FFC for DOLFIN with option -l.
///
/// A note on the order of trial and test spaces: FEniCS numbers
/// argument spaces starting with the leading dimension of the
/// corresponding tensor (matrix). In other words, the test space is
/// numbered 0 and the trial space is numbered 1. However, in order to
/// have a notation that agrees with most existing finite element
/// literature, in particular
///
///  \f[   a = a(u, v)        \f]
///
/// the spaces are numbered from right to left
///
///  \f[   a: V_1 \times V_0 \rightarrow \mathbb{R}  \f]
///
/// This is reflected in the ordering of the spaces that should be
/// supplied to generated subclasses. In particular, when a bilinear
/// form is initialized, it should be initialized as `a(V_1, V_0) =
/// ...`, where `V_1` is the trial space and `V_0` is the test space.
/// However, when a form is initialized by a list of argument spaces
/// (the variable `function_spaces` in the constructors below), the list
/// of spaces should start with space number 0 (the test space) and then
/// space number 1 (the trial space).

class Form
{
public:
  /// Create form
  ///
  /// @param[in] function_spaces
  /// @param[in] integrals
  /// @param[in] coefficients
  /// @param[in] constants
  ///            Vector of pairs (name, constant). The index in the vector
  ///            is the position of the constant in the original
  ///            (nonsimplified) form.
  Form(const std::vector<std::shared_ptr<const function::FunctionSpace>>&
           function_spaces,
       const FormIntegrals& integrals, const FormCoefficients& coefficients,
       const std::vector<
           std::pair<std::string, std::shared_ptr<const function::Constant>>>
           constants,
       std::shared_ptr<const CoordinateMapping> coord_mapping);

  /// Create form (no UFC integrals). Integrals can be attached later
  /// using FormIntegrals::set_cell_tabulate_tensor. Experimental.
  ///
  /// @param[in] function_spaces (std::vector<_function::FunctionSpace_>)
  ///         Vector of function spaces.
  Form(const std::vector<std::shared_ptr<const function::FunctionSpace>>&
           function_spaces);

  /// Move constructor
  Form(Form&& form) = default;

  /// Destructor
  virtual ~Form() = default;

  /// Return rank of form (bilinear form = 2, linear form = 1,
  /// functional = 0, etc)
  ///
  /// @return std::size_t
  ///         The rank of the form.
  int rank() const;

  /// Set coefficient with given number (shared pointer version)
  ///
  /// @param[in]  i (std::size_t)
  ///         The given number.
  /// @param[in]    coefficient (_Function_)
  ///         The coefficient.
  void set_coefficients(
      std::map<std::size_t, std::shared_ptr<const function::Function>>
          coefficients);

  /// Set coefficient with given name (shared pointer version)
  ///
  /// @param[in]    name (std::string)
  ///         The name.
  /// @param[in]    coefficient (_Function_)
  ///         The coefficient.
  void set_coefficients(
      std::map<std::string, std::shared_ptr<const function::Function>>
          coefficients);

  /// Return original coefficient position for each coefficient (0
  /// <= i < n)
  ///
  /// @return std::size_t
  ///         The position of coefficient i in original ufl form
  ///         coefficients.
  int original_coefficient_position(int i) const;

  /// Set constants based on their names
  ///
  /// This method is used in command-line workflow, when users set
  /// constants to the form in cpp file.
  ///
  /// Names of the constants must agree with their names in UFL file.
  void set_constants(
      std::map<std::string, std::shared_ptr<const function::Constant>> constants);

  /// Set constants based on their order (without names)
  ///
  /// This method is used in python workflow, when constants
  /// are automatically attached to the form based on their order
  /// in the original form.
  ///
  /// The order of constants must match their order in
  /// original ufl Form.
  void
  set_constants(std::vector<std::shared_ptr<const function::Constant>> constants);

  /// Set mesh, necessary for functionals when there are no function
  /// spaces
  ///
  /// @param[in] mesh (_mesh::Mesh_)
  ///         The mesh.
  void set_mesh(std::shared_ptr<const mesh::Mesh> mesh);

  /// Extract common mesh from form
  ///
  /// @return mesh::Mesh
  ///         Shared pointer to the mesh.
  std::shared_ptr<const mesh::Mesh> mesh() const;

  /// Return function space for given argument
  ///
  /// @param  i (std::size_t)
  ///         Index
  ///
  /// @return function::FunctionSpace
  ///         Function space shared pointer.
  std::shared_ptr<const function::FunctionSpace> function_space(int i) const;

  /// Register the function for 'tabulate_tensor' for cell integral i
  void register_tabulate_tensor_cell(
      int i, void (*fn)(PetscScalar*, const PetscScalar*, const PetscScalar*,
                        const double*, const int*, const int*));

  /// Set cell domains
  ///
  /// @param[in]    cell_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The cell domains.
  void set_cell_domains(const mesh::MeshFunction<std::size_t>& cell_domains);

  /// Set exterior facet domains
  ///
  ///  @param[in]   exterior_facet_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The exterior facet domains.
  void set_exterior_facet_domains(
      const mesh::MeshFunction<std::size_t>& exterior_facet_domains);

  /// Set interior facet domains
  ///
  ///  @param[in]   interior_facet_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The interior facet domains.
  void set_interior_facet_domains(
      const mesh::MeshFunction<std::size_t>& interior_facet_domains);

  /// Set vertex domains
  ///
  ///  @param[in]   vertex_domains (_mesh::MeshFunction_ <std::size_t>)
  ///         The vertex domains.
  void
  set_vertex_domains(const mesh::MeshFunction<std::size_t>& vertex_domains);

  /// Access coefficients (non-const)
  FormCoefficients& coefficients();

  /// Access coefficients (const)
  const FormCoefficients& coefficients() const;

  /// Access form integrals (const)
  const FormIntegrals& integrals() const;

  /// Access constants (const)
  ///
  /// @return Vector of attached constants with their names.
  ///         Names are used to set constants in user's c++ code.
  ///         Index in the vector is the position of the constant in the
  ///         original (nonsimplified) form.
  const std::vector<
      std::pair<std::string, std::shared_ptr<const function::Constant>>>&
  constants() const;

  /// Get coordinate_mapping (experimental)
  std::shared_ptr<const fem::CoordinateMapping> coordinate_mapping() const;

private:
  // Integrals associated with the Form
  FormIntegrals _integrals;

  // Coefficients associated with the Form
  FormCoefficients _coefficients;

  // Constants associated with the Form
  std::vector<std::pair<std::string, std::shared_ptr<const function::Constant>>>
      _constants;

  // Function spaces (one for each argument)
  std::vector<std::shared_ptr<const function::FunctionSpace>> _function_spaces;

  // The mesh (needed for functionals when we don't have any spaces)
  std::shared_ptr<const mesh::Mesh> _mesh;

  // Coordinate_mapping
  std::shared_ptr<const fem::CoordinateMapping> _coord_mapping;
};
} // namespace fem
} // namespace dolfin
