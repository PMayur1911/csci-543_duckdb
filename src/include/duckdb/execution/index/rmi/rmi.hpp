//===----------------------------------------------------------------------===//
//                         DuckDB
//
// duckdb/execution/index/rmi/rmi.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/execution/index/bound_index.hpp"


namespace duckdb {

class RMI : public BoundIndex {

public:
    // Index type name for the RMI
    static constexpr const char *TYPE_NAME = "RMI";

    int rmi_value;

public:
    RMI(const string &name, const IndexConstraintType index_constraint_type, const vector<column_t> &column_ids,
        TableIOManager &table_io_manager, const vector<unique_ptr<Expression>> &unbound_expressions,
        AttachedDatabase &db, const case_insensitive_map_t<Value> &options,
        const IndexStorageInfo &info = IndexStorageInfo(), idx_t estimated_cardinality = 0);

    //! Create a index instance of this type
    static unique_ptr<BoundIndex> Create(CreateIndexInput &input) {
        auto rmi = make_uniq<RMI>(input.name, input.constraint_type, input.column_ids, input.table_io_manager,
		                          input.unbound_expressions, input.db, input.options, input.storage_info);
		return std::move(rmi);
    }
    
    //! Plan Index Construction
    static PhysicalOperator &CreatePlan(PlanIndexInput &input);


    // CSCI543 for later
    // unique_ptr<IndexScanState> InitializeScan() const;
    // idx_t Scan(IndexScanState &state, Vector &result) const;
        // Vector &result depends on how we want to scan to be

public:

    //! Called when data is appended to the index. The lock obtained from InitializeLock must be held
	ErrorData Append(IndexLock &lock, DataChunk &entries, Vector &row_identifiers) override;

    //! Insert a chunk of entries into the index
	ErrorData Insert(IndexLock &lock, DataChunk &data, Vector &row_ids) override;

    //! Delete a chunk of entries from the index. The lock obtained from InitializeLock must be held
	void Delete(IndexLock &lock, DataChunk &entries, Vector &row_identifiers) override;

    //! Deletes all data from the index. The lock obtained from InitializeLock must be held
	void CommitDrop(IndexLock &index_lock) override;


    // CSCI543 - Do we need to adapt this?
    //! Build an RMI Index from a vector of sorted keys and their row IDs.
	// ARTConflictType Build(unsafe_vector<ARTKey> &keys, unsafe_vector<ARTKey> &row_ids, const idx_t row_count);


    //! Serializes RMI memory to disk and returns the RMI storage information.
	IndexStorageInfo SerializeToDisk(QueryContext context, const case_insensitive_map_t<Value> &options) override;
	
    //! Serializes RMI memory to the WAL and returns the RMI storage information.
	IndexStorageInfo SerializeToWAL(const case_insensitive_map_t<Value> &options) override;

    //! Returns the in-memory usage of the ART.
	idx_t GetInMemorySize(IndexLock &index_lock) override;


    //! Merge another RMI index into this RMI index. The lock obtained from InitializeLock must be held, and the other
	//! index must also be locked during the merge
	bool MergeIndexes(IndexLock &state, BoundIndex &other_index) override;

    //! Traverses an RMI Index and vacuums the qualifying nodes. The lock obtained from InitializeLock must be held
	void Vacuum(IndexLock &state) override;


    //! Verifies the nodes and optionally returns a string of the RMI.
	string VerifyAndToString(IndexLock &l, const bool only_verify) override;

	//! Verifies that the node allocations match the node counts.
	void VerifyAllocations(IndexLock &l) override;
	
    //! Verifies the index buffers.
	void VerifyBuffers(IndexLock &l) override;


    string GetConstraintViolationMessage(VerifyExistenceType verify_type, idx_t failed_index,
	                                     DataChunk &input) override {
		return "Constraint violation in RTree index";
	}

    };
} // namespace duckdb