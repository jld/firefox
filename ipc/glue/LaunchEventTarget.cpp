/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "LaunchEventTarget.h"

#include "mozilla/Assertions.h"
#include "mozilla/Services.h"
#include "mozilla/StaticMutex.h"
#include "mozilla/StaticPtr.h"
#include "mozilla/ThreadSafety.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsISupportsImpl.h"
#include "nsIThread.h"
#include "nsThreadPool.h"
#include "nsThreadUtils.h"

#include <string>

namespace mozilla::ipc {

// Windows needs a single dedicated thread for process launching,
// because of thread-safety restrictions/assertions in the sandbox
// code.
//
// Android also needs a single dedicated thread to simplify thread
// safety in java.
//
// Fork server doesn't need a dedicated thread anymore, but the server
// itself is single-threaded so there's not much point in concurrent
// requests.
#if defined(XP_WIN) || defined(MOZ_WIDGET_ANDROID) || \
    defined(MOZ_ENABLE_FORKSERVER)

using ILaunchEventTarget = nsIThread;

static already_AddRefed<ILaunchEventTarget> CreateLaunchEventTarget() {
  nsCOMPtr<nsIThread> thread;

  nsresult rv = NS_NewNamedThread("IPC Launch"_ns, getter_AddRefs(thread));
  NS_ENSURE_SUCCESS(rv, nullptr);

  return thread.forget();
}

#else  // other build types (mostly Mac)

using ILaunchEventTarget = nsIThreadPool;

static already_AddRefed<ILaunchEventTarget> CreateLaunchEventTarget() {
  nsCOMPtr<nsIThreadPool> pool = new nsThreadPool();

  nsresult rv = pool->SetName("IPC Launch"_ns);
  NS_ENSURE_SUCCESS(rv, nullptr);

  // Defaults are 1 idle thread (60s timeout) and 4 max threads (100ms)
  // Should be reasonable as-is.

  return pool.forget();
}

#endif  // end thread-vs-pool differences

static mozilla::StaticMutex gLaunchEventTargetMutex;
static mozilla::StaticRefPtr<ILaunchEventTarget> gLaunchEventTarget
    MOZ_GUARDED_BY(gLaunchEventTargetMutex);

class LaunchEventTargetObserver final : public nsIObserver {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER
 protected:
  virtual ~LaunchEventTargetObserver() = default;
};

NS_IMPL_ISUPPORTS(LaunchEventTargetObserver, nsIObserver, nsISupports)

NS_IMETHODIMP
LaunchEventTargetObserver::Observe(nsISupports* aSubject, const char* aTopic,
                                   const char16_t* aData) {
  MOZ_RELEASE_ASSERT(strcmp(aTopic, "xpcom-shutdown-threads") == 0);

  nsCOMPtr<ILaunchEventTarget> target;
  {
    StaticMutexAutoLock lock(gLaunchEventTargetMutex);
    target = gLaunchEventTarget.forget();
  }

  nsresult rv = target ? target->Shutdown() : NS_OK;
  mozilla::Unused << NS_WARN_IF(NS_FAILED(rv));
  return rv;
}

nsCOMPtr<nsIEventTarget> GetLaunchEventTarget() {
  StaticMutexAutoLock lock(gLaunchEventTargetMutex);
  if (!gLaunchEventTarget) {
    nsCOMPtr<ILaunchEventTarget> target = CreateLaunchEventTarget();
    if (target) {
      NS_DispatchToMainThread(NS_NewRunnableFunction(
          "GeckoChildProcessHost::GetLaunchEventTarget", [] {
            nsCOMPtr<nsIObserverService> obsService =
                mozilla::services::GetObserverService();
            nsCOMPtr<nsIObserver> obs = new LaunchEventTargetObserver();
            obsService->AddObserver(obs, "xpcom-shutdown-threads", false);
          }));
      gLaunchEventTarget = target.forget();
    }
  }

  nsCOMPtr<nsIEventTarget> target = gLaunchEventTarget.get();
  MOZ_DIAGNOSTIC_ASSERT(target);
  return target;
}

}  // namespace mozilla::ipc
