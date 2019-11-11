#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <memory>
#include "../common/define.hpp"
#include "../common/db.hpp"

extern "C" {
#include "sqlite/sqlite3.h"
}

namespace shine
{
	namespace db {
		class sqlite_result : public db_result
		{
			friend class sqlite;
		private:
			static int parse(void* para, int count, char** value, char** name) {
				if (para == 0 || count <= 0)
				{
					return 0;
				}

				sqlite_result *result = (sqlite_result*)para;

				shine::size_t Count = (shine::size_t)count;
				if (result->_columns.empty())
				{
					for (size_t i = 0; i < Count; i++)
					{
						result->_columns.emplace(name[i], i);
						result->_head.emplace_back(name[i]);
					}
				}

				row_t row;
				row.resize(Count);
				for (size_t i = 0; i < Count; i++)
				{
					row[i] = value[i];
				}

				result->_rows.emplace_back(std::move(row));

				return 0;
			}
		};

		class sqlite
		{
		public:
			enum encode_t { UTF8, UTF16 };

		public:
			sqlite()
			{
			}

			virtual ~sqlite()
			{
				close();
			}

			static void dump_error(sqlite3* handle) {
				std::cout << sqlite3_errmsg(handle) << sqlite3_errcode(handle) << std::endl;
			}

			void dump_error() {
				dump_error(handle);
			}

			bool execute(const string &sql, sqlite_result *result = NULL) {
				if (!handle)
					return false;

				if (result)
					result->clear();

				char *msg = 0;
				int rc = sqlite3_exec(handle, sql, sqlite_result::parse, (void*)result, &msg);
				if (rc != SQLITE_OK) {
					std::cout <<"SQL error:" << sql << "  " << msg << std::endl;
					sqlite3_free(msg);
					return false;
				}
				else {
					return true;
				}
			}

			bool select(const string &sql, sqlite_result &result) {
				return execute(sql, &result);
			}

			sqlite3 *get_handle() const { return handle; }
			inline int libversion_number() const { return sqlite3_libversion_number(); }
			int changes() const { return sqlite3_changes(handle); }
			int total_changes() const { return sqlite3_total_changes(handle); }
			void interrupt() const { sqlite3_interrupt(handle); }
			int get_autocommit() const { return sqlite3_get_autocommit(handle); }

			inline void trace() const { sqlite3_trace(handle, &sqlite::trace_output, 0); }
			inline void profile() const { sqlite3_profile(handle, &sqlite::ProfileOutput, 0); }

			inline void soft_heap_limit(int heapLimit) const { sqlite3_soft_heap_limit(heapLimit); }

			inline int release_memory(int bytesOfMemory) const { return sqlite3_release_memory(bytesOfMemory); }

			long long memory_used() const { return sqlite3_memory_used(); }

			long long memory_highwater(bool resetFlag = false) const { return sqlite3_memory_highwater(resetFlag); }

			long long last_insert_rowid() const { return sqlite3_last_insert_rowid(handle); }

			inline void release_memory() { sqlite3_db_release_memory(handle); }


			bool open(const char *filename, int flags, const char *zVfs)
			{
				if (handle)
					close();

				if (sqlite3_open_v2(filename, &handle, flags, zVfs) != SQLITE_OK) {
					dump_error();
					return false;
				}

				sqlite3_extended_result_codes(handle, true);

				name = std::string(filename);
				name_utf16 = L"";

				return true;
			}

