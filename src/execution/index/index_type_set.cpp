#include "duckdb/execution/index/index_type.hpp"
#include "duckdb/execution/index/index_type_set.hpp"
#include "duckdb/execution/index/art/art.hpp"
#include "duckdb/execution/index/rmi/rmi.hpp"

namespace duckdb {

IndexTypeSet::IndexTypeSet() {
	// Register the ART index type by default
	IndexType art_index_type;
	art_index_type.name = ART::TYPE_NAME;
	art_index_type.create_instance = ART::Create;
	art_index_type.create_plan = ART::CreatePlan;

	RegisterIndexType(art_index_type);
	
	IndexType rmi_index_type;
	rmi_index_type.name = RMI::TYPE_NAME;
	rmi_index_type.create_instance = RMI::Create;
	rmi_index_type.create_plan = RMI::CreatePlan;

	RegisterIndexType(rmi_index_type);
}

optional_ptr<IndexType> IndexTypeSet::FindByName(const string &name) {
	lock_guard<mutex> g(lock);
	auto entry = functions.find(name);
	if (entry == functions.end()) {
		return nullptr;
	}
	return &entry->second;
}

void IndexTypeSet::RegisterIndexType(const IndexType &index_type) {
	lock_guard<mutex> g(lock);
	if (functions.find(index_type.name) != functions.end()) {
		throw CatalogException("Index type with name \"%s\" already exists!", index_type.name.c_str());
	}
	functions[index_type.name] = index_type;
}

} // namespace duckdb
