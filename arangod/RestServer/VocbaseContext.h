////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2014-2017 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
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
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGOD_REST_SERVER_VOCBASE_CONTEXT_H
#define ARANGOD_REST_SERVER_VOCBASE_CONTEXT_H 1

#include <velocypack/Builder.h>
#include <velocypack/velocypack-aliases.h>

#include "Basics/Common.h"

#include "Rest/GeneralRequest.h"
#include "Utils/ExecContext.h"

struct TRI_vocbase_t;

namespace arangodb {
/// @brief just also stores the context
class VocbaseContext final : public arangodb::ExecContext {
 private:
  VocbaseContext(VocbaseContext const&) = delete;
  VocbaseContext& operator=(VocbaseContext const&) = delete;
  VocbaseContext(GeneralRequest*, TRI_vocbase_t*, bool isSuper,
                 AuthLevel systemLevel, AuthLevel dbLevel);

 public:
  static double ServerSessionTtl;
  ~VocbaseContext();

 public:
  
  static VocbaseContext* create(GeneralRequest*, TRI_vocbase_t*);
  
  TRI_vocbase_t* vocbase() const { return _vocbase; }
  
  /// @brief upgrade to internal superuser
  void upgradeSuperuser();
  /// @brief upgrade to internal read-only user
  void upgradeReadOnly();

 private:
  TRI_vocbase_t* _vocbase;
};
}

#endif
