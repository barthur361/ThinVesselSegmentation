#pragma once
#include "SparseMatrixDataCol.h"
#include "SparseMatrixDataRow.h"

class SparseMatrixData
{
	// data
	SparseMatrixDataCol* datacol; // matrix data stored in collumn order
	SparseMatrixDataRow* datarow; // matrix data stored in row order

public:
	// constructor & destructor 
	SparseMatrixData( int num_rows, int num_cols, 
		const double non_zero_value[], 
		const int col_index[], 
		const int row_pointer[], 
		int N )
	{
		datarow = new SparseMatrixDataRow( num_rows, num_cols, non_zero_value, col_index, row_pointer, N ); 
	}
	~SparseMatrixData(){
		if( datacol ) delete datacol;
		if( datarow ) delete datarow; 
	}

	const SparseMatrixDataCol* const getCol(){ 
		if( datarow==NULL || datacol ) return datacol; 

		/* Convert the compressed row fromat to the compressed column format. */
		double* nzval = NULL; 
		int *rowidx = NULL;
		int *colptr = NULL; 
		dCompRow_to_CompCol( 
			datarow->supermatrix->nrow, // number of rows
			datarow->supermatrix->ncol, // number of cols
			datarow->getData()->nnz,    // number of non-zero entries
			datarow->nzvel(),           // non-zero entries
			datarow->colinx(),          // column index
			datarow->rowptr(),          // row pointers
			&nzval,                     // non-zero entries (for column-order matrix)
			&rowidx,                    // row indeces
			&colptr );                  // column pointers

		datacol = new SparseMatrixDataCol( 
			datarow->supermatrix->nrow,
			datarow->supermatrix->ncol,
			nzval,
			rowidx,
			colptr,
			datarow->getData()->nnz );
		return datacol;
	}

	const SparseMatrixDataRow* const getRow(){
		if( datacol==NULL || datarow ) return datarow;

		/* Convert the compressed row fromat to the compressed column format. */
		double* nzval = NULL; 
		int *colidx = NULL;
		int *rowptr = NULL; 
		dCompRow_to_CompCol( 
			datacol->supermatrix->nrow, // number of rows
			datacol->supermatrix->ncol, // number of cols
			datacol->getData()->nnz,    // number of non-zero entries
			datacol->nzvel(),           // non-zero entries
			datacol->rowinx(),          // column index
			datacol->colptr(),          // row pointers
			&nzval,                     // non-zero entries (for row-order matrix)
			&colidx,                    // column indeces
			&rowptr );                  // column pointers

		datarow = new SparseMatrixDataRow( 
			datarow->supermatrix->nrow,
			datarow->supermatrix->ncol,
			nzval,
			colidx,
			rowptr,
			datacol->getData()->nnz );
		return datarow;
	}
};

