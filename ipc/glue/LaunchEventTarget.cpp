/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "LaunchEventTarget.h"

#include "mozilla/Assertions.h"
#include "mozilla/Services.h"
#include "mozilla/SharedThreadPool.h"
#include "mozilla/StaticMutex.h"
#include "mozilla/StaticPtr.h"
#include "mozilla/ThreadSafety.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsISupportsImpl.h"
#include "nsIThread.h"
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
// Fork server needs a dedicated thread for accessing
// |ForkServiceChild|.
#if defined(XP_WIN) || defined(MOZ_WIDGET_ANDROID) || \
    defined(MOZ_ENABLE_FORKSERVER)

static mozilla::StaticMutex gIPCLaunchThreadMutex;
static mozilla::StaticRefPtr<nsIThread> gIPCLaunchThread
    MOZ_GUARDED_BY(gIPCLaunchThreadMutex);

class IPCLaunchThreadObserver final : public nsIObserver {
 public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIOBSERVER
 protected:
  virtual ~IPCLaunchThreadObserver() = default;
};

NS_IMPL_ISUPPORTS(IPCLaunchThreadObserver, nsIObserver, nsISupports)

NS_IMETHODIMP
IPCLaunchThreadObserver::Observe(nsISupports* aSubject, const char* aTopic,
                                 const char16_t* aData) {
  MOZ_RELEASE_ASSERT(strcmp(aTopic, "xpcom-shutdown-threads") == 0);

  nsCOMPtr<nsIThread> thread;
  {
    StaticMutexAutoLock lock(gIPCLaunchThreadMutex);
    thread = gIPCLaunchThread.forget();
  }

  nsresult rv = thread ? thread->Shutdown() : NS_OK;
  mozilla::Unused << NS_WARN_IF(NS_FAILED(rv));
  return rv;
}

nsCOMPtr<nsIEventTarget> GetLaunchEventTarget() {
  StaticMutexAutoLock lock(gIPCLaunchThreadMutex);
  if (!gIPCLaunchThread) {
    nsCOMPtr<nsIThread> thread;
    nsresult rv = NS_NewNamedThread("IPC Launch"_ns, getter_AddRefs(thread));
    if (!NS_WARN_IF(NS_FAILED(rv))) {
      NS_DispatchToMainThread(
          NS_NewRunnableFunction("GeckoChildProcessHost::GetIPCLauncher", [] {
            nsCOMPtr<nsIObserverService> obsService =
                mozilla::services::GetObserverService();
            nsCOMPtr<nsIObserver> obs = new IPCLaunchThreadObserver();
            obsService->AddObserver(obs, "xpcom-shutdown-threads", false);
          }));
      gIPCLaunchThread = thread.forget();
    }
  }

  nsCOMPtr<nsIEventTarget> thread = gIPCLaunchThread.get();
  MOZ_DIAGNOSTIC_ASSERT(thread);
  return thread;
}

#else  // defined(XP_WIN) || defined(MOZ_WIDGET_ANDROID) ||
       // defined(MOZ_ENABLE_FORKSERVER)

// Other platforms use an on-demand thread pool.

nsCOMPtr<nsIEventTarget> GetLaunchEventTarget() {
  nsCOMPtr<nsIEventTarget> pool =
      mozilla::SharedThreadPool::Get("IPC Launch"_ns);
  MOZ_DIAGNOSTIC_ASSERT(pool);
  return pool;
}

#endif  // XP_WIN || MOZ_WIDGET_ANDROID || MOZ_ENABLE_FORKSERVER

}  // namespace mozilla::ipc
