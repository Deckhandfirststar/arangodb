////////////////////////////////////////////////////////////////////////////////
/// DISCLAIMER
///
/// Copyright 2016 ArangoDB GmbH, Cologne, Germany
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

#ifndef APPLICATION_FEATURES_V8_DEALER_FEATURE_H
#define APPLICATION_FEATURES_V8_DEALER_FEATURE_H 1

#include "ApplicationFeatures/ApplicationFeature.h"

#include "Basics/ConditionVariable.h"
#include "V8/JSLoader.h"

#include <velocypack/Slice.h>
#include <velocypack/Builder.h>
#include <velocypack/velocypack-aliases.h>

struct TRI_vocbase_t;

namespace arangodb {
class Thread;
class V8Context;

class V8DealerFeature final : public application_features::ApplicationFeature {
  struct stats {
    size_t available;
    size_t busy;
    size_t dirty;
    size_t free;
    size_t max;
  };
 public:
  static V8DealerFeature* DEALER;
  static constexpr ssize_t ANY_CONTEXT = -1;
  static constexpr ssize_t ANY_CONTEXT_OR_PRIORITY = -2;

 public:
  explicit V8DealerFeature(application_features::ApplicationServer* server);

 public:
  void collectOptions(std::shared_ptr<options::ProgramOptions>) override final;
  void validateOptions(std::shared_ptr<options::ProgramOptions>) override final;
  void start() override final;
  void unprepare() override final;

 private:
  double _gcFrequency;
  uint64_t _gcInterval;
  double _maxContextAge;
  std::string _appPath;
  std::string _startupDirectory;
  std::vector<std::string> _moduleDirectory;
  uint64_t _nrMaxContexts;  // maximum number of contexts to create
  uint64_t _nrMinContexts; // minimum number of contexts to keep
  uint64_t _nrInflightContexts; // number of contexts currently in creation
  uint64_t _maxContextInvocations; // maximum number of V8 context invocations
  bool _allowAdminExecute;

 public:
  JSLoader* startupLoader() { return &_startupLoader; };
  bool addGlobalContextMethod(std::string const&);
  void collectGarbage();

  // In the following two, if the builder pointer is not nullptr, then
  // the Javascript result(s) are returned as VPack in the builder,
  // the builder is not cleared and thus should be empty before the call.
  void loadJavaScriptFileInAllContexts(TRI_vocbase_t*, std::string const& file,
                                       VPackBuilder* builder);
  void loadJavaScriptFileInDefaultContext(TRI_vocbase_t*, std::string const& file,
                                       VPackBuilder* builder);
  void startGarbageCollection();

  /// @brief forceContext == -1 means that any free context may be
  /// picked, or a new one will be created if we have not exceeded
  /// the maximum number of contexts
  /// forceContext == -2 means that any free context may be picked, 
  /// or a new one will be created if we have not exceeded or exactly
  /// reached the maximum number of contexts. this can be used to
  /// force the creation of another context for high priority tasks
  /// forceContext >= 0 means picking the context with that exact id 
  V8Context* enterContext(TRI_vocbase_t*, bool allowUseDatabase,
                          ssize_t forceContext = ANY_CONTEXT);
  void exitContext(V8Context*);

  void defineContextUpdate(
      std::function<void(v8::Isolate*, v8::Handle<v8::Context>, size_t)>,
      TRI_vocbase_t*);

  void setMinimumContexts(size_t nr) { 
    if (nr > _nrMinContexts) {
      _nrMinContexts = nr; 
    }
  }

  V8DealerFeature::stats getCurrentContextNumbers();

  void setNumberContexts(size_t nr) { _forceNrContexts = nr; }

  void defineBoolean(std::string const& name, bool value) {
    _definedBooleans[name] = value;
  }

  void defineDouble(std::string const& name, double value) {
    _definedDoubles[name] = value;
  }

  std::string const& appPath() const { return _appPath; }

 private:
  uint64_t nextId() { return _nextId++; }
  V8Context* addContext();
  V8Context* buildContext(size_t id);
  V8Context* pickFreeContextForGc();
  void shutdownContext(V8Context* context);
  void unblockContextsModification();
  void loadJavaScriptFileInternal(std::string const& file, V8Context* context,
                                  VPackBuilder* builder);
  bool loadJavaScriptFileInContext(TRI_vocbase_t*, std::string const& file, V8Context* context, VPackBuilder* builder);
  void enterContextInternal(TRI_vocbase_t* vocbase, V8Context* context, bool allowUseDatabase);
  void enterLockedContext(TRI_vocbase_t*, V8Context*, bool allowUseDatabase);
  void exitContextInternal(V8Context*);
  void exitLockedContext(V8Context*);
  void applyContextUpdate(V8Context* context);
  void shutdownContexts();

 private:
  std::atomic<bool> _ok;
  std::atomic<uint64_t> _nextId;

  std::unique_ptr<Thread> _gcThread;
  std::atomic<bool> _stopping;
  std::atomic<bool> _gcFinished;

  basics::ConditionVariable _contextCondition;
  std::vector<V8Context*> _contexts;
  std::vector<V8Context*> _freeContexts;
  std::vector<V8Context*> _dirtyContexts;
  std::unordered_set<V8Context*> _busyContexts;
  size_t _forceNrContexts;
  size_t _contextsModificationBlockers;

  JSLoader _startupLoader;

  std::map<std::string, bool> _definedBooleans;
  std::map<std::string, double> _definedDoubles;
  std::map<std::string, std::string> _definedStrings;

  std::vector<std::pair<
      std::function<void(v8::Isolate*, v8::Handle<v8::Context>, size_t)>,
      TRI_vocbase_t*>> _contextUpdates;
};
}

#endif
