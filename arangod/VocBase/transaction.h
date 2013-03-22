////////////////////////////////////////////////////////////////////////////////
/// @brief transaction subsystem
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_VOC_BASE_TRANSACTION_H
#define TRIAGENS_VOC_BASE_TRANSACTION_H 1

#include "BasicsC/common.h"

#include "BasicsC/associative.h"
#include "BasicsC/hashes.h"
#include "BasicsC/locks.h"
#include "BasicsC/vector.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

struct TRI_vocbase_s;
struct TRI_vocbase_col_s;

// -----------------------------------------------------------------------------
// --SECTION--                                                 TRANSACTION TYPES
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

typedef uint64_t TRI_transaction_cid_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction type
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_TRANSACTION_READ  = 1,
  TRI_TRANSACTION_WRITE = 2
}
TRI_transaction_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction statuses
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_TRANSACTION_UNDEFINED    = 0,
  TRI_TRANSACTION_CREATED      = 1,
  TRI_TRANSACTION_RUNNING      = 2,
  TRI_TRANSACTION_COMMITTED    = 3,
  TRI_TRANSACTION_ABORTED      = 4,
  TRI_TRANSACTION_FAILED       = 5
}
TRI_transaction_status_e;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  TRANSACTION LIST
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                               TRANSACTION CONTEXT
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief global transaction context typedef
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_transaction_context_s {
  //TRI_transaction_id_t      _id;                // last transaction id assigned
  
  TRI_read_write_lock_t     _rwLock;            // lock used to either simulatensously read this structure, 
                                                // or uniquely modify this structure
#if 0  
  TRI_mutex_t               _lock;              // lock used to serialize starting/stopping transactions
  TRI_mutex_t               _collectionLock;    // lock used when accessing _collections
  TRI_transaction_list_t    _readTransactions;  // global list of currently ongoing read transactions
  TRI_transaction_list_t    _writeTransactions; // global list of currently ongoing write transactions
  TRI_associative_pointer_t _collections;       // list of collections (TRI_transaction_collection_global_t)
#endif
  struct TRI_vocbase_s*     _vocbase;           // pointer to vocbase  
}
TRI_transaction_context_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create the global transaction context
////////////////////////////////////////////////////////////////////////////////

TRI_transaction_context_t* TRI_CreateTransactionContext (struct TRI_vocbase_s* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief free the global transaction context
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeTransactionContext (TRI_transaction_context_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief free all data associated with a specific collection
/// this function must be called for all collections that are dropped
////////////////////////////////////////////////////////////////////////////////

void TRI_RemoveCollectionTransactionContext (TRI_transaction_context_t* const,
                                             const TRI_transaction_cid_t);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                       TRANSACTION
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief collection used in a transaction
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_transaction_collection_s {
  TRI_transaction_cid_t       _cid;            // collection id
  TRI_transaction_type_e      _accessType;     // access type (read|write)
  int                         _nestingLevel;   // the transaction level that added this collection
  struct TRI_vocbase_col_s*   _collection;     // vocbase collection pointer
  uint64_t                    _numWrites;      // number of writes performed for the collection
  bool                        _locked;         // collection lock flag
}
TRI_transaction_collection_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief typedef for transaction hints
////////////////////////////////////////////////////////////////////////////////

typedef uint32_t TRI_transaction_hint_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief hints that can be used for transactions
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_TRANSACTION_HINT_NONE             = 0,
  TRI_TRANSACTION_HINT_SINGLE_OPERATION = 1,
  TRI_TRANSACTION_HINT_LOCK_ENTIRELY    = 2
}
TRI_transaction_hint_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction typedef
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_transaction_s {
  TRI_transaction_context_t*    _context;        // global context object
  // TODO: fix
  uint64_t                      _id; 
  TRI_transaction_type_e        _type;           // access type (read|write)
  TRI_transaction_status_e      _status;         // current status
  TRI_vector_pointer_t          _collections;    // list of participating collections
  TRI_transaction_hint_t        _hints;          // hints;
  int                           _nestingLevel; 
}
TRI_transaction_t;

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new transaction container
////////////////////////////////////////////////////////////////////////////////

TRI_transaction_t* TRI_CreateTransaction (TRI_transaction_context_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @brief free a transaction container
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeTransaction (TRI_transaction_t* const);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @addtogroup VocBase
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @brief dump information about a transaction
////////////////////////////////////////////////////////////////////////////////

#if 0
void TRI_DumpTransaction (TRI_transaction_t* const);
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief check if a collection is contained in a transaction and return it
////////////////////////////////////////////////////////////////////////////////

struct TRI_vocbase_col_s* TRI_GetCollectionTransaction (TRI_transaction_t* const,
                                                        const TRI_transaction_cid_t,
                                                        const TRI_transaction_type_e);

////////////////////////////////////////////////////////////////////////////////
/// @brief add a collection to a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_AddCollectionTransaction (TRI_transaction_t* const,
                                  const TRI_transaction_cid_t,
                                  const TRI_transaction_type_e, 
                                  const int);

////////////////////////////////////////////////////////////////////////////////
/// @brief request a lock for a collection
////////////////////////////////////////////////////////////////////////////////

int TRI_LockCollectionTransaction (TRI_transaction_t* const,
                                   const TRI_transaction_cid_t,
                                   const TRI_transaction_type_e,
                                   const int);

////////////////////////////////////////////////////////////////////////////////
/// @brief request an unlock for a collection
////////////////////////////////////////////////////////////////////////////////

int TRI_UnlockCollectionTransaction (TRI_transaction_t* const,
                                     const TRI_transaction_cid_t,
                                     const TRI_transaction_type_e, 
                                     const int);

////////////////////////////////////////////////////////////////////////////////
/// @brief check whether a collection is locked in a transaction
////////////////////////////////////////////////////////////////////////////////

bool TRI_IsLockedCollectionTransaction (TRI_transaction_t* const,
                                        const TRI_transaction_cid_t,
                                        const TRI_transaction_type_e,
                                        const int);

////////////////////////////////////////////////////////////////////////////////
/// @brief begin a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_BeginTransaction (TRI_transaction_t* const, 
                          TRI_transaction_hint_t,
                          const int);

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_CommitTransaction (TRI_transaction_t* const,
                           const int);

////////////////////////////////////////////////////////////////////////////////
/// @brief abort a transaction
////////////////////////////////////////////////////////////////////////////////

int TRI_AbortTransaction (TRI_transaction_t* const,
                          const int);

////////////////////////////////////////////////////////////////////////////////
/// @}
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
