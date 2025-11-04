#include "duckdb/execution/index/rmi/rmi.hpp"

#include "duckdb/storage/table/scan_state.hpp"

namespace duckdb {

//------------------------------------------------------------------------------
// RMI Index Scan State
//------------------------------------------------------------------------------
class RMIIndexScanState final : public IndexScanState {
    //! The Predicates to scan.
    //! A single predicate for point lookups, and two predicates for range scans.
    Value values[2];

    //! THe expressions over the scan predicates.
    ExpressionType expressions[2];
    bool checked = false;

    //! All scanned row IDs.
    set<row_t> row_ids;
};


//------------------------------------------------------------------------------
// RMI Index Methods
//------------------------------------------------------------------------------
RMI::RMI(const string &name, const IndexConstraintType index_constraint_type, const vector<column_t> &column_ids,
        TableIOManager &table_io_manager, const vector<unique_ptr<Expression>> &unbound_expressions,
        AttachedDatabase &db, const case_insensitive_map_t<Value> &options,
        const IndexStorageInfo &info, idx_t estimated_cardinality) 
    : BoundIndex(name, RMI::TYPE_NAME, index_constraint_type, column_ids, table_io_manager, unbound_expressions, db) {
    
        for (idx_t i = 0; i < types.size(); i++) {
            switch(types[i]) {
                case PhysicalType::INT8:
                case PhysicalType::INT16:
                case PhysicalType::INT32:
                case PhysicalType::INT64:
                case PhysicalType::INT128:
                case PhysicalType::UINT8:
                case PhysicalType::UINT16:
                case PhysicalType::UINT32:
                case PhysicalType::UINT64:
                case PhysicalType::UINT128:
                case PhysicalType::FLOAT:
                case PhysicalType::DOUBLE:
                    break;
                default:
                    throw InvalidTypeException(logical_types[i], "Unsupported type for RMI index key.");
            }
        }

        if (index_constraint_type != IndexConstraintType::NONE) {
            throw NotImplementedException("RMI Indexes do not support UNIQUE or PRIMARY KEY constraints.");
        }

        
        //! Check for NULLs, NaNs, Inf etc maybe?
        
        //! Initialize the RMI model or something here?
        rmi_value = 0;
        
        
        // if (!info.IsValid()) {
        //     // We create a new RMI Index.
        //     return;
        // }
    }


//------------------------------------------------------------------------------
// RMI Index Override Methods
//------------------------------------------------------------------------------
ErrorData RMI::Insert(IndexLock &lock, DataChunk &data, Vector &row_ids) {
    //! Implement Later
    // return ErrorData();
    return ErrorData {};
}

ErrorData RMI::Append(IndexLock &lock, DataChunk &entries, Vector &row_identifiers) {
    //! Implement Later

    return Insert(lock, entries, row_identifiers);
}

void RMI::Delete(IndexLock &lock, DataChunk &entries, Vector &row_identifiers) {
    //! Do Nothing
    ;
}

void RMI::CommitDrop(IndexLock &index_lock) {
    //! Do Nothing
    ;
}

//------------------------------------------------------------------------------
// RMI Index Serialize Methods
//------------------------------------------------------------------------------

IndexStorageInfo RMI::SerializeToDisk(QueryContext context, const case_insensitive_map_t<Value> &options) {
    return IndexStorageInfo();
}

IndexStorageInfo RMI::SerializeToWAL(const case_insensitive_map_t<Value> &options) {
    return IndexStorageInfo();
}

idx_t RMI::GetInMemorySize(IndexLock &index_lock) {
    return 0;
}

//------------------------------------------------------------------------------
// RMI Index Misc and Verify Methods
//------------------------------------------------------------------------------

bool RMI::MergeIndexes(IndexLock &state, BoundIndex &other_index) {
    return false;
}

void RMI::Vacuum(IndexLock &state) {
    ;
}

string RMI::VerifyAndToString(IndexLock &l, const bool only_verify) {
    return "[RMI] : <returns>";
}

void RMI::VerifyAllocations(IndexLock &l) {
    ;
}

void RMI::VerifyBuffers(IndexLock &l) {
    ;
}

}   // namespace duckdb