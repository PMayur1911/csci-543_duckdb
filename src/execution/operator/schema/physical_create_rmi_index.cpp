#include "duckdb/execution/operator/schema/physical_create_rmi_index.hpp"

#include "duckdb/catalog/catalog_entry/duck_index_entry.hpp"
#include "duckdb/catalog/catalog_entry/duck_table_entry.hpp"
#include "duckdb/catalog/catalog_entry/table_catalog_entry.hpp"
#include "duckdb/common/exception/catalog_exception.hpp"
#include "duckdb/common/exception/transaction_exception.hpp"
#include "duckdb/main/client_context.hpp"
#include "duckdb/storage/table_io_manager.hpp"

namespace duckdb {

//-------------------------------------------------------------------------//
// Physical Create RMI Index
//-------------------------------------------------------------------------//

PhysicalCreateRMIIndex::PhysicalCreateRMIIndex(PhysicalPlan &physical_plan, LogicalOperator &op,
                                               TableCatalogEntry &table_p, const vector<column_t> &column_ids,
                                               unique_ptr<CreateIndexInfo> info,
                                               vector<unique_ptr<Expression>> unbound_expressions,
                                               idx_t estimated_cardinality, const bool sorted, 
                                               unique_ptr<AlterTableInfo> alter_table_info)
    : PhysicalOperator(physical_plan, PhysicalOperatorType::CREATE_INDEX, op.types, estimated_cardinality),
      table(table_p.Cast<DuckTableEntry>()), info(std::move(info)), unbound_expressions(std::move(unbound_expressions)), 
      sorted(sorted), alter_table_info(std::move(alter_table_info)) {
	// Convert the logical column ids to physical column ids.
	for (auto &column_id : column_ids) {
		storage_ids.push_back(table.GetColumns().LogicalToPhysical(LogicalIndex(column_id)).index);
	}
}

//===--------------------------------------------------------------------===//
// Sink
//===--------------------------------------------------------------------===//

class CreateRMIIndexGlobalSinkState : public GlobalSinkState {
public:
	unique_ptr<BoundIndex> global_index;
};

unique_ptr<GlobalSinkState> PhysicalCreateRMIIndex::GetGlobalSinkState(ClientContext &context) const {
	auto state = make_uniq<CreateRMIIndexGlobalSinkState>();
	auto &storage = table.GetStorage();
	state->global_index = make_uniq<RMI>(info->index_name, info->constraint_type, storage_ids,
	                                     TableIOManager::Get(storage), unbound_expressions, storage.db, info->options);
	return std::move(state);
}

SinkResultType PhysicalCreateRMIIndex::Sink(ExecutionContext &context, DataChunk &chunk,
                                            OperatorSinkInput &input) const {
	// For the checkpoint milestone we do not build the physical index yet.
	return SinkResultType::NEED_MORE_INPUT;
}

SinkFinalizeType PhysicalCreateRMIIndex::Finalize(Pipeline &pipeline, Event &event, ClientContext &context,
                                                  OperatorSinkFinalizeInput &input) const {
	auto &state = input.global_state.Cast<CreateRMIIndexGlobalSinkState>();
	auto &storage = table.GetStorage();
	if (!storage.IsMainTable()) {
		throw TransactionException(
		    "Transaction conflict: cannot add an index to a table that has been altered or dropped");
	}

	auto &schema = table.schema;
	info->column_ids = storage_ids;

	// Ensure that the index does not yet exist in the catalog.
	auto entry = schema.GetEntry(schema.GetCatalogTransaction(context), CatalogType::INDEX_ENTRY, info->index_name);
	if (entry) {
		if (info->on_conflict != OnCreateConflict::IGNORE_ON_CONFLICT) {
			throw CatalogException("Index with name \"%s\" already exists!", info->index_name);
		}
		// IF NOT EXISTS on existing index. We are done.
		return SinkFinalizeType::READY;
	}

	// Register the index in the catalog.
	auto index_entry = schema.CreateIndex(schema.GetCatalogTransaction(context), *info, table).get();
	D_ASSERT(index_entry);
	auto &index = index_entry->Cast<DuckIndexEntry>();
	auto in_memory_size = state.global_index ? state.global_index->GetInMemorySize() : 0;
	index.initial_index_size = in_memory_size;

	// Attach the in-memory index instance to the storage layer.
	if (!state.global_index) {
		state.global_index = make_uniq<RMI>(info->index_name, info->constraint_type, storage_ids,
		                                    TableIOManager::Get(storage), unbound_expressions, storage.db,
		                                    info->options);
	}
	storage.AddIndex(std::move(state.global_index));
	return SinkFinalizeType::READY;
}

} // namespace duckdb

