add_library_unity(
    duckdb_execution_index_rmi
    OBJECT
    rmi.cpp
    rmi_index_create_logical.cpp)

set(ALL_OBJECT_FILES
    ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_execution_index_rmi>
    PARENT_SCOPE)
