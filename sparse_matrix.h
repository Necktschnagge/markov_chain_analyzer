/**
 * @file sparse_matrix.h
 *
 * sparse matrix and adaptive stuff for AMGCL
 *
 */
#pragma once

#include <amgcl/backend/builtin.hpp>
#include <amgcl/make_solver.hpp>
#include <amgcl/amg.hpp>
#include <amgcl/coarsening/smoothed_aggregation.hpp>
#include <amgcl/coarsening/aggregation.hpp>
#include <amgcl/relaxation/spai0.hpp>
#include <amgcl/solver/cg.hpp>
#include <amgcl/profiler.hpp>

#include "Eigen/IterativeLinearSolvers"

#include <vector>
#include <unordered_map>


/**
	@brief Simple sparse matrix implementation to be used with AMGCL sparse linear system solver.
*/ //## make this class template, recheck all usages! -> rational type and int type
class sparse_matrix {
public:
	typedef std::unordered_map<int, double> sparse_row;

private:
	int m, n;
	std::vector<sparse_row> rows;

public:
	using size_t = decltype(m);

	sparse_matrix(int mm, int nn) : m(mm), n(nn), rows(mm) { }

	int size_m() const { return m; }
	int size_n() const { return n; }


	// Get a value at row i and column j
	double operator()(int i, int j) const {
		const sparse_row::const_iterator elem{ rows[i].find(j) };
		return elem == rows[i].end() ? 0.0 : elem->second;
	}

	// Get reference to a value at row i and column j
	double& operator()(int i, int j) { return rows[i][j]; }

	// Access the whole row
	const sparse_row& operator[](int i) const { return rows[i]; }

	/**
		@brief Subtracts the unity martix.
	*/
	inline void subtract_unity_matrix() {
		if (size_m() != size_n()) throw std::invalid_argument("Matrix is not quadratic.");
		for (sparse_matrix::size_t it{ 0 }; it != size_m(); ++it) rows[it][it] -= 1;
	}

	/**
		unsafe
	*/
	Eigen::SparseMatrix<double> copy_to_eigen() {
		auto matrix{ Eigen::SparseMatrix<double>(m, n) };
		for (decltype(rows)::size_type m{ 0 }; m < rows.size(); ++m) 
			for (const auto& pair : rows[m]) 
				matrix.insert(m, pair.first) = pair.second;
		return matrix;
	}
};



/* Define type traits required by amgcl for own class sparse_matrix */
namespace amgcl {
	namespace backend {

		// Let AMGCL know the value type of our matrix:
		template <> struct value_type<sparse_matrix> {
			typedef double type;
		};

		// Let AMGCL know the size of our matrix:
		template<> struct rows_impl<sparse_matrix> {
			static int get(const sparse_matrix& A) { return A.size_m(); }
		};

		template<> struct cols_impl<sparse_matrix> {
			static int get(const sparse_matrix& A) { return A.size_n(); }
		};

		template<> struct nonzeros_impl<sparse_matrix> {
			static int get(const sparse_matrix& A) {
				int m = A.size_m(), nnz = 0;
				for (int i = 0; i < m; ++i)
					nnz += A[i].size();
				return nnz;
			}
		};

		// Allow AMGCL to iterate over the rows of our matrix:
		template<> struct row_iterator<sparse_matrix> {
			struct iterator {
				sparse_matrix::sparse_row::const_iterator _it, _end;

				iterator(const sparse_matrix& A, int row)
					: _it(A[row].begin()), _end(A[row].end()) { }

				// Check if we are at the end of the row.
				operator bool() const {
					return _it != _end;
				}

				// Advance to the next nonzero element.
				iterator& operator++() {
					++_it;
					return *this;
				}

				// Column number of the current nonzero element.
				int col() const { return _it->first; }

				// Value of the current nonzero element.
				double value() const { return _it->second; }
			};

			typedef iterator type;
		};

		template<> struct row_begin_impl<sparse_matrix> {
			typedef row_iterator<sparse_matrix>::type iterator;
			static iterator get(const sparse_matrix& A, int row) {
				return iterator(A, row);
			}
		};

	} // namespace backend

} // namespace amgcl