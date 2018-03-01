"""Unit tests for SubDomain"""

# Copyright (C) 2013 Johan Hake
#
# This file is part of DOLFIN.
#
# DOLFIN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# DOLFIN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.


import numpy as np
from dolfin import *
from dolfin_utils.test import skip_in_parallel
import pytest
from dolfin.jit.pybind11jit import compile_cpp_code


def xtest_compiled_subdomains():
    def noDefaultValues():
        CompiledSubDomain("a")

    def wrongDefaultType():
        CompiledSubDomain("a", a="1")

    def wrongParameterNames():
        CompiledSubDomain("foo", bar=1.0)

    with pytest.raises(RuntimeError):
        noDefaultValues()
    with pytest.raises(TypeError):
        wrongDefaultType()
    with pytest.raises(RuntimeError):
        wrongParameterNames()


@skip_in_parallel
def test_compiled_subdomains_compilation_failure():
    def invalidCppCode():
        CompiledSubDomain("/")
    with pytest.raises(RuntimeError):
        invalidCppCode()


def test_creation_and_marking():

    class Left(SubDomain):
        def inside(self, x, on_boundary):
            return x[:, 0] < DOLFIN_EPS

    class LeftOnBoundary(SubDomain):
        def inside(self, x, on_boundary):
            return np.logical_and(x[:, 0] < DOLFIN_EPS, on_boundary)

    class Right(SubDomain):
        def inside(self, x, on_boundary):
            return x[:, 0] > 1.0 - DOLFIN_EPS

    class RightOnBoundary(SubDomain):
        def inside(self, x, on_boundary):
            return np.logical_and(x[:, 0] > 1.0 - DOLFIN_EPS, on_boundary)

    cpp_code = """
        #include<pybind11/pybind11.h>
        #include<pybind11/eigen.h>
        namespace py = pybind11;

        #include<Eigen/Dense>
        #include<dolfin/mesh/SubDomain.h>

        class Left : public dolfin::mesh::SubDomain
        {
        public:

          virtual Eigen::Matrix<bool, Eigen::Dynamic, 1> inside(Eigen::Ref<const Eigen::Matrix<double, Eigen::Dynamic,
                                         Eigen::Dynamic, Eigen::RowMajor>> x, bool on_boundary) const
          {
            Eigen::Matrix<bool, Eigen::Dynamic, 1> result(x.rows());
            for (unsigned int i = 0; i < x.rows(); ++i)
                result[i] = (x(i, 0) < DOLFIN_EPS);
            return result;
          }
        };

        class LeftOnBoundary : public dolfin::mesh::SubDomain
        {
        public:

          virtual Eigen::Matrix<bool, Eigen::Dynamic, 1> inside(Eigen::Ref<const Eigen::Matrix<double, Eigen::Dynamic,
                                         Eigen::Dynamic, Eigen::RowMajor>> x, bool on_boundary) const
          {
            Eigen::Matrix<bool, Eigen::Dynamic, 1> result(x.rows());
            for (unsigned int i = 0; i < x.rows(); ++i)
                result[i] = (x(i, 0) < DOLFIN_EPS and on_boundary);
            return result;
          }
        };

        class Right : public dolfin::mesh::SubDomain
        {
        public:

          virtual Eigen::Matrix<bool, Eigen::Dynamic, 1> inside(Eigen::Ref<const Eigen::Matrix<double, Eigen::Dynamic,
                                         Eigen::Dynamic, Eigen::RowMajor>> x, bool on_boundary) const
          {
            Eigen::Matrix<bool, Eigen::Dynamic, 1> result(x.rows());
            for (unsigned int i = 0; i < x.rows(); ++i)
                result[i] = (x(i, 0) > 1.0 - DOLFIN_EPS);
            return result;
          }
        };

        class RightOnBoundary : public dolfin::mesh::SubDomain
        {
        public:

          virtual Eigen::Matrix<bool, Eigen::Dynamic, 1> inside(Eigen::Ref<const Eigen::Matrix<double, Eigen::Dynamic,
                                         Eigen::Dynamic, Eigen::RowMajor>> x, bool on_boundary) const
          {
            Eigen::Matrix<bool, Eigen::Dynamic, 1> result(x.rows());
            for (unsigned int i = 0; i < x.rows(); ++i)
                result[i] = (x(i, 0) > 1.0 - DOLFIN_EPS and on_boundary);
            return result;
          }
        };

    PYBIND11_MODULE(SIGNATURE, m) {
       py::class_<Left, std::shared_ptr<Left>, dolfin::mesh::SubDomain>(m, "Left").def(py::init<>());
       py::class_<Right, std::shared_ptr<Right>, dolfin::mesh::SubDomain>(m, "Right").def(py::init<>());
       py::class_<LeftOnBoundary, std::shared_ptr<LeftOnBoundary>, dolfin::mesh::SubDomain>(m, "LeftOnBoundary").def(py::init<>());
       py::class_<RightOnBoundary, std::shared_ptr<RightOnBoundary>, dolfin::mesh::SubDomain>(m, "RightOnBoundary").def(py::init<>());
    }
    """

    compiled_domain_module = compile_cpp_code(cpp_code)

    subdomain_pairs = [(Left(), Right()),
                       (LeftOnBoundary(), RightOnBoundary()),
#                       (AutoSubDomain(lambda x, on_boundary: x[0] < DOLFIN_EPS),
#                        AutoSubDomain(lambda x, on_boundary: x[0] > 1.0 - DOLFIN_EPS)),
#                       (AutoSubDomain(lambda x, on_boundary: x[0] < DOLFIN_EPS and on_boundary),
#                        AutoSubDomain(lambda x, on_boundary: x[0] > 1.0 - DOLFIN_EPS and on_boundary)),
#                       (CompiledSubDomain("std::abs(x[0]-a) < DOLFIN_EPS", a=0.0),
#                        CompiledSubDomain("std::abs(x[0]-a) < DOLFIN_EPS", a=1.0)),
#                       (CompiledSubDomain("std::abs(x[0]-a) < DOLFIN_EPS and on_boundary", a=0.0),
#                        CompiledSubDomain("std::abs(x[0]-a) < DOLFIN_EPS and on_boundary", a=1.0)),
#                       (CompiledSubDomain("std::abs(x[0]) < DOLFIN_EPS"),
#                        CompiledSubDomain("std::abs(x[0]-1.0) < DOLFIN_EPS")),
#                       (CompiledSubDomain("std::abs(x[0]) < DOLFIN_EPS and on_boundary"),
#                        CompiledSubDomain("std::abs(x[0]-1.0) < DOLFIN_EPS and on_boundary")),
                       #
                       (compiled_domain_module.Left(),
                        compiled_domain_module.Right()),
                       (compiled_domain_module.LeftOnBoundary(),
                        compiled_domain_module.RightOnBoundary())
                       ]

