// https://theorangeduck.com/page/animation-blend-spaces-without-triangulation
// https://github.com/orangeduck/Animation-Blendspace/

/*
MIT License

Copyright (c) 2021 Daniel Holden

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <cassert>
#include <glm/glm.hpp>
#include <unordered_map>
#include <stdio.h>

namespace BlendMatrixInterpolation
{
	template<typename T> struct array1d;
	template<typename T> struct array2d;

	// Basic type representing a pointer to some
	// data and the size of the data. `__restrict__`
	// here is used to indicate the data should not
	// alias against any other input parameters and 
	// can sometimes produce important performance
	// gains.
	template<typename T>
	struct slice1d
	{
		int size;
		T* __restrict  data;

		slice1d(int _size, T* _data) : size(_size), data(_data) {}

		slice1d& operator=(const slice1d<T>& rhs) { assert(rhs.size == size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };
		slice1d& operator=(const array1d<T>& rhs) { assert(rhs.size == size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };

		void zero() { memset((char*)data, 0, sizeof(T) * size); }
		void set(const T& x) { for (int i = 0; i < size; i++) { data[i] = x; } }

		inline T& operator()(int i) const { assert(i >= 0 && i < size); return data[i]; }
	};

	// Same but for a 2d array of data.
	template<typename T>
	struct slice2d
	{
		int rows, cols;
		T* __restrict  data;

		slice2d(int _rows, int _cols, T* _data) : rows(_rows), cols(_cols), data(_data) {}

		slice2d& operator=(const array2d<T>& rhs) { assert(rhs.rows == rows); assert(rhs.cols == cols); memcpy(data, rhs.data, rhs.rows * rhs.cols * sizeof(T)); return *this; };
		slice2d& operator=(const slice2d<T>& rhs) { assert(rhs.rows == rows); assert(rhs.cols == cols); memcpy(data, rhs.data, rhs.rows * rhs.cols * sizeof(T)); return *this; };

		void zero() { memset((char*)data, 0, sizeof(T) * rows * cols); }
		void set(const T& x) { for (int i = 0; i < rows * cols; i++) { data[i] = x; } }

		inline slice1d<T> operator()(int i) const { assert(i >= 0 && i < rows); return slice1d<T>(cols, &data[i * cols]); }
		inline T& operator()(int i, int j) const { assert(i >= 0 && i < rows && j >= 0 && j < cols); return data[i * cols + j]; }
	};
	template<typename T>
	struct array1d
	{
		int size;
		T* data;

		array1d() : size(0), data(NULL) {}
		array1d(int _size) : array1d() { resize(_size); }
		array1d(const slice1d<T>& rhs) : array1d() { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); }
		array1d(const array1d<T>& rhs) : array1d() { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); }
		~array1d() { resize(0); }

		array1d& operator=(const slice1d<T>& rhs) { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };
		array1d& operator=(const array1d<T>& rhs) { resize(rhs.size); memcpy(data, rhs.data, rhs.size * sizeof(T)); return *this; };

		inline T& operator()(int i) const { assert(i >= 0 && i < size); return data[i]; }
		operator slice1d<T>() const { return slice1d<T>(size, data); }

		slice1d<T> slice(int start, int stop) const { return slice1d<T>(stop - start, data + start); }

		void zero() { memset(data, 0, sizeof(T) * size); }
		void set(const T& x) { for (int i = 0; i < size; i++) { data[i] = x; } }

		void resize(int _size)
		{
			if (_size == 0 && size != 0)
			{
				free(data);
				data = NULL;
				size = 0;
			}
			else if (_size > 0 && size == 0)
			{
				data = (T*)malloc(_size * sizeof(T));
				size = _size;
				assert(data != NULL);
			}
			else if (_size > 0 && size > 0 && _size != size)
			{
				data = (T*)realloc(data, _size * sizeof(T));
				size = _size;
				assert(data != NULL);
			}
		}
	};

	template<typename T>
	void array1d_write(const array1d<T>& arr, FILE* f)
	{
		fwrite(&arr.size, sizeof(int), 1, f);
		size_t num = fwrite(arr.data, sizeof(T), arr.size, f);
		assert((int)num == arr.size);
	}

	template<typename T>
	void array1d_read(array1d<T>& arr, FILE* f)
	{
		int size;
		fread(&size, sizeof(int), 1, f);
		arr.resize(size);
		size_t num = fread(arr.data, sizeof(T), size, f);
		assert((int)num == size);
	}

	// Similar type but for 2d data
	template<typename T>
	struct array2d
	{
		int rows, cols;
		T* data;

		array2d() : rows(0), cols(0), data(NULL) {}
		array2d(int _rows, int _cols) : array2d() { resize(_rows, _cols); }
		~array2d() { resize(0, 0); }

		array2d& operator=(const array2d<T>& rhs) { resize(rhs.rows, rhs.cols); memcpy(data, rhs.data, rhs.rows * rhs.cols * sizeof(T)); return *this; };
		array2d& operator=(const slice2d<T>& rhs) { resize(rhs.rows, rhs.cols); memcpy(data, rhs.data, rhs.rows * rhs.cols * sizeof(T)); return *this; };

		inline slice1d<T> operator()(int i) const { assert(i >= 0 && i < rows); return slice1d<T>(cols, &data[i * cols]); }
		inline T& operator()(int i, int j) const { assert(i >= 0 && i < rows && j >= 0 && j < cols); return data[i * cols + j]; }
		operator slice2d<T>() const { return slice2d<T>(rows, cols, data); }

		slice2d<T> slice(int start, int stop) const { return slice2d<T>(stop - start, cols, data + start * cols); }

		void zero() { memset(data, 0, sizeof(T) * rows * cols); }
		void set(const T& x) { for (int i = 0; i < rows * cols; i++) { data[i] = x; } }

		void resize(int _rows, int _cols)
		{
			int _size = _rows * _cols;
			int size = rows * cols;

			if (_size == 0 && size != 0)
			{
				free(data);
				data = NULL;
				rows = 0;
				cols = 0;
			}
			else if (_size > 0 && size == 0)
			{
				data = (T*)malloc(_size * sizeof(T));
				rows = _rows;
				cols = _cols;
				assert(data != NULL);
			}
			else if (_size > 0 && size > 0 && _size != size)
			{
				data = (T*)realloc(data, _size * sizeof(T));
				rows = _rows;
				cols = _cols;
				assert(data != NULL);
			}
		}
	};

	template<typename T>
	void array2d_write(const array2d<T>& arr, FILE* f)
	{
		fwrite(&arr.rows, sizeof(int), 1, f);
		fwrite(&arr.cols, sizeof(int), 1, f);
		size_t num = fwrite(arr.data, sizeof(T), arr.rows * arr.cols, f);
		assert((int)num == arr.rows * arr.cols);
	}

	template<typename T>
	void array2d_read(array2d<T>& arr, FILE* f)
	{
		int rows, cols;
		fread(&rows, sizeof(int), 1, f);
		fread(&cols, sizeof(int), 1, f);
		arr.resize(rows, cols);
		size_t num = fread(arr.data, sizeof(T), rows * cols, f);
		assert((int)num == rows * cols);
	}

	template<typename T>
	void array2d_transpose(array2d<T>& a0, const array2d<T>& a1)
	{
		assert(a0.rows == a1.cols);
		assert(a0.cols == a1.rows);
		for (int i = 0; i < a1.rows; i++)
		{
			for (int j = 0; j < a1.cols; j++)
				a0(j, i) = a1(i, j);
		}
	}

	void mat_lu_solve_inplace(
		slice1d<float> vector,
		const slice2d<float> decomp,
		const slice1d<int> row_order)
	{
		assert(decomp.rows == decomp.cols);
		assert(decomp.rows == row_order.size);
		assert(decomp.rows == vector.size);

		int n = decomp.rows;
		int ii = -1;

		// Forward Substitution

		for (int i = 0; i < n; i++)
		{
			float sum = vector(row_order(i));
			vector(row_order(i)) = vector(i);

			if (ii != -1)
			{
				for (int j = ii; j <= i - 1; j++)
					sum -= decomp(i, j) * vector(j);
			}
			else if (sum)
				ii = i;

			vector(i) = sum;
		}

		// Backward Substitution

		for (int i = n - 1; i >= 0; i--)
		{
			float sum = vector(i);
			for (int j = i + 1; j < n; j++)
				sum -= decomp(i, j) * vector(j);
			vector(i) = sum / decomp(i, i);
		}
	}


	bool mat_lu_decompose_inplace(
		slice2d<float> matrix,     // Matrix to decompose in-place
		slice1d<int> row_order,    // Output row order
		slice1d<float> row_scale)  // Temp row scale
	{
		assert(matrix.rows == matrix.cols);
		assert(matrix.rows == row_order.size);
		assert(matrix.rows == row_scale.size);

		int n = matrix.rows;

		// Compute scaling for each row

		for (int i = 0; i < n; i++)
		{
			float vmax = 0.0f;
			for (int j = 0; j < n; j++)
				vmax = glm::max(vmax, (float) fabs(matrix(i, j)));

			if (vmax == 0.0)
				return false;

			row_scale(i) = 1.0 / vmax;
		}

		// Loop over columns using Crout's method

		for (int j = 0; j < n; j++)
		{
			for (int i = 0; i < j; i++)
			{
				float sum = matrix(i, j);
				for (int k = 0; k < i; k++)
					sum -= matrix(i, k) * matrix(k, j);
				matrix(i, j) = sum;
			}

			// Search Largest Pivot

			float vmax = 0.0f;
			int imax = -1;
			for (int i = j; i < n; i++)
			{
				float sum = matrix(i, j);
				for (int k = 0; k < j; k++)
					sum -= matrix(i, k) * matrix(k, j);
				matrix(i, j) = sum;
				float val = row_scale(i) * fabs(sum);
				if (val >= vmax)
				{
					vmax = val;
					imax = i;
				}
			}

			if (vmax == 0.0) { return false; }

			// Interchange Rows

			if (j != imax)
			{
				for (int k = 0; k < n; k++)
				{
					float val = matrix(imax, k);
					matrix(imax, k) = matrix(j, k);
					matrix(j, k) = val;
				}
				row_scale(imax) = row_scale(j);
			}

			// Divide by Pivot

			row_order(j) = imax;

			if (matrix(j, j) == 0.0) { return false; }

			if (j != n - 1)
			{
				float val = 1.0 / matrix(j, j);
				for (int i = j + 1; i < n; i++)
					matrix(i, j) *= val;
			}
		}

		return true;
	}

	void mat_transpose_inplace(slice2d<float> mat)
	{
		for (int i = 0; i < mat.rows; i++)
		{
			for (int j = i + 1; j < mat.cols; j++)
			{
				float val = mat(i, j);
				mat(i, j) = mat(j, i);
				mat(j, i) = val;
			}
		}
	}

	bool mat_inv(
		slice2d<float> output,
		const slice2d<float> input,
		const float lambda = 0.0f)
	{
		assert(input.rows == input.cols);

		array2d<float> decomp(input.rows, input.cols);
		decomp = input;

		// Add lambda to diagonal to improve inversion stability

		for (int i = 0; i < input.rows; i++)
			decomp(i, i) += lambda;

		array1d<int> row_order(input.rows);
		array1d<float> row_scale(input.rows);
		if (!mat_lu_decompose_inplace(decomp, row_order, row_scale))
		{
			output.zero();
			return false;
		}

		output.zero();
		for (int i = 0; i < output.rows; i++)
		{
			output(i, i) = 1.0f;
			mat_lu_solve_inplace(output(i), decomp, row_order);
		}

		mat_transpose_inplace(output);

		return true;
	}

	void mat_mul_vec(
		slice1d<float> output,
		const slice2d<float> lhs,
		const slice1d<float> rhs)
	{
		assert(output.size == lhs.rows);
		assert(rhs.size == lhs.cols);

		output.zero();
		for (int i = 0; i < output.size; i++)
		{
			for (int j = 0; j < rhs.size; j++)
				output(i) += rhs(j) * lhs(i, j);
		}
	}

	// Clamp and normalize blend weights so that they are greater than
	// zero and sum to one
	void clamp_normalize_blend_weights(slice1d<float> blend_weights)
	{
		float total_blend_weight = 0.0f;
		for (int i = 0; i < blend_weights.size; i++)
		{
			blend_weights(i) = glm::max(blend_weights(i), 0.0f);
			total_blend_weight += blend_weights(i);
		}

		for (int i = 0; i < blend_weights.size; i++)
			blend_weights(i) /= total_blend_weight;
	}

	// Fit the blend matrix to the given animation parameters
	void fit_blend_matrix(
		slice2d<float> blend_matrix,
		const glm::vec2* animation_points,
		int animation_count)
	{
		// Compute Pairwise Distances
		array2d<float> distances(animation_count, animation_count);
		for (int i = 0; i < animation_count; i++)
		{
			for (int j = 0; j < animation_count; j++)
			{
				distances(i, j) = 0.0f;
				distances(i, j) += glm::pow(
					animation_points[i].x -
					animation_points[j].x, 2.0f);
				distances(i, j) += glm::pow(
					animation_points[i].y -
					animation_points[j].y, 2.0f);
				distances(i, j) = glm::sqrt(distances(i, j));
			}
		}

		// Subtract epsilon from diagonal this helps the stability
		// of the decomposition and solve
		for (int i = 0; i < animation_count; i++)
			distances(i, i) -= 1e-4f;

		// Decompose in place
		array1d<int> row_order(animation_count);
		array1d<float> row_scale(animation_count);

		bool success = mat_lu_decompose_inplace(distances, row_order, row_scale);
		assert(success);

		// Write associated blend weights into blend matrix
		for (int i = 0; i < animation_count; i++)
		{
			for (int j = 0; j < animation_count; j++)
				blend_matrix(i, j) = i == j ? 1.0f : 0.0f;
		}

		// Solve for blend matrix in-place
		for (int i = 0; i < animation_count; i++)
			mat_lu_solve_inplace(blend_matrix(i), distances, row_order);
	}

	void compute_blend_weights(
		slice1d<float> blend_weights,
		const slice1d<float> query_distances,
		const slice2d<float> blend_matrix)
	{
		mat_mul_vec(blend_weights, blend_matrix, query_distances);
		clamp_normalize_blend_weights(blend_weights);
	}
}