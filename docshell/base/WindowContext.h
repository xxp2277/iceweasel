/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim: set ts=8 sts=2 et sw=2 tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_WindowContext_h
#define mozilla_dom_WindowContext_h

#include "mozilla/Span.h"
#include "mozilla/dom/MaybeDiscarded.h"
#include "mozilla/dom/SyncedContext.h"
#include "mozilla/net/NeckoChannelParams.h"

namespace mozilla {
namespace dom {

class WindowGlobalParent;

#define MOZ_EACH_WC_FIELD(FIELD)                                       \
  FIELD(OuterWindowId, uint64_t)                                       \
  FIELD(CookieJarSettings, Maybe<mozilla::net::CookieJarSettingsArgs>) \
  /* Whether the given window hierarchy is third party. See            \
   * ThirdPartyUtil::IsThirdPartyWindow for details */                 \
  FIELD(IsThirdPartyWindow, bool)                                      \
  /* Whether this window's channel has been marked as a third-party    \
   * tracking resource */                                              \
  FIELD(IsThirdPartyTrackingResourceWindow, bool)                      \
  /* Whether the user has overriden the mixed content blocker to allow \
   * mixed content loads to happen */                                  \
  FIELD(AllowMixedContent, bool)

class WindowContext : public nsISupports, public nsWrapperCache {
  MOZ_DECL_SYNCED_CONTEXT(WindowContext, MOZ_EACH_WC_FIELD)

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(WindowContext)

 public:
  static already_AddRefed<WindowContext> GetById(uint64_t aInnerWindowId);
  static LogModule* GetLog();

  BrowsingContext* GetBrowsingContext() const { return mBrowsingContext; }
  BrowsingContextGroup* Group() const;
  uint64_t Id() const { return InnerWindowId(); }
  uint64_t InnerWindowId() const { return mInnerWindowId; }
  uint64_t OuterWindowId() const { return GetOuterWindowId(); }
  bool IsDiscarded() const { return mIsDiscarded; }

  bool IsCached() const;

  bool IsInProcess() { return mInProcess; }

  // Get the parent WindowContext of this WindowContext, taking the BFCache into
  // account. This will not cross chrome/content <browser> boundaries.
  WindowContext* GetParentWindowContext();
  WindowContext* TopWindowContext();

  Span<RefPtr<BrowsingContext>> Children() { return mChildren; }

  // Cast this object to it's parent-process canonical form.
  WindowGlobalParent* Canonical();

  nsIGlobalObject* GetParentObject() const;
  JSObject* WrapObject(JSContext* cx,
                       JS::Handle<JSObject*> aGivenProto) override;

  void Discard();

  struct IPCInitializer {
    uint64_t mInnerWindowId;
    uint64_t mBrowsingContextId;

    FieldTuple mFields;

    bool operator==(const IPCInitializer& aOther) const {
      return mInnerWindowId == aOther.mInnerWindowId &&
             mBrowsingContextId == aOther.mBrowsingContextId &&
             mFields == aOther.mFields;
    }
  };
  IPCInitializer GetIPCInitializer() {
    return {mInnerWindowId, mBrowsingContext->Id(), mFields.Fields()};
  }

  static already_AddRefed<WindowContext> Create(WindowGlobalChild* aWindow);
  static void CreateFromIPC(IPCInitializer&& aInit);

 protected:
  WindowContext(BrowsingContext* aBrowsingContext, uint64_t aInnerWindowId,
                bool aInProcess, FieldTuple&& aFields);
  virtual ~WindowContext();

  void Init();

 private:
  friend class BrowsingContext;

  void AppendChildBrowsingContext(BrowsingContext* aBrowsingContext);
  void RemoveChildBrowsingContext(BrowsingContext* aBrowsingContext);

  // Send a given `BaseTransaction` object to the correct remote.
  void SendCommitTransaction(ContentParent* aParent,
                             const BaseTransaction& aTxn, uint64_t aEpoch);
  void SendCommitTransaction(ContentChild* aChild, const BaseTransaction& aTxn,
                             uint64_t aEpoch);

  bool CheckOnlyOwningProcessCanSet(ContentParent* aSource);

  // Overload `CanSet` to get notifications for a particular field being set.
  bool CanSet(FieldIndex<IDX_OuterWindowId>, const uint64_t& aValue,
              ContentParent* aSource) {
    return GetOuterWindowId() == 0 && aValue != 0;
  }

  bool CanSet(FieldIndex<IDX_AllowMixedContent>, const bool& aAllowMixedContent,
              ContentParent* aSource);

  bool CanSet(FieldIndex<IDX_CookieJarSettings>,
              const Maybe<mozilla::net::CookieJarSettingsArgs>& aValue,
              ContentParent* aSource);
  bool CanSet(FieldIndex<IDX_IsThirdPartyWindow>,
              const bool& IsThirdPartyWindow, ContentParent* aSource);
  bool CanSet(FieldIndex<IDX_IsThirdPartyTrackingResourceWindow>,
              const bool& aIsThirdPartyTrackingResourceWindow,
              ContentParent* aSource);

  // Overload `DidSet` to get notifications for a particular field being set.
  //
  // You can also overload the variant that gets the old value if you need it.
  template <size_t I>
  void DidSet(FieldIndex<I>) {}
  template <size_t I, typename T>
  void DidSet(FieldIndex<I>, T&& aOldValue) {}

  uint64_t mInnerWindowId;
  RefPtr<BrowsingContext> mBrowsingContext;

  // --- NEVER CHANGE `mChildren` DIRECTLY! ---
  // Changes to this list need to be synchronized to the list within our
  // `mBrowsingContext`, and should only be performed through the
  // `AppendChildBrowsingContext` and `RemoveChildBrowsingContext` methods.
  nsTArray<RefPtr<BrowsingContext>> mChildren;

  bool mIsDiscarded = false;
  bool mInProcess = false;
};

using WindowContextTransaction = WindowContext::BaseTransaction;
using WindowContextInitializer = WindowContext::IPCInitializer;
using MaybeDiscardedWindowContext = MaybeDiscarded<WindowContext>;

// Don't specialize the `Transaction` object for every translation unit it's
// used in. This should help keep code size down.
extern template class syncedcontext::Transaction<WindowContext>;

}  // namespace dom

namespace ipc {
template <>
struct IPDLParamTraits<dom::MaybeDiscarded<dom::WindowContext>> {
  static void Write(IPC::Message* aMsg, IProtocol* aActor,
                    const dom::MaybeDiscarded<dom::WindowContext>& aParam);
  static bool Read(const IPC::Message* aMsg, PickleIterator* aIter,
                   IProtocol* aActor,
                   dom::MaybeDiscarded<dom::WindowContext>* aResult);
};

template <>
struct IPDLParamTraits<dom::WindowContext::IPCInitializer> {
  static void Write(IPC::Message* aMessage, IProtocol* aActor,
                    const dom::WindowContext::IPCInitializer& aInitializer);

  static bool Read(const IPC::Message* aMessage, PickleIterator* aIterator,
                   IProtocol* aActor,
                   dom::WindowContext::IPCInitializer* aInitializer);
};
}  // namespace ipc
}  // namespace mozilla

#endif  // !defined(mozilla_dom_WindowContext_h)
