// Copyright (C) 2009 Anders Logg.
// Licensed under the GNU LGPL Version 2.1.
//
// First added:  2009-09-28
// Last changed: 2010-01-26
//
// Modified by Johan Hake, 2009.

#include <boost/scoped_array.hpp>
#include <dolfin/log/log.h>
#include <dolfin/mesh/Mesh.h>
#include <dolfin/mesh/Cell.h>
#include <dolfin/mesh/Vertex.h>
#include <dolfin/fem/UFCCell.h>
#include "Data.h"
#include "Expression.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
Expression::Expression()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
Expression::Expression(uint dim)
{
  value_shape.resize(1);
  value_shape[0] = dim;
}
//-----------------------------------------------------------------------------
Expression::Expression(uint dim0, uint dim1)
{
  value_shape.resize(2);
  value_shape[0] = dim0;
  value_shape[1] = dim1;
}
//-----------------------------------------------------------------------------
Expression::Expression(std::vector<uint> value_shape)
  : value_shape(value_shape)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
Expression::Expression(const Expression& expression)
  : value_shape(expression.value_shape)
{
  // Do nothing
}
//-----------------------------------------------------------------------------
Expression::~Expression()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
dolfin::uint Expression::value_rank() const
{
  return value_shape.size();
}
//-----------------------------------------------------------------------------
dolfin::uint Expression::value_dimension(uint i) const
{
  if (i >= value_shape.size())
    error("Illegal axis %d for value dimension for value of rank %d.",
          i, value_shape.size());
  return value_shape[i];
}
//-----------------------------------------------------------------------------
void Expression::eval(Array<double>& values, const Data& data) const
{
  // Redirect to simple eval
  eval(values, data.x);
}
//-----------------------------------------------------------------------------
void Expression::restrict(double* w,
                          const FiniteElement& element,
                          const Cell& dolfin_cell,
                          const ufc::cell& ufc_cell,
                          int local_facet) const
{
  // Restrict as UFC function (by calling eval)
  restrict_as_ufc_function(w, element, dolfin_cell, ufc_cell, local_facet);
}
//-----------------------------------------------------------------------------
void Expression::compute_vertex_values(double* vertex_values,
                                       const Mesh& mesh) const
{
  assert(vertex_values);

  // Local data for vertex values
  const uint size = value_size();
  Array<double> local_vertex_values(size);
  Data data;

  // Iterate over cells, overwriting values when repeatedly visiting vertices
  UFCCell ufc_cell(mesh);
  for (CellIterator cell(mesh); !cell.end(); ++cell)
  {
    // Update cell data
    ufc_cell.update(*cell);
    data.set(*cell, ufc_cell, -1);

    // Iterate over cell vertices
    for (VertexIterator vertex(*cell); !vertex.end(); ++vertex)
    {
      // Update coordinate data
      data.set(mesh.geometry().dim(), vertex->x());

      // Evaluate at vertex
      eval(local_vertex_values, data);

      // Copy to array
      for (uint i = 0; i < size; i++)
      {
        const uint global_index = i*mesh.num_vertices() + vertex->index();
        vertex_values[global_index] = local_vertex_values[i];
      }
    }
  }
}
//-----------------------------------------------------------------------------
void Expression::eval(Array<double>& values, const Array<double>& x) const
{
  error("Missing eval() for Expression (must be overloaded).");
}
//-----------------------------------------------------------------------------