#    empty = CompiledSubDomain("false")
#    every = CompiledSubDomain("true")

    for ind, MeshClass in enumerate([UnitIntervalMesh, UnitSquareMesh,
                                     UnitCubeMesh]):
        dim = ind + 1
        args = [10]*dim
        mesh = MeshClass(MPI.comm_world, *args)

        mesh.init()

        for left, right in subdomain_pairs:
            for t_dim, f_dim in [(0, 0),
                                 (mesh.topology().dim()-1, dim - 1),
                                 (mesh.topology().dim(), dim)]:
                f = MeshFunction("size_t", mesh, t_dim, 0)

                left.mark(f, int(1))
                right.mark(f, 2)

                correct = {(1, 0): 1,
                           (1, 0): 1,
                           (1, 1): 0,
                           (2, 0): 11,
                           (2, 1): 10,
                           (2, 2): 0,
                           (3, 0): 121,
                           (3, 2): 200,
                           (3, 3): 0}

                # Check that the number of marked entities are at least the
                # correct number (it can be larger in parallel)
                assert all(value >= correct[dim, f_dim]
                           for value in [
                    MPI.sum(mesh.mpi_comm(), float((f.array() == 2).sum())),
                    MPI.sum(mesh.mpi_comm(), float((f.array() == 1).sum())),
                ])

        for t_dim, f_dim in [(0, 0),
                             (mesh.topology().dim()-1, dim-1),
                             (mesh.topology().dim(), dim)]:
            f = MeshFunction("size_t", mesh, t_dim, 0)

#            empty.mark(f, 1)
#            every.mark(f, 2)

            # Check that the number of marked entities is correct
#            assert sum(f.array() == 1) == 0
#            assert sum(f.array() == 2) == mesh.num_entities(f_dim)