			bool open(const std::string &filename)
			{
				return open(filename.c_str(), SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
			}

			bool open(const wchar_t *filename)
			{
				if (handle)
					close();

				// standard usage: SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
				if (sqlite3_open16(filename, &handle) != SQLITE_OK) {
					dump_error();
					return false;
				}

				sqlite3_extended_result_codes(handle, true);

				name = "";
				name_utf16 = filename;
				return true;
			}

			void close()
			{
				// detach database if the database was moved into memory
				if (memory)
				{
					if (sqlite3_exec(handle, "DETACH DATABASE origin", 0, 0, 0) != SQLITE_OK) {
						dump_error();
						return;
					}
				}

				// close the database
				if (handle && sqlite3_close(handle) != SQLITE_OK) {
					dump_error();
					return;
				}
				else
				{
					handle = 0;
					name = "";
					name_utf16 = L"";
					memory = false;
				}
			}

			static void trace_output(void *ptr, const char *sql)
			{
				std::cout << "trace: " << sql << std::endl;
			}

			static void ProfileOutput(void* ptr, const char* sql, sqlite3_uint64 time)
			{
				std::cout << "profile: " << sql << std::endl;
				std::cout << "profile time: " << time << std::endl;
			}

			void move_to_memory(encode_t encoding)
			{
				if (!memory)
				{
					if (name == "" && name_utf16 == L"")
					{
						;//KOMPEX_EXCEPT("No opened database! Please open a database first.", -1);
						return;
					}

					sqlite3 *memoryDatabase;
					if (name != "")
						sqlite3_open(":memory:", &memoryDatabase);
					else
						sqlite3_open16(L":memory:", &memoryDatabase);

					// create the in-memory schema from the origin database
					sqlite3_exec(handle, "BEGIN", 0, 0, 0);

					if (encoding == UTF8)
					{
						sqlite3_exec(handle, "SELECT sql FROM sqlite_master WHERE sql NOT NULL AND tbl_name != 'sqlite_sequence'", &sqlite::ProcessDDLRow, memoryDatabase, 0);
					}
					else if (encoding == UTF16)
					{
						struct sqlite3_stmt *statement;
						if (sqlite3_prepare_v2(handle, "SELECT sql FROM sqlite_master WHERE sql NOT NULL AND tbl_name != 'sqlite_sequence'", -1, &statement, 0) != SQLITE_OK)
							cleanup(memoryDatabase, handle, false, true, statement, sqlite3_errmsg(handle), sqlite3_errcode(handle));

						bool resultsAvailable = true;
						while (resultsAvailable)
						{
							switch (sqlite3_step(statement))
							{
							case SQLITE_ROW:
							{
								resultsAvailable = true;
								struct sqlite3_stmt *transferStatement;
								if (sqlite3_prepare16_v2(memoryDatabase, sqlite3_column_text16(statement, 0), -1, &transferStatement, 0) != SQLITE_OK)
								{
									sqlite3_finalize(transferStatement);
									cleanup(memoryDatabase, handle, false, true, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
								}

								// SQLITE_DONE should always be returned
								if (sqlite3_step(transferStatement) != SQLITE_DONE)
								{
									sqlite3_finalize(transferStatement);
									cleanup(memoryDatabase, handle, false, true, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
								}

								sqlite3_finalize(transferStatement);
								break;
							}
							case SQLITE_DONE:
								resultsAvailable = false;
								break;
							case SQLITE_BUSY:
								cleanup(memoryDatabase, handle, false, true, statement, "SQLITE_BUSY", SQLITE_BUSY);
								resultsAvailable = false;
								break;
							case SQLITE_ERROR:
								cleanup(memoryDatabase, handle, false, true, statement, sqlite3_errmsg(handle), sqlite3_errcode(handle));
								resultsAvailable = false;
								break;
							default:
								cleanup(memoryDatabase, handle, false, true, statement, sqlite3_errmsg(handle), sqlite3_errcode(handle));
								resultsAvailable = false;
							}
						}

						sqlite3_finalize(statement);
					}

					sqlite3_exec(handle, "COMMIT", 0, 0, 0);

					// attach the origin database to the in-memory
					if (name != "")
					{
						std::string sql = "ATTACH DATABASE '" + name + "' as origin";
						if (sqlite3_exec(memoryDatabase, sql.c_str(), 0, 0, 0) != SQLITE_OK)
						{
							sqlite3_close(memoryDatabase);
							dump_error(memoryDatabase);
						}
					}
					else
					{
						struct sqlite3_stmt *statement;
						std::wstring sql = L"ATTACH DATABASE '" + name_utf16 + L"' as origin";
						if (sqlite3_prepare16_v2(memoryDatabase, sql.c_str(), -1, &statement, 0) != SQLITE_OK)
							cleanup(memoryDatabase, memoryDatabase, false, false, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));

						if (sqlite3_step(statement) != SQLITE_DONE)
							cleanup(memoryDatabase, memoryDatabase, false, false, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));

						sqlite3_finalize(statement);
					}

					// copy the data from the origin database to the in-memory
					sqlite3_exec(memoryDatabase, "BEGIN", 0, 0, 0);

					if (encoding == UTF8)
					{
						sqlite3_exec(memoryDatabase, "SELECT name FROM origin.sqlite_master WHERE type='table'",  sqlite::ProcessDMLRow, memoryDatabase, 0);
					}
					else if (encoding == UTF16)
					{
						struct sqlite3_stmt *statement;
						if (sqlite3_prepare_v2(memoryDatabase, "SELECT name FROM origin.sqlite_master WHERE type='table'", -1, &statement, 0) != SQLITE_OK)
							cleanup(memoryDatabase, memoryDatabase, true, false, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));

						bool resultsAvailable = true;
						while (resultsAvailable)
						{
							switch (sqlite3_step(statement))
							{
							case SQLITE_ROW:
							{
								resultsAvailable = true;

								std::wstring tableName = (wchar_t*)sqlite3_column_text16(statement, 0);
								std::wstring stmt = std::wstring(L"INSERT INTO main.") + tableName + std::wstring(L" SELECT * FROM origin.") + tableName;
								struct sqlite3_stmt *transferStatement;
								if (sqlite3_prepare16_v2(memoryDatabase, stmt.c_str(), -1, &transferStatement, 0) != SQLITE_OK)
								{
									sqlite3_finalize(transferStatement);
									cleanup(memoryDatabase, memoryDatabase, true, true, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
								}

								// SQLITE_DONE should always be returned
								if (sqlite3_step(transferStatement) != SQLITE_DONE)
								{
									sqlite3_finalize(transferStatement);
									cleanup(memoryDatabase, memoryDatabase, true, true, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
								}

								sqlite3_finalize(transferStatement);

								break;
							}
							case SQLITE_DONE:
								resultsAvailable = false;
								break;
							case SQLITE_BUSY:
								cleanup(memoryDatabase, memoryDatabase, true, true, statement, "SQLITE_BUSY", SQLITE_BUSY);
								resultsAvailable = false;
								break;
							case SQLITE_ERROR:
								cleanup(memoryDatabase, memoryDatabase, true, true, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
								resultsAvailable = false;
								break;
							default:
								cleanup(memoryDatabase, memoryDatabase, true, true, statement, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
								resultsAvailable = false;
							}
						}

						sqlite3_finalize(statement);
					}

					if (sqlite3_exec(memoryDatabase, "COMMIT", 0, 0, 0) == SQLITE_OK)
					{
						sqlite3_close(handle);
						handle = memoryDatabase;
						memory = true;
					}
					else
					{
						cleanup(memoryDatabase, memoryDatabase, true, true, 0, sqlite3_errmsg(memoryDatabase), sqlite3_errcode(memoryDatabase));
					}
				}
			}

			void cleanup(sqlite3 *memoryDatabase, sqlite3 *rollbackDatabase, bool isDetachNecessary, bool isRollbackNecessary, sqlite3_stmt *stmt, const std::string &errMsg, int internalSqliteErrCode)
			{
				if (stmt != 0)
					sqlite3_finalize(stmt);

				if (isRollbackNecessary)
					sqlite3_exec(rollbackDatabase, "ROLLBACK", 0, 0, 0);

				if (isDetachNecessary)
					sqlite3_exec(memoryDatabase, "DETACH DATABASE origin", 0, 0, 0);

				sqlite3_close(memoryDatabase);
			}

			static int ProcessDDLRow(void *db, int columnsCount, char **values, char **columns)
			{
				if (columnsCount != 1)
				{
					return -1;
				}

				// execute a sql statement in values[0] in the database db.
				if (sqlite3_exec(static_cast<sqlite3*>(db), values[0], 0, 0, 0) != SQLITE_OK) {
					dump_error((sqlite3*)db);
					return false;
				}

				return 0;
			}

			static int ProcessDMLRow(void *db, int columnsCount, char **values, char **columns)
			{
				if (columnsCount != 1)
				{
					return -1;
				}

				char *stmt = sqlite3_mprintf("INSERT INTO main.%q SELECT * FROM origin.%q", values[0], values[0]);

				if (sqlite3_exec(static_cast<sqlite3*>(db), stmt, 0, 0, 0) != SQLITE_OK) {
					dump_error((sqlite3*)db);
					return false;
				}

				sqlite3_free(stmt);

				return 0;
			}

			bool save_file(const std::string &filename = "")
			{
				if (memory)
				{
					sqlite3 *fileDatabase;
					if (filename == "")
					{
						if (name != "")
						{
							if (sqlite3_open_v2(name.c_str(), &fileDatabase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0) != SQLITE_OK) {
								dump_error(fileDatabase);
								return false;
							}
						}
						else
						{
							if (sqlite3_open16(name_utf16.c_str(), &fileDatabase) != SQLITE_OK) {
								dump_error(fileDatabase);
								return false;
							}
						}
					}
					else
					{
						if (sqlite3_open_v2(filename.c_str(), &fileDatabase, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0) != SQLITE_OK) {
							dump_error(fileDatabase);
							return false;
						}
					}

					return backup(fileDatabase);
				}

				return true;
			}

			void save_file(const wchar_t *filename)
			{
				if (memory)
				{
					sqlite3 *fileDatabase;
					if (sqlite3_open16(filename, &fileDatabase) != SQLITE_OK) {
						dump_error(fileDatabase);
					}

					backup(fileDatabase);
				}
			}

			bool backup(sqlite3 *destinationDatabase)
			{
				sqlite3_backup *backup;
				backup = sqlite3_backup_init(destinationDatabase, "main", handle, "main");
				if (backup)
				{
					// -1 to copy the entire source database to the destination
					if (sqlite3_backup_step(backup, -1) != SQLITE_DONE)
					{
						// save error message and error number - otherwise an error in sqlite3_backup_finish() could overwrite them
// 						std::string errmsg = sqlite3_errmsg(destinationDatabase);
// 						int errcode = sqlite3_errcode(destinationDatabase);
						// there must be exactly one call to sqlite3_backup_finish() for each successful call to sqlite3_backup_init()
						sqlite3_backup_finish(backup);
						dump_error(destinationDatabase);
						return false;
					}

					// clean up resources allocated by sqlite3_backup_init()
					if (sqlite3_backup_finish(backup) != SQLITE_OK) {
						dump_error(destinationDatabase);
						return false;
					}
				}

				if (sqlite3_close(destinationDatabase) != SQLITE_OK) {
					dump_error(destinationDatabase);
					return false;
				}

				return true;
			}

			bool db_readonly()
			{
				int result = sqlite3_db_readonly(handle, "main");
				if (result == -1) {
					dump_error();
					return false;
				}

				return true;
			}

			void create_module(const std::string &moduleName, const sqlite3_module *module, void *clientData, void(*xDestroy)(void*))
			{
				if (sqlite3_create_module_v2(handle, moduleName.c_str(), module, clientData, xDestroy)) {
					dump_error();
				}
			}

			int db_status(int operation, bool highwater = false, bool reset_value = false)
			{
				int cur;
				int high;

				if (sqlite3_db_status(handle, operation, &cur, &high, reset_value) != SQLITE_OK) {
					dump_error();
					return -1;
				}

				if (highwater)
					return high;

				return cur;
			}

			int DBSTATUS_LOOKASIDE_USED() 
			{
				return db_status(SQLITE_DBSTATUS_LOOKASIDE_USED);
			}

			int DBSTATUS_CACHE_USED() 
			{
				return db_status(SQLITE_DBSTATUS_CACHE_USED);
			}

			int DBSTATUS_SCHEMA_USED() 
			{
				return db_status(SQLITE_DBSTATUS_SCHEMA_USED);
			}

			int DBSTATUS_STMT_USED() 
			{
				return db_status(SQLITE_DBSTATUS_STMT_USED);
			}

			int DBSTATUS_CACHE_HIT() 
			{
				return db_status(SQLITE_DBSTATUS_CACHE_HIT);
			}

			int DBSTATUS_CACHE_MISS() 
			{
				return db_status(SQLITE_DBSTATUS_CACHE_MISS);
			}

			int DBSTATUS_CACHE_WRITE() 
			{
				return db_status(SQLITE_DBSTATUS_CACHE_WRITE);
			}

			int DBSTATUS_DEFERRED_FKS() 
			{
				return db_status(SQLITE_DBSTATUS_DEFERRED_FKS);
			}

			int DBSTATUS_LOOKASIDE_USED(bool reset_value)
			{
				return db_status(SQLITE_DBSTATUS_LOOKASIDE_USED, true, reset_value);
			}

			int DBSTATUS_LOOKASIDE_HIT(bool reset_value)
			{
				return db_status(SQLITE_DBSTATUS_LOOKASIDE_HIT, true, reset_value);
			}

			int DBSTATUS_LOOKASIDE_MISS_SIZE(bool reset_value)
			{
				return db_status(SQLITE_DBSTATUS_LOOKASIDE_MISS_SIZE, true, reset_value);
			}

			int DBSTATUS_LOOKASIDE_MISS_FULL(bool reset_value)
			{
				return db_status(SQLITE_DBSTATUS_LOOKASIDE_MISS_FULL, true, reset_value);
			}

			private:
				struct sqlite3 *handle = 0;
				std::string name;
				std::wstring name_utf16;
				bool memory = false;

		};
	} // namespace db
}	// namespace shine
