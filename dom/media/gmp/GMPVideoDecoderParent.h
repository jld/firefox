/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef GMPVideoDecoderParent_h_
#define GMPVideoDecoderParent_h_

#include "mozilla/RefPtr.h"
#include "gmp-video-decode.h"
#include "mozilla/gmp/PGMPVideoDecoderParent.h"
#include "GMPMessageUtils.h"
#include "GMPSharedMemManager.h"
#include "GMPUtils.h"
#include "GMPVideoHost.h"
#include "GMPVideoDecoderProxy.h"
#include "VideoUtils.h"
#include "GMPCrashHelperHolder.h"

namespace mozilla::gmp {

class GMPContentParent;

class GMPVideoDecoderParent final : public PGMPVideoDecoderParent,
                                    public GMPVideoDecoderProxy,
                                    public GMPSharedMemManager,
                                    public GMPCrashHelperHolder {
  friend class PGMPVideoDecoderParent;

 public:
  // Mark AddRef and Release as `final`, as they overload pure virtual
  // implementations in PGMPVideoDecoderParent.
  NS_INLINE_DECL_REFCOUNTING(GMPVideoDecoderParent, final)

  explicit GMPVideoDecoderParent(GMPContentParent* aPlugin);

  GMPVideoHostImpl& Host();
  nsresult Shutdown();

  // GMPVideoDecoder
  void Close() override;
  nsresult InitDecode(const GMPVideoCodec& aCodecSettings,
                      const nsTArray<uint8_t>& aCodecSpecific,
                      GMPVideoDecoderCallbackProxy* aCallback,
                      int32_t aCoreCount) override;
  nsresult Decode(GMPUniquePtr<GMPVideoEncodedFrame> aInputFrame,
                  bool aMissingFrames,
                  const nsTArray<uint8_t>& aCodecSpecificInfo,
                  int64_t aRenderTimeMs = -1) override;
  nsresult Reset() override;
  nsresult Drain() override;
  uint32_t GetPluginId() const override { return mPluginId; }
  GMPPluginType GetPluginType() const override { return mPluginType; }
  nsCString GetDisplayName() const override;

  // GMPSharedMemManager
  bool MgrAllocShmem(size_t aSize, Shmem* aMem) override {
    return AllocShmem(aSize, aMem);
  }

  void MgrDeallocShmem(Shmem& aMem) override { DeallocShmem(aMem); }

 protected:
  bool MgrIsOnOwningThread() const override;

 private:
  ~GMPVideoDecoderParent();

  // PGMPVideoDecoderParent
  void ActorDestroy(ActorDestroyReason aWhy) override;
  mozilla::ipc::IPCResult RecvReturnShmem(ipc::Shmem&& aInputShmem) override;
  mozilla::ipc::IPCResult RecvDecodedShmem(
      const GMPVideoi420FrameData& aDecodedFrame,
      ipc::Shmem&& aDecodedShmem) override;
  mozilla::ipc::IPCResult RecvDecodedData(
      const GMPVideoi420FrameData& aDecodedFrame,
      nsTArray<uint8_t>&& aDecodedArray) override;
  mozilla::ipc::IPCResult RecvReceivedDecodedReferenceFrame(
      const uint64_t& aPictureId) override;
  mozilla::ipc::IPCResult RecvReceivedDecodedFrame(
      const uint64_t& aPictureId) override;
  mozilla::ipc::IPCResult RecvInputDataExhausted() override;
  mozilla::ipc::IPCResult RecvDrainComplete() override;
  mozilla::ipc::IPCResult RecvResetComplete() override;
  mozilla::ipc::IPCResult RecvError(const GMPErr& aError) override;
  mozilla::ipc::IPCResult RecvShutdown() override;

  bool HandleDecoded(const GMPVideoi420FrameData& aDecodedFrame,
                     size_t aDecodedSize);

  void UnblockResetAndDrain();
  void CancelResetCompleteTimeout();

  size_t mDecodedShmemSize = 0;
  bool mIsOpen;
  bool mShuttingDown;
  bool mActorDestroyed;
  bool mIsAwaitingResetComplete;
  bool mIsAwaitingDrainComplete;
  RefPtr<GMPContentParent> mPlugin;
  RefPtr<GMPVideoDecoderCallbackProxy> mCallback;
  GMPVideoHostImpl mVideoHost;
  const uint32_t mPluginId;
  GMPPluginType mPluginType = GMPPluginType::Unknown;
  int32_t mFrameCount;
  RefPtr<SimpleTimer> mResetCompleteTimeout;
};

}  // namespace mozilla::gmp

#endif  // GMPVideoDecoderParent_h_
